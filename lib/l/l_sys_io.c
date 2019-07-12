
/* $Id: l_sys_io.c 22182 2018-07-14 23:56:00Z kobus $ */

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

#include "l/l_gen.h"     /* Only safe as first include in a ".c" file. */

#include "l/l_sys_term.h"
#include "l/l_string.h"
#include "l/l_sys_str.h"
#include "l/l_parse.h"
#include "l/l_sys_io.h"
#include "l/l_sys_scan.h"
#include "l/l_sys_sig.h"
#include "l/l_sys_time.h"
#include "l/l_sys_tsig.h"
#include "l/l_verbose.h"
#include "l/l_word_list.h"

/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#    include <sys/stat.h>
#endif

#ifdef UNIX
/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#    include <sys/param.h>
#    include <pwd.h>
#    include <fcntl.h>
#    include <glob.h>
#endif
#endif

#ifdef  SUN4
/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#    include <sys/filio.h>
#endif
#endif

#ifdef  HPUX
/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#    include <sys/ioctl.h>
#endif
#endif

#ifdef  LINUX
/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#    include <sys/ioctl.h>
#    include <time.h>
#endif
#endif

#ifdef  MAC_OSX
/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#    include <sys/ioctl.h>
#    include <sys/time.h>
#endif
#endif

#ifdef MS_OS
#    include "limits.h"
#    include "direct.h"
#    include "io.h"
#    include "time.h"

#    ifdef __STDC__
#        define S_IFMT   _S_IFMT
#        define S_IFDIR  _S_IFDIR
#        define S_IFCHR  _S_IFCHR
#        define S_IFREG  _S_IFREG
#        define S_IREAD  _S_IREAD
#        define S_IWRITE _S_IWRITE
#        define S_IEXEC  _S_IEXEC

#        define stat _stat 

#        define dup _dup
#        define read _read
#        define write _write

#        define unlink _unlink
#        define fdopen _fdopen
#        define popen _popen
#        define pclose _pclose
#        define mkdir _mkdir
#        define rmdir _rmdir
#        define fstat _fstat
#        define isatty _isatty
#    endif 
    
#endif  /* End of CASE MS_OS. */

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef SSIZE_MAX
     /*
      * Could be max of unsigned long, but since it is not defined, we will be
      * conservative.
     */
#    define SSIZE_MAX INT_MAX
#endif 

/*
#define TEST_LARGE_IO
*/

#ifdef TEST_LARGE_IO
#    define MAX_FILE_DESCRIPTOR_IO_SIZE  100
#    define MAX_STREAM_IO_SIZE           100
#    define PAGE_SIZE                    11
#else
#    define PAGE_SIZE                    1024
#    define MAX_FILE_DESCRIPTOR_IO_SIZE  SSIZE_MAX
#    define MAX_STREAM_IO_SIZE           LONG_MAX
#endif

/* -------------------------------------------------------------------------- */

#ifdef SUN4
    extern char* realpath(char*, char[ MAXPATHLEN ]);
#endif

#ifdef NeXT
    extern char* realpath(char*, char[ MAXPATHLEN ]);
#endif


#ifndef EXCHANGE_USR_AND_NET
#    define EXCHANGE_USR_AND_NET  FALSE
#endif

#ifndef DISABLE_DIR_OPEN
#    define DISABLE_DIR_OPEN FALSE
#endif

static int   fs_uncompress_files     = TRUE;
#if 0 /* ifdef OBSOLETE_BUT_KEEP_AS_EXAMPLE_CODE */
static int   fs_exchange_usr_and_net = FALSE;
#endif
static int   fs_disable_dir_open     = FALSE;
static FILE* fs_stderr_shadow_fp     = NULL;
static FILE* fs_stdout_shadow_fp     = NULL;
static int   fs_enable_stdout        = TRUE;


/*
 * Some extra stuff that we use which is defined in some sys/stats but not all
 * (NeXT for example). On those systems that I have deal with so far, the
 * building blocks have been defined defined, so we use them for now.
*/

#ifndef S_IRWXU
#    define S_IRWXU         0000700        /* rwx, owner */
#endif

#ifdef S_IFMT   /* Needed for these to work. */
#ifndef S_ISSOCK
#ifdef S_IFSOCK
#    define S_ISSOCK(m)    (((m)&S_IFMT) == S_IFSOCK)
#endif 
#endif

#ifndef S_ISFIFO
#ifdef S_IFIFO
#    define S_ISFIFO(m)    (((m)&S_IFMT) == S_IFIFO)
#endif
#endif

#ifndef S_ISCHR
#ifdef S_IFCHR
#    define S_ISCHR(mode)  (((mode) & (S_IFMT)) == (S_IFCHR))
#endif
#endif

#ifndef S_ISDIR
#ifdef S_IFDIR
#    define S_ISDIR(mode)  (((mode) & (S_IFMT)) == (S_IFDIR))
#endif
#endif

#ifndef S_ISBLK
#ifdef S_IFBLK
#    define S_ISBLK(mode)  (((mode) & (S_IFMT)) == (S_IFBLK))
#endif
#endif

#ifndef S_ISLNK
#ifdef S_IFLNK
#    define S_ISLNK(mode)  (((mode) & (S_IFMT)) == (S_IFLNK))
#endif
#endif

#ifndef S_ISREG
#ifdef S_IFREG
#    define S_ISREG(mode)  (((mode) & (S_IFMT)) == (S_IFREG))
#endif
#endif
#endif


/* ------------------------------------------------------------------------- */

/*
// This must be at least the maximum number of file descriptors available.
*/
#define  MAX_NUM_NAMED_FILES      255

#define BUFF_EXPAND_PATH(x, y)    expand_path((x), (y), sizeof(x))
#define OPEN_FOR_WRITING(x)       (x != MODE_IS_READ)

/* ------------------------------------------------------------------------- */

typedef enum Mode
{
    MODE_IS_NOT_SET,
    MODE_IS_READ,
    MODE_IS_WRITE,
    MODE_IS_APPEND,
    MODE_IS_READ_PLUS,
    MODE_IS_WRITE_PLUS,
    MODE_IS_APPEND_PLUS
}
Mode;

/* -------------------------------------------------------------------------- */


static FILE* kjb_fopen_2(const char* input_fd_name, const char* mode);

static FILE* kjb_fopen_3(const char* input_fd_name, const char* mode);

static int expand_path(char*, const char*, size_t);

static Path_type get_path_type_guts(struct stat* path_stats_ptr);

static int cache_file_name
(
    FILE*       fp,
    const char* expanded_path,
    const char* fd_name,
    const char* uncompressed_fd_name,
    const char* mode
);



#ifdef TRACK_MEMORY_ALLOCATION
static void free_cached_file_names(void);
#endif

static int uncache_file_name(int file_des);
static int initialize_cache(void);
static Mode get_mode_for_cache(const char* mode);

static void get_mode_for_error_message
(
    char*  mode,
    char*  buff,
    size_t len
);

static int kjb_isatty_guts(int file_des);

static int process_file_open_mode
(
    const char* mode,
    char*       system_dependent_mode,
    size_t      system_dependent_mode_max_len,
    char*       unix_mode,
    size_t      unix_mode_max_len
);

static void close_debug_file(void);

/* ------------------------------------------------------------------------- */

static char* fs_cached_fnames                [ MAX_NUM_NAMED_FILES ];
static char* fs_cached_user_fnames           [ MAX_NUM_NAMED_FILES ];
static char* fs_cached_uncompressed_fnames   [ MAX_NUM_NAMED_FILES ];

/*
// The first element of the following does double duty as an invalidation
// indicator.
*/
static int   fs_cached_file_reference_count  [ MAX_NUM_NAMED_FILES ] = { NOT_SET };

static FILE* fs_cached_fp                    [ MAX_NUM_NAMED_FILES ];
static Mode  fs_cached_mode                  [ MAX_NUM_NAMED_FILES ];
static int   fs_cached_isatty                [ MAX_NUM_NAMED_FILES ];

#ifdef OLD_SYSV_NON_BLOCKING
    /*
    // If we have to deal with systems where return from a non-blocking device
    // is indestinguishable from EOF, then we may want to resurect keeping
    // track of what we are blocking on. If the need arises, #define
    // OLD_SYSV_NON_BLOCKING in the header file (l_sys_io.h). This will implement
    // a is_blocking() routine for devices whose blocking is modified only
    // through set_blocking() and set_no_blocking().
    */
    static int fs_cached_is_blocking[ MAX_NUM_NAMED_FILES ];
#endif

/* ------------------------------------------------------------------------- */

/*
// Not sure if we want to have fs_debug_fp and pdo only for debugging. Leave
// available to all code for now.
*/
static FILE* fs_debug_fp = NULL;

/* -------------------------------------------------------------------------- */



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             set_low_level_io_options
 *
 * This lets the user control various input-output behavior.
 *
 * This function lets one control some of the behind-the-scenes behavior of
 * library code.  For example, do you want the library to silently expand
 * compressed files?  If you kjb_fopen() a filename that is actually a
 * directory, do you want the function call to fail on you, or not?  You should
 * read the source code for a complete list of options, but hopefully these
 * two examples give you an idea of the flavor of library behavior that is
 * controlled here.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Documenter:
 *    Andrew Predoehl
 *
 * Returns:
 *    On success, returns NO_ERROR.  If the 'option' input is not one of the
 *    recognized options, then this returns NOT_FOUND.  If an internal error
 *    occurs (which is unlikely), then this returns ERROR.
 *
 * Index: input, output, I/O, files
 *
 * -----------------------------------------------------------------------------
*/
int set_low_level_io_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  temp_boolean_value;
    int  result             = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);


#if 0 /* was ifdef OBSOLETE_BUT_KEEP_AS_EXAMPLE_CODE */
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "exchange-usr-and-net"))
         || (match_pattern(lc_option, "exchange-net-and-usr"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("exchange-usr-and-net = <obsolute>\n"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("/usr/local-<x> and /net/local-<x> are treated as %s.\n",
                    fs_exchange_usr_and_net ? "equavalent" : "different" ));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_exchange_usr_and_net = temp_boolean_value;
        }
        result = NO_ERROR;
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "uncompress-files"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("uncompress-files = %s\n",
                    fs_uncompress_files ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Files %s (temporarily) uncompressed on open.\n",
                    fs_uncompress_files ? "are" : "are not" ));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_uncompress_files = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "disable-dir-open"))
         || (match_pattern(lc_option, "disable-directory-open"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("disable-directory-open = %s\n",
                    fs_disable_dir_open ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Use of directories for file names %s an error.\n",
                    fs_disable_dir_open ? "is" : "is not" ));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_disable_dir_open = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             kjb_read_exact
 *
 * Reads a specifed number of bytes from a file descriptor.
 *
 * This routine is very similar to kjb_read, execpt that exactly "len" bytes
 * must be successfully read for a successfull return. Otherwise, ERROR is
 * returned, with an approapriate error measure being set.
 *
 * Returns:
 *    On success kjb_read_exact returns NO_ERROR. Since the number of bytes that
 *    must be read is in the parameter, there is no need to return that (thus
 *    avoiding problems due to signed versus unsigned types). On failure ERROR
 *    is returned and an error message is set. Other kjb_read returns such as
 *    EOF will not occur.
 *
 * Index: input, I/O
 *
 * -----------------------------------------------------------------------------
*/

int kjb_read_exact(int file_des, void* buff, size_t len)
{
    int read_res;
    size_t num_bytes_read = 0;


    UNTESTED_CODE(); /* Since switch to kjb_read_2(), etc. */

    ERE(read_res = kjb_read_2(file_des, buff, len, &num_bytes_read));

    if (read_res == NO_ERROR)
    {
        if (num_bytes_read != len)
        {
            set_error("The number of bytes read (%ul) is not the number requested (%ul).\n",
                      (unsigned long)num_bytes_read, (unsigned long)len);
            add_error("Error occurred in a request to read exactly %lu bytes.\n",
                      (unsigned long)len);
            return ERROR;
        }
        else
        {
            return NO_ERROR;
        }
    }

    if (read_res == WOULD_BLOCK)
    {
        UNTESTED_CODE();
        set_error("Read from %D failed because it would block.", file_des);
    }
    else if (read_res == INTERRUPTED)
    {
        UNTESTED_CODE();
        set_error("Read from %D failed because it was interrupted.", file_des);
    }
    else if (read_res == EOF)
    {
        UNTESTED_CODE();
        set_error("Unexpected end of file reading %D.", file_des);
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
    }

    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_read
 *
 * Reads up to specified number of bytes from a device
 *
 * Similer to system read except that error handling is simplified. If the
 * number of bytes requested is more than MIN_OF(LONG_MAX, SSIZE_MAX), this
 * routine returns ERROR.
 *
 * Note:
 *     The routine kjb_read_2() can be used to get around the above byte limit
 *     to a certain extent, but under many circumstances, only a factor of 2
 *     more bytes can be read in this way and, in fact, typically only up to a
 *     factor of 2 more bytes are likely to be available. .
 *
 * Returns:
 *    On success kjb_read returns the number of bytes read. On end of file. EOF
 *    is returned. On failure ERROR is returned and an error message is set.
 *    If the I/O is non blocking, then WOULD_BLOCK may also be returned.
 *    Finally, depending on the type of signal handling in place, INTERRUPTED
 *    may also be returned.
 *
 * Index: input, I/O
 *
 * -----------------------------------------------------------------------------
*/

long kjb_read
(
    int des,           /* File descriptor */
    void* buff,        /* Buffer for read */
    size_t len   /* Number of bytes to read */
)
{
#ifndef errno  /* GNU defines errno as a macro, leading to warning messages. */
    IMPORT int errno;
#endif
    long result;


    UNTESTED_CODE();

    if (len > MIN_OF(MAX_STREAM_IO_SIZE, MAX_FILE_DESCRIPTOR_IO_SIZE))
    {
        set_error("The routine kjb_read can only read up to %ld bytes.\n",
                  MIN_OF(MAX_STREAM_IO_SIZE, MAX_FILE_DESCRIPTOR_IO_SIZE));
        add_error("Consider using kjb_read_2() instead.");
        return ERROR;
    }

    if (len == 0)
    {
        return 0;
    }

    result = read(des, buff, len);

    if (result > 0)
    {
        return result;
    }
    else if (result == 0)
    {
        return EOF;
    }
#ifdef UNIX
    else if (errno == NON_BLOCKING_READ_FAILURE_ERROR_NUM)
    {
        return WOULD_BLOCK;
    }
#endif 
    else if (errno == EINTR)
    {
        return INTERRUPTED;
    }
    else
    {
        set_error("Read failed.%S");
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_read_2
 *
 * Wraps read(2) to conform with the kjb library conventions.
 *
 * This routine essentially wraps read(2) to conform with the kjb library
 * conventions.  Unlike kjb_read(), it can read up to the maximum of size_t
 * bytes.
 *
 * Returns:
 *    On success kjb_read_2 returns NO_ERROR, and the number of bytes is put
 *    into *num_bytes_read_ptr if it is not NULL. On end of file. EOF is
 *    returned. On failure ERROR is returned and an error message is set.  If
 *    the I/O is non blocking, then WOULD_BLOCK may also be returned.  Finally,
 *    depending on the type of signal handling in place, INTERRUPTED may also be
 *    returned.
 *
 * Index: input, I/O
 *
 * -----------------------------------------------------------------------------
*/

int kjb_read_2
(
    int     des,  /* File descriptor */
    void*   buff, /* Buffer for read */
    size_t  len,  /* Number of bytes to read */
    size_t* len_ptr  /* Address for number of bytes read; can be NULL */
)
{
#ifndef errno  /* GNU defines errno as a macro, leading to warning messages. */
    IMPORT int errno;
#endif
#ifdef PRAVEEN_WAY
    size_t block_size = PAGE_SIZE * 100000; /* 100MB */
#else
    /*
     * Paranoid. It does not seem that we can rely on writes bigger than 2GB
     * even on 64 bit machines? Or is something else to blame?
     *
    size_t block_size = PAGE_SIZE * (MIN_OF(MAX_STREAM_IO_SIZE, MAX_FILE_DESCRIPTOR_IO_SIZE) / PAGE_SIZE);
    */
    size_t block_size = PAGE_SIZE * (MIN_OF(INT32_MAX, MIN_OF(MAX_STREAM_IO_SIZE, MAX_FILE_DESCRIPTOR_IO_SIZE)) / PAGE_SIZE);
#endif
    size_t bytes_to_read, system_read_res;
    size_t total_read = 0;


    UNTESTED_CODE();

    if (block_size == 0)
    {
        block_size = MIN_OF(INT32_MAX, (MIN_OF(MAX_STREAM_IO_SIZE, MAX_FILE_DESCRIPTOR_IO_SIZE)));
    }

    if (len == 0)
    {
        if (len_ptr != NULL)
        {
            *len_ptr = 0;
        }
        return NO_ERROR;
    }


    UNTESTED_CODE(); /* Size code to make sure that we can read up to max of size_t bytes. */

#if 0 /* was ifdef HOW_IT_WAS */
    result = read(des, buff, len);
#endif

    /* Code patterned from code provided by Praveen Rao.  */

    while (total_read < len)
    {
        bytes_to_read = (len - total_read) > block_size ? block_size : (len - total_read);

        system_read_res = read(des, (void *)((char *) buff + total_read), bytes_to_read);

        total_read += system_read_res;

        if (system_read_res < bytes_to_read)
        {
            break;
        }
    }

    if (total_read > 0)
    {
        if (len_ptr != NULL)
        {
            *len_ptr = total_read;
        }
        return NO_ERROR;
    }
    else if (total_read == 0)
    {
        return EOF;
    }
#ifdef UNIX
    else if (errno == NON_BLOCKING_READ_FAILURE_ERROR_NUM)
    {
        return WOULD_BLOCK;
    }
#endif 
    else if (errno == EINTR)
    {
        return INTERRUPTED;
    }
    else
    {
        set_error("Read failed.%S");
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
static void broken_pipe_trap_fn(TRAP_FN_DUMMY_ARGS)
{
    dbp("In broken_pipe_trap_fn.\n");

    TEST_PSE(("In broken_pipe_trap_fn.\n"));
}
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                               safe_pipe_write
 *
 * Writes to pipe with trap for SIGPIPE
 *
 * This routine writes to a pipe, trapping SIGPIPE so that if raised, error is
 * returned. Otherwise, this routine is the same as kjb_write. This routine can
 * be safely called with a descriptor for any type of file, but if the type of
 * file is known not to be a pipe, then there is nothing to be gained over
 * kjb_write.
 *
 * Returns:
 *    On success safe_pipe_write returns the number of bytes written. On
 *    failure ERROR is returned and an error message is set.
 *
 * Index: output, I/O
 *
 * -----------------------------------------------------------------------------
*/

int safe_pipe_write
(
     int des,           /* File descriptor */
     const void* buff,  /* Location of bytes to write.  */
     size_t num_bytes   /* Number of bytes to write. */
)
{
#ifdef UNIX
    Signal_info save_broken_pipe_vec;
    Signal_info new_broken_pipe_vec;
#endif 
    int         result;
    Error_action save_error_action;



#ifdef UNIX
    INIT_SIGNAL_INFO(new_broken_pipe_vec);
    new_broken_pipe_vec.SIGNAL_HANDLER = broken_pipe_trap_fn;
    kjb_sigvec(SIGPIPE, &new_broken_pipe_vec, &save_broken_pipe_vec);
#endif 

    verbose_pso(300, (const char*)buff);

    result = kjb_write(des, buff, num_bytes);


    if (result == ERROR)
    {
        save_error_action = get_error_action();
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
    }

#ifdef UNIX
    kjb_sigvec(SIGPIPE, &save_broken_pipe_vec, (Signal_info*)NULL);
#endif 

    if (result == ERROR)
    {
        set_error_action(save_error_action);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_write
 *
 * Writes a specifed number of bytes to a device
 *
 * Similer to system write except that error handling is simplified.
 * This routine can only write up to  MIN_OF(LONG_MAX, SSIZE_MAX), bytes. If
 * num_bytes is more than that, this routine returns ERROR. The alternative
 * wrapper, kjb_write_2() can be used to write up to the maximun value that
 * size_t can hold.
 *
 * Returns:
 *    On success kjb_write returns the number of bytes written.   On failure
 *    ERROR is returned and an error message is set.
 *
 * Index: output, I/O, paging
 *
 * -----------------------------------------------------------------------------
*/

long kjb_write
(
     int des           /* File descriptor */,
     const void* buff  /* Location of bytes to write.  */,
     size_t len  /* Number of bytes to write. */
)
{
    size_t write_res;


    if (len > MIN_OF(MAX_STREAM_IO_SIZE, MAX_FILE_DESCRIPTOR_IO_SIZE))
    {
        set_error("The routine kjb_write can only write up to %ld bytes at a time.\n",
                  MIN_OF(MAX_STREAM_IO_SIZE, MAX_FILE_DESCRIPTOR_IO_SIZE));
        add_error("Consider using kjb_write_2() instead.");
        return ERROR;
    }

    if (len == 0)
    {
        return 0;
    }

    write_res = write(des, buff, len);

    if (write_res == len)
    {
        return write_res;
    }
    else
    {
#ifdef TEST
        set_error("Write of %ld bytes returned %lu.%S.", (long)len,
                  (unsigned long)write_res);
#else
        set_error("Write failed.%S");
#endif

        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             kjb_write_2
 *
 * Writes a specified number of bytes to a stream
 *
 * This routine is similar to write (but note the difference in arguments!).
 * However, error handling is made more consistant with the rest of the KJB
 * library. Several other KJB library IO conventions and features are also
 * implemented. (One should not mix KJB and non-KJB IO)!
 *
 * The number of bytes that are written are returned in len_ptr. This can be set
 * to NULL if you are not interested.
 *
 * Index: output, I/O, paging
 *
 * Returns:
 *    On success kjb_write returns the number of bytes written.   On failure
 *    ERROR is returned and an error message is set.
 *
 * -----------------------------------------------------------------------------
*/

int kjb_write_2
(
    int         des,     /* File descriptor */
    const void* buff,    /* Pointer to bytes to write */
    size_t      len,     /* Number of bytes to write. */
    size_t*     len_ptr  /* Number of bytes written. */
)
{
#ifdef PRAVEEN_WAY
    size_t block_size = PAGE_SIZE * 100000; /* 100MB */
#else
    /*
     * Paranoid. It does not seem that we can rely on writes bigger than 2GB
     * even on 64 bit machines? Or is something else to blame?
     *
    size_t block_size = PAGE_SIZE * (MIN_OF(MAX_STREAM_IO_SIZE, MAX_FILE_DESCRIPTOR_IO_SIZE) / PAGE_SIZE);
    */
    size_t block_size = PAGE_SIZE * (MIN_OF(INT32_MAX, MIN_OF(MAX_STREAM_IO_SIZE, MAX_FILE_DESCRIPTOR_IO_SIZE)) / PAGE_SIZE);
#endif
    size_t bytes_to_write;
    int    write_res = NO_ERROR;
    size_t total_written = 0;


    UNTESTED_CODE();

    if (block_size == 0)
    {
        block_size = MIN_OF(INT32_MAX, (MIN_OF(MAX_STREAM_IO_SIZE, MAX_FILE_DESCRIPTOR_IO_SIZE)));
    }

    /* Code patterned from code provided by Praveen Rao.  */

    while (total_written < len)
    {
        bytes_to_write = (len - total_written) > block_size ? block_size : (len - total_written);

        write_res = kjb_write(des, (const void *)((const char *) buff + total_written), bytes_to_write);

        if (write_res < 0) break;

        total_written += write_res;
    }

    if (len_ptr != NULL)
    {
        *len_ptr = total_written;
    }

    if (write_res < 0)
    {
        return write_res;
    }
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_fread_exact
 *
 * Reads a specifed number of bytes from a stream.
 *
 * This routine is very similar to kjb_fread, execpt that exactly "len" bytes
 * must be successfully read for a successfull return. Otherwise, ERROR is
 * returned, with an approapriate error measure being set.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *    On success kjb_fread_exact returns NO_ERROR. Since the number of bytes that
 *    must be read is in the parameter, there is no need to return that (thus
 *    avoiding problems due to signed versus unsigned types). On failure ERROR
 *    is returned and an error message is set. Other kjb_fread returns such as
 *    EOF will not occur.
 *
 * Macros:
 *    FIELD_READ
 *
 * Index: input, I/O
 *
 * -----------------------------------------------------------------------------
*/

int kjb_fread_exact(FILE* fp, void* buff, size_t len)
{
    int read_res;
    size_t num_bytes_read;


#ifdef TOO_NOISY
    UNTESTED_CODE();   /* Since use of kjb_read_2(). */
#endif

    ERE(read_res = kjb_fread_2(fp, buff, len, &num_bytes_read));

    if (read_res == NO_ERROR)
    {
        if (num_bytes_read != len)
        {
            set_error("The number of bytes read (%ul) is not the number requested (%ul).\n",
                      (unsigned long)num_bytes_read,
                      (unsigned long)len);
            add_error("Error occurred in a request to read exactly %lu bytes.\n",
                      (unsigned long)len);
            return ERROR;
        }
        else
        {
            return NO_ERROR;
        }
    }
    else if (read_res == WOULD_BLOCK)
    {
        UNTESTED_CODE();
        set_error("Read from %F failed because it would block.", fp);
    }
    else if (read_res == INTERRUPTED)
    {
        UNTESTED_CODE();
        set_error("Read from %F failed because it was interrupted.", fp);
    }
    else if (read_res == EOF)
    {
        UNTESTED_CODE();
        set_error("Unexpected end of file reading file %F.", fp);
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
    }

    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_fread
 *
 * Reads a specifed number of bytes from a stream.
 *
 * This is essentially a front end for fread, but there are some differences.
 * First, the arguments are different. The file pointer is first, and the size
 * arguments are combined. Second, the error reporting is more consistent with
 * the rest of the library.
 *
 * This routine can only read up to LONG_MAX bytes at a time.  If the number of
 * bytes requested is more than MIN_OF(LONG_MAX, SSIZE_MAX), this routine
 * returns ERROR.  The routine kjb_read() can be used to read up the number of
 * bytes that can stored in size_t.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *    On success kjb_fread returns the number of bytes read. On end of file. EOF
 *    is returned. On failure ERROR is returned and an error message is set.
 *    If the I/O is non blocking, then WOULD_BLOCK may also be returned.
 *    Finally, depending on the type of signal handling in place, INTERRUPTED
 *    may also be returned.
 *
 * Index: input, I/O
 *
 * -----------------------------------------------------------------------------
*/

/* TODO. Consider an alternative version that forces the result to be null
 * terminated. (See writup_to_man.c where this is done as part of using this
 * routine). 
 *
 * TODO. Sort out the difference between this routime and kjb_fread_2.
*/

long kjb_fread(FILE* fp, void* buff, size_t len)
{
#ifndef errno  /* GNU defines errno as a macro, leading to warning messages. */
    IMPORT int          errno;
#endif
    IMPORT volatile int num_term_lines;
    IMPORT volatile Bool pause_on_next;
    size_t              read_res;


    if (len > MAX_STREAM_IO_SIZE)
    {
        set_error("The routine kjb_fread can only read up to %ld bytes.\n",
                  MAX_STREAM_IO_SIZE);
        add_error("Consider using kjb_fread_2() instead.");
        return ERROR;
    }

    clearerr(fp);

    if (kjb_isatty(fileno(fp)))
    {
        pause_on_next = FALSE;
        num_term_lines = 0;
        num_term_chars = 0;
    }

#ifdef MAX_IO_RESULT_IO_SIZE
    if (len > MAX_IO_RESULT_IO_SIZE)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
#endif

    if (len == 0) return 0;

    read_res = fread((void*)buff, (size_t)1, (size_t)len, fp);

    if (read_res > 0)
    {
        return read_res;
    }
#ifdef UNIX
    else if (errno == NON_BLOCKING_READ_FAILURE_ERROR_NUM)
    {
        return WOULD_BLOCK;
    }
#endif 
    else if (errno == EINTR)
    {
        UNTESTED_CODE();
        return INTERRUPTED;
    }
    else if (feof(fp))
    {
        return EOF;
    }
    else
    {
        UNTESTED_CODE();
        set_error("Read from %F failed.%S", fp);
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_fread_2
 *
 * Reads a specifed number of bytes from a stream.
 *
 * This routine is essentially a front end for fread, but there are some
 * differences.  First, the arguments are different. The file pointer is first,
 * and the size arguments are combined. Second, the error reporting is more
 * consistent with the rest of the library.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *    On success kjb_fread returns the number of bytes read. On end of file. EOF
 *    is returned. On failure ERROR is returned and an error message is set.
 *    If the I/O is non blocking, then WOULD_BLOCK may also be returned.
 *    Finally, depending on the type of signal handling in place, INTERRUPTED
 *    may also be returned.
 *
 * Index: input, I/O
 *
 * -----------------------------------------------------------------------------
*/

int kjb_fread_2(FILE* fp, void* buff, size_t len, size_t* len_ptr)
{
    IMPORT volatile int num_term_lines;
    IMPORT volatile Bool pause_on_next;
#ifdef PRAVEEN_WAY
    size_t block_size = PAGE_SIZE * 100000; /* 100MB */
#else
    /*
     * Paranoid. It does not seem that we can rely on writes bigger than 2GB
     * even on 64 bit machines? Or is something else to blame?
     *
    size_t block_size = PAGE_SIZE * (MAX_STREAM_IO_SIZE / PAGE_SIZE);
    */
    size_t block_size = PAGE_SIZE * (MIN_OF(INT32_MAX, MAX_STREAM_IO_SIZE) / PAGE_SIZE);
#endif
    size_t total_read, bytes_to_read;
    long read_res = NO_ERROR;


    if (block_size == 0)
    {
        block_size = MIN_OF(INT32_MAX, MAX_STREAM_IO_SIZE);
    }

    clearerr(fp);

    if (kjb_isatty(fileno(fp)))
    {
        pause_on_next = FALSE;
        num_term_lines = 0;
        num_term_chars = 0;
    }

#ifdef MAX_IO_RESULT_IO_SIZE
    if (len > MAX_IO_RESULT_IO_SIZE)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
#endif

    if (len == 0)
    {
        if (len_ptr != NULL)
        {
            *len_ptr = 0;
        }
        return NO_ERROR;
    }

#ifdef TOO_NOISY
    UNTESTED_CODE(); /* Size code to make sure that we can read up to max of size_t bytes. */
#endif

#if 0 /* was ifdef HOW_IT_WAS */
    total_read = fread(buff, (size_t)1, len, fp);
#endif

    /* Code patterned from code provided by Praveen Rao.  */

    total_read = 0;

    while (total_read < len)
    {
        bytes_to_read = (len - total_read) > block_size ? block_size : (len - total_read);

        read_res = kjb_fread(fp, (void *)((char *) buff + total_read), bytes_to_read);

        if (read_res < 0)
        {
            break;
        }

        total_read += read_res;

        if ((size_t)read_res < bytes_to_read)
        {
            break;
        }
    }

    if (read_res < 0)
    {
        return read_res;
    }
    else
    {
        if (len_ptr != NULL)
        {
            *len_ptr = total_read;
        }
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             kjb_fwrite
 *
 * Writes a specified number of bytes to a stream
 *
 * This routine is similar to fwrite (but note the difference in arguments!).
 * However, error handling is made more consistant with the rest of the KJB
 * library. Several other KJB library IO conventions and features are also
 * implemented. (One should not mix KJB and non-KJB IO)!
 *
 * This routine can only write up to LONG_MAX bytes at a time. The routine
 * kjb_fwrite_2() can be used to write up the number of bytes that can stored in
 * size_t.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Index: output, I/O, paging
 *
 * Returns:
 *    On success kjb_write returns the number of bytes written.   On failure
 *    ERROR is returned and an error message is set.
 *
 * -----------------------------------------------------------------------------
*/

long kjb_fwrite
(
     FILE* fp          /* File pointer */,
     const void* line  /* Pointer to bytes to write */,
     size_t len        /* Number of bytes to write. */
)
{
    IMPORT volatile Bool halt_all_output;
    IMPORT volatile Bool halt_term_output;
    size_t              fwrite_res;
    int result = NO_ERROR;


    if ((len == 0) || halt_all_output) return 0;

    if (len > MAX_STREAM_IO_SIZE)
    {
        set_error("The routine kjb_fwrite can only write up to %ld bytes.\n",
                  MAX_STREAM_IO_SIZE);
        add_error("Consider using kjb_fwrite_2() instead.");
        return ERROR;
    }

    if (fp == stderr)
    {
        ERE(kjb_fflush((FILE*)NULL));
    }

    if ((fp != stdout) || (fs_enable_stdout))
    {
        if (kjb_isatty(fileno(fp)))
        {
            if (halt_term_output) return 0;
            result = term_put_n_chars((const char *)line, len);
        }
        else
        {
            fwrite_res = fwrite((const void*)line,(size_t)1,
                                (size_t)len, fp);

            if (fwrite_res < len)
            {
                UNTESTED_CODE(); /* Error handling has not been validated. */

                set_error("Write to %F failed. (%lu bytes out of %lu written)",
                          fp, (unsigned long)fwrite_res,
                          (unsigned long)len);
                add_error("%S", fp);
                result = ERROR;
            }
            else
            {
                result = fwrite_res;
            }
        }
    }

    if (    ((fp == stdout) && (fs_stdout_shadow_fp != NULL))
         || ((fp == stderr) && (fs_stderr_shadow_fp != NULL))
       )
    {
        FILE* shadow_fp         = (fp == stdout) ? fs_stdout_shadow_fp : fs_stderr_shadow_fp;
        size_t   shadow_fwrite_res = fwrite((const void*)line,(size_t)1, (size_t)len, shadow_fp);

        if (shadow_fwrite_res < len)
        {
            UNTESTED_CODE(); /* Error handling has not bee validated. */

            if (result == ERROR)
            {
                add_error("Write to %F also failed. (%ld bytes out of %ld written)",
                          shadow_fp, len, shadow_fwrite_res);
                add_error("%S", fp);
                result = ERROR;
            }
            else
            {
                set_error("Write to %F failed. (%ld bytes out of %ld written)",
                          shadow_fp, len, shadow_fwrite_res);
                add_error("%S", fp);
                result = ERROR;
            }
        }
    }

    if (fp_get_path_type(fp) != PATH_IS_REGULAR_FILE) 
    {
        kjb_fflush(fp); 
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             kjb_fwrite_2
 *
 * Writes a specified number of bytes to a stream
 *
 * This routine is similar to fwrite (but note the difference in arguments!).
 * However, error handling is made more consistant with the rest of the KJB
 * library. Several other KJB library IO conventions and features are also
 * implemented. (One should not mix KJB and non-KJB IO)!
 *
 * The number of bytes that are written are returned in len_ptr. This can be set
 * to NULL if you are not interested.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Index: output, I/O, paging
 *
 * Returns:
 *    On success kjb_write returns the number of bytes written.   On failure
 *    ERROR is returned and an error message is set.
 *
 * -----------------------------------------------------------------------------
*/

int kjb_fwrite_2
(
    FILE*       fp,      /* File pointer */
    const void* buff,    /* Pointer to bytes to write */
    size_t      len,     /* Number of bytes to write. */
    size_t*     len_ptr  /* Number of bytes written. */
)
{
#ifdef PRAVEEN_WAY
    size_t block_size = PAGE_SIZE * 100000; /* 100MB */
#else
    /*
     * Paranoid. It does not seem that we can rely on writes bigger than 2GB
     * even on 64 bit machines? Or is something else to blame?
     *
    size_t block_size = PAGE_SIZE * (MAX_STREAM_IO_SIZE / PAGE_SIZE);
    */
    size_t block_size = PAGE_SIZE * (MIN_OF(INT32_MAX, MAX_STREAM_IO_SIZE) / PAGE_SIZE);
#endif
    size_t bytes_to_write;
    long   write_res = NO_ERROR;
    size_t total_written = 0;


    if (block_size == 0)
    {
        block_size = MIN_OF(INT32_MAX, MAX_STREAM_IO_SIZE);
    }

    /* Code patterned from code provided by Praveen Rao.  */

    while (total_written < len)
    {
        bytes_to_write = (len - total_written) > block_size ? block_size : (len - total_written);

        write_res = kjb_fwrite(fp, (const void *)((const char *) buff + total_written), bytes_to_write);

        if (write_res < 0) break;

        total_written += write_res;

        ERE(kjb_fflush(fp));
    }

    if (len_ptr != NULL)
    {
        *len_ptr = total_written;
    }

    if (write_res < 0)
    {
        return write_res;
    }
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_safe_fflush
 *
 * KJB library replacement for fflush, but not as strict as kjb_fflush.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Index: output, I/O
 *
 * -----------------------------------------------------------------------------
*/
int kjb_safe_fflush
(
     FILE* fp  /* File pointer, or NULL to flush all open files.*/
)
{
    /*
    File static
        int fs_cached_file_reference_count[ MAX_NUM_NAMED_FILES ];
    */


    if (fs_cached_file_reference_count[ 0 ] != NOT_SET)
    {
        return kjb_fflush(fp);
    }
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_fflush
 *
 * KJB library replacement for fflush.
 *
 * This routine is an anolog of the system routine fflush. It implements
 * additional error reporting and also implements taking a NULL argument to
 * flush all open files. This is available on many systems, but here we
 * implement our own symantics. Specifically we flush stderr LAST.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Warning:
 *    It is critical than any file pointer passed to kjb_fflush was obtained
 *    from this library using kjb_fopen and friends.
 *
 * Index: output, I/O, paging
 *
 * -----------------------------------------------------------------------------
*/

/*
// Even though many systems implement fflush(NULL), we want to impose our own
// symantics on this. This is because fflush(NULL) does not specify the order
// of the flushing, and we want stderr to be last (for sure!).
*/
#define IMPLEMENT_FLUSH_ALL


int kjb_fflush
(
     FILE* fp /* File pointer, or NULL to flush all open files. */
)
{


    ERE(initialize_cache());


    if (fp == NULL)
    {
#ifdef IMPLEMENT_FLUSH_ALL
        int i;
        Bool error_flag = FALSE;

        /*
        // We do stderr last, stdout second last. Thus do the others first.
        */

        for (i=3; i<MAX_NUM_NAMED_FILES; i++)
        {
            if (    (fs_cached_fp[ i ] != NULL)
                 && (OPEN_FOR_WRITING(fs_cached_mode[ i ]))
               )
            {
                if (fflush(fs_cached_fp[ i ]) == -1)
                {
                    if ( ! error_flag)
                    {
                        kjb_clear_error();
                        error_flag = TRUE;
                    }
                    add_error("Flush of %F failed.%S", fs_cached_fp[ i ]);
                }
            }
        }

        for (i=0; i<3; i++)
        {
            if (    (fs_cached_fp[ i ] != NULL)
                 && (OPEN_FOR_WRITING(fs_cached_mode[ i ]))
               )
            {
                if (fflush(fs_cached_fp[ i ]) == -1)
                {
                    if ( ! error_flag)
                    {
                        kjb_clear_error();
                        error_flag = TRUE;
                    }
                    add_error("Flush of %F failed.%S", fs_cached_fp[ i ]);
                }
            }
        }

        if (error_flag) return ERROR;
#else
        if (fflush(fp) == -1)
        {
            set_error("Attempt to flush all open files failed.%S");
            return ERROR;
        }
#endif
        return NO_ERROR;
    }
    else if (fflush(fp) == -1)
    {
        set_error("Flush of %F failed.%S", fp);
        return ERROR;
    }
    else if (    (fp == stdout)
              && (fs_stdout_shadow_fp != NULL)
              && (fflush(fs_stdout_shadow_fp) == -1)
            )
    {
        set_error("Flush of %F failed.%S", fs_stdout_shadow_fp);
        return ERROR;
    }
    else if (    (fp == stderr)
              && (fs_stderr_shadow_fp != NULL)
              && (fflush(fs_stderr_shadow_fp) == -1)
            )
    {
        set_error("Flush of %F failed.%S", fs_stderr_shadow_fp);
        return ERROR;
    }
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_ioctl
 *
 * Front end for ioctl
 *
 * The function kjb_ioctl is very similar to the standard ioctl, except that
 * error reporting is made consistent with the other library routines, and the
 * error messages are more meaningfull.
 *
 * Returns:
 *    On failure an error message is set, and ERROR is returned. Otherwise
 *    the result of the underlying ioctl call is retuned.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

#ifdef UNIX

int kjb_ioctl(int file_des, IOCTL_REQUEST_TYPE request, void* arg)
{
    int result;


    result = ioctl(file_des, request, (caddr_t)arg);

    if (result == -1)
    {
        set_error("Control of device failed.%S");
        return ERROR;
    }
    else
    {
        return result;
    }
}

#else     /* Matches #ifdef UNIX .... */
/* -----------------------------------------------------------------------------
|                            Non UNIX code
|                                  ||
|                                 \||/
|                                  \/
*/


/*ARGSUSED*/
int kjb_ioctl(int file_des, IOCTL_REQUEST_TYPE  request, void* arg)
{

    UNTESTED_CODE(); 
    set_error("Unable to control defive with descripter %D.", file_des);
    add_error("Calling a system routine currently not wrapped in %s.",
              SYSTEM_NAME_TWO);
    return ERROR;
}
#endif  /* #ifdef UNIX .... #else ....                            */
        /* (Ends handling of system dependent natures of "ioctl") */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX

/*
 * =============================================================================
 *                              set_blocking
 *
 * Sets a device into blocking mode on subsequent reads.
 *
 * Note:
 *    Often better to use "poll". This routine is intended to be used where poll
 *    is not quite the right thing.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *    On failure an error message is set, and ERROR is returned. Otherwise
 *    NO_ERROR is retuned.
 *
 * Related:
 *    term_set_blocking, term_set_no_blocking
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
 */

int set_blocking(int des)
{
#ifdef OLD_SYSV_NON_BLOCKING
    extern int fs_cached_is_blocking[ MAX_NUM_NAMED_FILES ];
#endif
    int        ioctl_arg;


    if ((des < 0) || (des >= MAX_NUM_NAMED_FILES))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

#ifdef OLD_SYSV_NON_BLOCKING
    ERE(initialize_cache());
#endif

#ifdef BSD
    ioctl_arg = 0;
    ERE(kjb_ioctl(des, (IOCTL_REQUEST_TYPE)FIONBIO, (void*)&ioctl_arg));
#else
    if ((ioctl_arg = fcntl(des, F_GETFL, 0)) < 0)
    {
        set_error("Fctl F_GETFL failed.%S");
        return ERROR;
    }

    ioctl_arg &= ~O_NONBLOCK;     /* Posix     */

    /*
    // OLD SYSV way. Best avoided if possible.
    //
    //       ioctl_arg &= ~O_NDELAY;
    */

    if (fcntl(des, F_SETFL, ioctl_arg) < 0)
    {
        set_error("Fctl F_SETFL failed.%S");
        return ERROR;
    }
#endif

#ifdef OLD_SYSV_NON_BLOCKING
    fs_cached_is_blocking[ des ] = TRUE;
#endif

    return NO_ERROR;
}


#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef UNIX
/*
 * =============================================================================
 *                                set_no_blocking
 *
 * Sets a device into non blocking mode on subsequent reads.
 *
 * Note:
 *    Often better to use "poll". This routine is intended to be used where poll
 *    is not quite the right thing.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *    On failure an error message is set, and ERROR is returned. Otherwise
 *    NO_ERROR is retuned.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

int set_no_blocking(int des)
{
#ifdef OLD_SYSV_NON_BLOCKING
    extern int fs_cached_is_blocking[ MAX_NUM_NAMED_FILES ];
#endif
    int        ioctl_arg;


    if ((des < 0) || (des >= MAX_NUM_NAMED_FILES))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

#ifdef OLD_SYSV_NON_BLOCKING
    ERE(initialize_cache());
#endif

#ifdef BSD
    ioctl_arg = 1;
    ERE(kjb_ioctl(des, (IOCTL_REQUEST_TYPE)FIONBIO, (void*) & ioctl_arg));
#else
    if ((ioctl_arg = fcntl(des, F_GETFL, 0)) < 0)
    {
        set_error("Fctl F_GETFL failed.%S");
        return ERROR;
    }

    /*
    // Using both breaks HPUX and who knows what else!
    */
    ioctl_arg |= O_NONBLOCK;     /* Posix     */
#if 0 /* was ifdef DONT_USE_EXCEPT_ON_SPECIFIC_SYSTEM */
    ioctl_arg |= O_NDELAY;       /* NOT Posix */
#endif

    if (fcntl(des, F_SETFL, ioctl_arg) < 0)
    {
        set_error("Fctl F_SETFL failed.%S");
        return ERROR;
    }
#endif

#ifdef OLD_SYSV_NON_BLOCKING
    fs_cached_is_blocking[ des ] = FALSE;
#endif

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef OLD_SYSV_NON_BLOCKING

/*
 * =============================================================================
 *                                is_blocking
 *
 * Returns effect of last set_blocking or set_no_blocking
 *
 * This routine returns the effect of the last call to set_blocking or
 * set_no_blocking. It does not attempt to determine whether the device
 * actually will block on reads (although this is possible on some/all systems,
 * we wish to avoid this overhead, and the trouble required to code the
 * capability). If all setting and unsetting of blocking is done through
 * set_blocking and set_no_blocking, then the result can be trusted.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *    On failure an error message is set, and ERROR is returned. Otherwise
 *    is_blocking returns either TRUE or FALSE.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

int is_blocking(des)
    int des;
{
    extern int fs_cached_is_blocking[ MAX_NUM_NAMED_FILES ];


    if ((des < 0) || (des >= MAX_NUM_NAMED_FILES))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(initialize_cache());

    return (fs_cached_is_blocking[ des ]);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                stdin_get_line
 *
 * Reads a line from stdin into a null terminated string.
 *
 * This routine is convenent for reading from stdin. If stdin is a terminal, it
 * calls term_get_line, in which case the prompt is used (if it is not NULL). 
 * If stdin is not a terminal, then the prompt is ignored, and this routine
 * works just like get_line (i.e., like calling fget-line with stdin as the
 * first argument.  
 *
 * Returns:
 *    On success fget_line returns the number of bytes placed in the buffer,
 *    excluding the NULL.  Since the new line chracter is consumed but not
 *    copied, zero is returned for null lines. Alternate return values are all
 *    negative. In the case of end of file, EOF returned. If the line was
 *    truncated, then TRUNCATED is returned. Depending on the signal traps and
 *    options in place, INTERRUPTED is an additional possible return value. In
 *    the case of non-blocking reads WOULD_BLOCK is returned unless a complete
 *    line can be returned. ERROR is returned for unexpected problems and an
 *    error message is set.
 *
 * Macros:
 *    BUFF_STDIN_GET_LINE(const char* prompt, char line[])
 *
 *    The max_len parameter is set to sizeof(line) with the appropriate cast.
 *    Using sizeof to set the buffer size is recomended where applicable, as
 *    the code will not be broken if the buffer size changes. HOWEVER, neither
 *    this method, nor the macro, is  applicable if line is NOT a character
 *    array.  If line is declared by "char* line", then the size of line is the
 *    number of bytes in a character pointer (usually 8), which is NOT what is
 *    normally intended. You have been WARNED!
 *
 * Index: input, I/O
 *
 * -----------------------------------------------------------------------------
*/
long stdin_get_line(const char* prompt_str, char* line, size_t max_len)
{
    
    if (kjb_isatty(fileno(stdin)))
    {
        return term_get_line((prompt_str == NULL) ? "" : prompt_str,
                             line, max_len);
    }
    else 
    {
        return fget_line(stdin, line, max_len);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                fget_line
 *
 * Reads a line from a stream into a null terminated string.
 *
 * This routine is similar to fgets(3C).  It reads a null terminated string
 * representing one line from the file/device represented by the parameter "fp"
 * into the buffer pointed to by the parameter "line". The new line character
 * is consumed but not returned. At most max_len characters are placed into
 * "line" including the NULL. If there are more than max_len-1 characters on
 * the line, the extra ones are purged, and a truncated version of the line is
 * left in the buffer and TRUNCATED is returned. The maximum number of
 * characters that can be read is also limited by an internal buffer. If the
 * number of characters exceed the size of the internal buffer, then TRUNCATED
 * is also returned. Since fget_line is intended for reading text, extremely
 * long lines are not expected. The exact limit is given by GET_LINE_BUFF_SIZE
 * which is defined in l_sys_io.h. An internal buffer is needed to get the proper
 * behaviour on non-blocking reads.
 *
 * If the device corresponding to fp has been set non-blocking then this
 * routine will return WOULD_BLOCK until an entire line can be returned.
 *
 * Similarly, if EOF is read before the new line is reached, then EOF is
 * returned until an entire line can be read. Thus if a separate process is
 * writing to a file, and this routine is used to read that file, then a loop
 * on EOF will read the file by lines, which can simplify parsing.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Note:
 *    Since the number of characters is limted by the internal buffer which is
 *    less than LONG_MAX, there will not be overflow in the return of the number
 *    of bytes. This routine is meant for reading relatively short ascii lines.
 *
 * Note:
 *    The behaviour on non-blocking and EOF assumes that all calls in a
 *    sequence trying to read a given line are made with the same file pointer
 *    (there is only one buffer). This is only relavent for situations where
 *    one is actively waiting for additional input to arrive (due to some other
 *    process).
 *
 * Note:
 *    This routine is not designed for performance. The assumption is that
 *    line at a time reads correspond to small amounts of I/O.
 *
 * Returns:
 *    On success fget_line returns the number of bytes placed in the buffer,
 *    excluding the NULL.  Since the new line chracter is consumed but not
 *    copied, zero is returned for null lines. Alternate return values are all
 *    negative. In the case of end of file, EOF returned. If the line was
 *    truncated, then TRUNCATED is returned. Depending on the signal traps and
 *    options in place, INTERRUPTED is an additional possible return value. In
 *    the case of non-blocking reads WOULD_BLOCK is returned unless a complete
 *    line can be returned. ERROR is returned for unexpected problems and an
 *    error message is set.
 *
 * Macros:
 *    BUFF_FGET_LINE(FILE fp, char line[])
 *
 *    The max_len parameter is set to sizeof(line) with the appropriate cast.
 *    Using sizeof to set the buffer size is recomended where applicable, as
 *    the code will not be broken if the buffer size changes. HOWEVER, neither
 *    this method, nor the macro, is  applicable if line is NOT a character
 *    array.  If line is declared by "char* line", then the size of line is the
 *    number of bytes in a character pointer (usually 8), which is NOT what is
 *    normally intended. You have been WARNED!
 *
 * Index: input, I/O
 *
 * -----------------------------------------------------------------------------
*/

/*
// Previously fget_line was simply defined in terms of dget_line as below.
// However, mixing stream and non-stream IO is bad practice at best, and
// in this case broke some code when it was ported to HPUX. Nonetheless,
// fget_line and dget_line are intended to be very similar, and thus
// changes should be synched.
//
// Old (bad) code.
//
// long fget_line(fp, line, max_len)
//   FILE* fp;
//   char* line;
//   long max_len;
//    {
//
//
//     return dget_line(fileno(fp), line, max_len);
//    }
//
*/

long fget_line(FILE* fp, char* line, size_t max_len)
{
#ifndef errno  /* GNU defines errno as a macro, leading to warning messages. */
    IMPORT int    errno;
#endif
    static char*  buff_pos                   = NULL;
    static char   buff[ GET_LINE_BUFF_SIZE ];
    static Bool   line_truncated_flag        = FALSE;
    static size_t i                          = 0;
    long          read_res                   = 0;


#ifdef SIZE_T_IS_32_BITS
    if (max_len > INT32_MAX)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
#endif

    if (max_len == 0) return 0;

#ifdef TEST
#ifdef PROGRAMMER_is_kobus
    if (max_len == sizeof(char*))
    {
        warn_pso("Suspicious buffer length (%lu) at line %d of %s.\n",
                 (unsigned long)max_len, __LINE__, __FILE__);
        kjb_optional_abort();
    }
#endif 
#endif 

    max_len--;

    if (max_len > sizeof(buff))
    {
        max_len = sizeof(buff);
    }

    if (buff_pos == NULL)
    {
        buff_pos = buff;
        i=0;
        line_truncated_flag = FALSE;
    }

    while (i < max_len)
    {
        read_res = fread(buff_pos, (size_t)1, (size_t)1, fp);

        if (read_res > 0)
        {
            if (*buff_pos == '\n') 
            {
                break;
            }
            else
            {
                buff_pos++;
                i++;
            }
        }
        else if (read_res == 0)     /* Case EOF */
        {
#ifdef OLD_SYSV_NON_BLOCKING
            if (is_blocking(fileno(fp)))
            {
                return WOULD_BLOCK;
            }
#endif
            /*
            // CHANGED  01/07/99
            //
            // WAS
            //    break;
            */

            /*
            // CHANGED  again 14/11/00
            //
            // WAS
            //    clearerr(fp);
            //    return EOF;
            //
            // Basically the question is what to do with the last chunk of text
            // in the file which may not have a line terminator. My current
            // feeling is to return it as though there was a line feed, and so
            // EOF is only trigured if the EOF is reached with no other input
            // having been read.
            */

            if (i == 0)
            {
                clearerr(fp);
                return EOF;
            }
            else
            {
                break;
            }
        }
#ifdef UNIX
        else if ((read_res == EOF) &&
                 (errno == NON_BLOCKING_READ_FAILURE_ERROR_NUM))
        {
            return WOULD_BLOCK;
        }
#endif
        else if ((read_res == EOF) && (errno == EINTR))
        {
            return INTERRUPTED;
        }
        else
        {
            set_error("Read of %F failed.%S\n", fp);
            buff_pos = NULL;
            return ERROR;
        }
    }

    /*
    // if we have hit max_len or more, we check for a read that would overflow
    // the buffer.
    */

    if (i >= max_len)
    {
        char read_buff[ 1 ];

        read_res = fread(read_buff, (size_t)1, (size_t)1, fp);

        if (    (read_res > 0)
             && (read_buff[ 0 ] != '\n')
           )
        {
            line_truncated_flag = TRUE;
        }

        /*
         * Swallow left over. 
        */
        while (    (read_res > 0)
                && (read_buff[ 0 ] != '\n')
              )
        {
            read_res = fread(read_buff, (size_t)1, (size_t)1, fp);
        }

        if (read_res >= 0)
        {
            /*EMPTY*/
            ; /* Do nothing */
        }
        else if (read_res != EOF)
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }
        else if (errno == EINTR)
        {
            return INTERRUPTED;
        }
#ifdef UNIX
        else if (errno == NON_BLOCKING_READ_FAILURE_ERROR_NUM)
        {
            return WOULD_BLOCK;
        }
#endif 
        else
        {
            set_error("Read failed on %F.%S", fp);
            buff_pos = NULL;
            return ERROR;
        }

        /*
        // Despite first appearances, "i" could be greater than max_len if max_len
        // changed between non-blocking reads and we fell through. (Not likely, but
        // possible). 
        */
        i = max_len; 
    }

    /*
    // "i" is at most max_len which is one less than what max_len started as.
    */

    /*
    //  CHANGED, 14/11/00
    //
    //  WAS
    //      kjb_strncpy(line, buff, i + ROOM_FOR_NULL);
    //
    //  Changed because this fools the trap in kjb_strncpy which checks for bad
    //  buffer sizes. Also, what do we want to do when there are NULLs in the
    //  file? The previous would not copy them. The current will copy them,
    //  but normally they would not get seen. It is a small point because the
    //  routine is meant for ASCII, but the action on bad data must be
    //  considered.
    */

    kjb_memcpy(line, buff, i);

    /*
    // HACK to deal with DOS format.
    */
    if ((i > 0) && (line[ i - 1 ] == '\r'))
    {
        i--;
    }

    line[ i ] = '\0';

    /*
    // Prepare for next read. Next time all statics will be reset.
    */
    buff_pos = NULL;

    if (line_truncated_flag)
    {
        return TRUNCATED;
    }
    else if (i > 0)
    {
        return i;
    }
    else if (read_res == 0)
    {
        return EOF;
    }
    else
    {
        return 0;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                            dget_line
 *
 * Gets a line into a null terimated string from a device.
 *
 * The routine dget_line reads a null terminated string representing one line
 * from the device designated by the descriptor "des" into the buffer pointed to
 * by the parameter "line". The new line character is consumed but not returned.
 * At most max_len characters are placed into "line" including the NULL. If
 * there are more than max_len-1 characters on the line, then the extra are
 * purged, , and a truncated version of the line is left in the buffer and
 * TRUNCATED is returned.   The maximum number of characters that can be read is
 * also limited by an internal buffer. If the number of characters exceed the
 * size of the internal buffer, then TRUNCATED is also returned. Since dget_line
 * is intended for reading text, extremely long lines are not expected. The
 * exact limit is given by GET_LINE_BUFF_SIZE which is defined in l_sys_io.h. An
 * internal buffer is needed to get the proper behaviour on non-blocking reads.
 *
 * If the device corresponding to des has been set non-blocking then this
 * routine will return WOULD_BLOCK until an entire line can be returned.
 *
 * Similarly, if EOF is read before the new line is reached, then EOF is
 * returned until an entire line can be read. Thus if a separate process is
 * writing to a file, and this routine is used to read that file, then a loop
 * on EOF will read the file by lines, which can simplify parsing.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Note:
 *    Since the number of characters is limted by the internal buffer which is
 *    less than LONG_MAX, there will not be overflow in the return of the number
 *    of bytes. This routine is meant for reading relatively short ascii lines.
 *
 * Note:
 *    The behaviour on non-blocking and EOF assumes that all calls in a
 *    sequence trying to read a given line are made with the same file pointer
 *    (there is only one buffer). This is only relavent for situations where
 *    one is actively waiting for additional input to arrive (due to some other
 *    process).
 *
 * Note:
 *    This routine is not designed for performance. The assumption is that
 *    line at a time reads correspond to small amounts of I/O.
 *
 * Returns:
 *    On success dget_line returns the number of bytes placed in the buffer.
 *    Since the new line chracter is consumed but not copied, the return may be
 *    zero for null lines. Alternate return values are all negative. In the
 *    case of end of file, EOF returned. If the line was truncated, then
 *    TRUNCATED is returned. Depending on the signal traps and options in
 *    place, INTERRUPTED is an additional possible return value. In the case of
 *    non-blocking reads WOULD_BLOCK is returned unless a complete line can be
 *    returned. ERROR is returned for unexpected problems and an error message
 *    is set.
 *
 * Macros:
 *    BUFF_DGET_LINE(FILE fp, char line[])
 *
 *    The max_len parameter is set to sizeof(line) with the appropriate cast.
 *    Using sizeof to set the buffer size is recomended where applicable, as
 *    the code will not be broken if the buffer size changes. HOWEVER, neither
 *    this method, nor the macro, is  applicable if line is NOT a character
 *    array.  If line is declared by "char* line", then the size of line is the
 *    number of bytes in a character pointer (usually 4), which is NOT what is
 *    normally intended.  You have been WARNED!
 *
 * Index: input, I/O
 *
 * -----------------------------------------------------------------------------
*/

/*
// Changes to dget_line should be synchronized with analogous changes in
// fget_line (if applicable).
*/

long dget_line(int des, char* line, size_t max_len)
{
#ifndef errno  /* GNU defines errno as a macro, leading to warning messages. */
    IMPORT int    errno;
#endif
    static char*  buff_pos                   = NULL;
    static char   buff[ GET_LINE_BUFF_SIZE ];
    static Bool   line_truncated_flag        = FALSE;
    static size_t i                          = 0;
    int           read_res                   = 0;


#ifdef SIZE_T_IS_32_BITS
    if (max_len > INT32_MAX)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
#endif

    if ((des < 0) || (des >= MAX_NUM_NAMED_FILES))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (max_len == 0)
    {
        return 0;
    }

    max_len--;

    if (max_len > sizeof(buff))
    {
        max_len = sizeof(buff);
    }

    if (buff_pos == NULL)
    {
        buff_pos = buff;
        i=0;
        line_truncated_flag = FALSE;
    }

    while (i < max_len)
    {
        read_res = read(des, buff_pos, (size_t)1);

        if (read_res > 0)
        {
            if (*buff_pos == '\n')
            {
                break;
            }

            buff_pos++;
            i++;
        }
        else if (read_res == 0)     /* Case EOF */
        {
#ifdef OLD_SYSV_NON_BLOCKING
            if (is_blocking(des))
            {
                return WOULD_BLOCK;
            }
#endif
            /*
            // CHANGED  01/07/99
            //
            break;
            */
            return EOF;
        }
#ifdef UNIX
        else if ((read_res == EOF) &&
                 (errno == NON_BLOCKING_READ_FAILURE_ERROR_NUM))
        {
            return WOULD_BLOCK;
        }
#endif 
        else if ((read_res == EOF) && (errno == EINTR))
        {
            return INTERRUPTED;
        }
        else
        {
            set_error("Read of %D failed.%S\n", des);
            buff_pos = NULL;
            return ERROR;
        }
    }

    /*
    // Despited first appearances, "i" could be greater than max_len if max_len
    // changed between non-blocking reads and we fell through. (Not likely, but
    // possible).
    */

    if (i >= max_len)
    {
        char read_buff[ 1 ];

        read_res = read(des, read_buff, (size_t)1);

        if ((read_res > 0) && (read_buff[ 0 ] != '\n'))
        {
            line_truncated_flag = TRUE;
        }

        while ((read_res > 0) && (read_buff[ 0 ] != '\n'))
        {
            read_res = read(des, read_buff, (size_t)1);
        }

        if (read_res -= 0)
        {
            return EOF;
        }
        else if (read_res > 0)
        {
            /*EMPTY*/
            ; /* Do nothing */
        }
        else if (read_res != EOF)
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }
        else if (errno == EINTR)
        {
            return INTERRUPTED;
        }
#ifdef UNIX
        else if (errno == NON_BLOCKING_READ_FAILURE_ERROR_NUM)
        {
            return WOULD_BLOCK;
        }
#endif 
        else
        {
            set_error("Read failed on %D.%S", des);
            buff_pos = NULL;
            return ERROR;
        }
    }

    /*
    // "i" is at most max_len which is one less than what max_len started as.
    */

    kjb_strncpy(line, buff, i + ROOM_FOR_NULL);

    /*
    // Prepare for next read. Next time all statics will be reset.
    */
    buff_pos = NULL;

    if (line_truncated_flag)
    {
        return TRUNCATED;
    }
    else if (i > 0)
    {
        return i;
    }
    else if (read_res == 0)
    {
        return EOF;
    }
    else
    {
        return 0;
    }
}

/*
 * =============================================================================
 *                                   fput_line
 *
 * Writes a line to a file.
 *
 * Fput_line writes a string to a stream. A new line character is then appended.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *    On success, the number of characters written is returned. On failure,
 *    an error is set, and ERROR is returned.
 *
 * Index: I/O, output
 *
 * -----------------------------------------------------------------------------
*/

long fput_line
(
    FILE* fp          /* Stream, assumed open for write or append */ ,
    const char* line  /* Null terminated string to output */
)
{
    int res;
    int res2;


    ERE(res = kjb_fputs(fp, line));
    ERE(res2 = kjb_fputs(fp, "\n"));

    /* Parnoid! */
    if (res == INT_MAX)
    {
        set_error("INT_MAX + 1 too large for return value of fput_line.");
        add_error("The write itself appears succussful.");
        return ERROR;
    }
    else
    {
        return (res + res2);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_mkdir
 *
 * KJB library version of mkdir(2).
 *
 * Makes the directory pointed to by dir_path. Unlike mkdir(2), there is no
 * mode argument. For now, the directory will be made with the usual permission
 * (RWX everybody) & ~ (umask). (The permission stuff has only been checked on
 * a few systems).
 *
 * Note:
 *    Unlike mkdir(2) and kjb_mkdir_2(), this routine succeeds if the directory
 *    already exists. Also unlike mkdir(2) on many systems, this routine
 *    recursivley makes subdirectories. See kjb_mkdir_2 for a simple wrapper for
 *    mkdir(2).
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

/*
// FIX
//
// Implement the permission semantics described above.
*/

int kjb_mkdir(const char* dir_path)
{
#ifndef errno  /* GNU defines errno as a macro, leading to warning messages. */
    IMPORT int errno;
#endif
    int        mkdir_res = -1;
    char       sub_dir_path[ MAX_FILE_NAME_SIZE ];
    char       sub_dir[ MAX_FILE_NAME_SIZE ];
    const char*      dir_path_pos = dir_path;
    int count = 0;

    sub_dir_path[ 0 ] = '\0';

    while (*dir_path_pos == DIR_CHAR)
    {
        sub_dir_path[ count ] = *dir_path_pos;
        count++;
        dir_path_pos++;
    }

    sub_dir_path[ count ] = '\0';

    while (BUFF_CONST_GEN_GET_TOKEN(&dir_path_pos, sub_dir, DIR_STR) > 0)
    {
        BUFF_CAT(sub_dir_path, sub_dir);

#ifdef UNIX
#ifdef _MODE_T
        mkdir_res = mkdir(sub_dir_path, (mode_t)(S_IRWXU | S_IRWXG | S_IRWXO));
#else
        mkdir_res = mkdir(sub_dir_path, (S_IRWXU | S_IRWXG | S_IRWXO));
#endif
#else
#ifdef MS_OS
        mkdir_res = mkdir(sub_dir_path);
#endif
#endif
        /* Ranjini : Debug message*/
        if ((mkdir_res == -1) && (errno != EEXIST))
        {
           if (errno == EACCES)
           {
               char details[4096];
               long ct = kjb_sprintf(
                       details,
                       sizeof(details),
                       "kjb_mkdir(): error EACCES encountered.\n"
                       "kjb_mkdir(): It is likely that the process "
                                     "will continue despite this error.\n"
                       "kjb_mkdir(): It occurs sometimes on NFS directories.\n"
                       "kjb_mkdir(): dir_path == %s\n"
                       "kjb_mkdir(): sub_dir_path == %s\n",
                       dir_path,
                       sub_dir_path
                   );
               if (ERROR == ct)
               {
                   set_error("kjb_mkdir() EACCES, cannot generate warning");
                   return ERROR;
               }
               warn_pso(details);
           }  
           else
           {
               set_error("This is neither a directory access nor directory "
                     "exists error. Unable to make directory %s.%S", dir_path);
               return ERROR;
           }
        }

        BUFF_CAT(sub_dir_path, DIR_STR);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_mkdir_2
 *
 * Another KJB library version of mkdir(2).
 *
 * Makes the directory pointed to by dir_path. Unlike mkdir(2), there is no
 * mode argument. For now, the directory will be made with the usual permission
 * (RWX everybody) & ~ (umask). (The permission stuff has only been checked on
 * a few systems).
 *
 * Note:
 *    See kjb_mkdir for a similar routine which succeeds even if the directory
 *    exists, and makes needed subdirectories.
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

/*
// FIX
//
// Implement the permission semantics described above.
*/

int kjb_mkdir_2(const char* dir_path)
{
    int mkdir_res = -1;


#ifdef UNIX
#ifdef _MODE_T
    mkdir_res = mkdir(dir_path, (mode_t)(S_IRWXU | S_IRWXG | S_IRWXO));
#else
    mkdir_res = mkdir(dir_path, (S_IRWXU | S_IRWXG | S_IRWXO));
#endif
#else
#ifdef MS_OS
    mkdir_res = mkdir(dir_path);
#endif
#endif

    if (mkdir_res == -1)
    {
        set_error("Unable to make directory %s.%S", dir_path);
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_unlink
 *
 * KJB library version of unlnk(2)
 *
 * The main point of this routine is that error reporting is made consistent
 * with the rest of the library. In addition, if file_name is NULL, or if it is
 * a null string (IE, *file_name == '\0'), then this is NO-OP, returning
 * success.
 *
 * Returns:
 *    ERROR on failure (with error message set), and NO_ERROR on success.
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int kjb_unlink(const char* file_name)
{
    Error_action save_error_action;
    char         real_path[ MAX_FILE_NAME_SIZE ];


    if ((file_name == NULL) ||  (*file_name == '\0')) return NO_ERROR;

    save_error_action = get_error_action();
    set_error_action(IGNORE_ERROR_ON_ERROR);

    if (BUFF_REAL_PATH(file_name, real_path) == ERROR)
    {
        BUFF_CPY(real_path, file_name);
    }

    set_error_action(save_error_action);

    if (unlink(real_path) == EOF)
    {
        set_error("Unable to remove %s.%S\n", real_path);
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_unlink_2
 *
 * Calls kjb_unlink with path constructed from a directory and file
 *
 * This routine calls kjb_unlink with the path constructed from the directory
 * argument and the file_name argument. If directory is NULL, then it is not
 * used.
 *
 * Returns:
 *    ERROR on failure (with error message set), and NO_ERROR on success.
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int kjb_unlink_2(const char* dir, const char* file_name)
{

    char path[ MAX_FILE_NAME_SIZE ];

    if (dir == NULL)
    {
        path[ 0 ] = '\0';
    }
    else
    {
        BUFF_CPY(path, dir);
        BUFF_CAT(path, DIR_STR);
        BUFF_CAT(path, file_name);
    }

    return kjb_unlink(path);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_rmdir
 *
 * KJB library version of rmdir(2)
 *
 * The main point of this routine is that error reporting is made consistent
 * with the rest of the library.
 *
 * Returns:
 *    ERROR on failure (with error message set), and NO_ERROR on success.
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int kjb_rmdir(const char* directory_name)
{


    if (rmdir(directory_name) == EOF)
    {
        set_error("Unable to remove directory %s.%S", directory_name);
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_fopen
 *
 * A front end for fopen
 *
 * The routine kjb_fopen should be used to open files that are used with the KJB
 * library.  It implements additional features such as more comprehensive
 * diagnostics, and sytem independent determination of file name from the file
 * pointer. It also implements acceptance of the "b" (for binary) mode
 * character.  (This is STILL not universally available). Another feature is a
 * routine print_open_files (development version only!) to print all open files.
 * An additional feature is that "~", and "~users"" is expanded. Of course, this
 * means that you cannot open a file that starts with a double ~.
 *
 * If the user settable option "disable-dir-open" is set, then an open attempt
 * on a directory failes.
 *
 * If the user settable option "uncompress-files" is set, and the filenanme has
 * suffix ".gz" or ".Z", then the file is uncompressed into tempory space, and
 * the temporary file is opened instead.  The temporary file is removed on file
 * close.
 *
 * Files opened with kjb_fopen MUST be closed with kjb_fclose.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Warning:
 *     Many KJB library routines requiring a file pointer will assume that this
 *     pointer came from one of kjb_fopen, kjb_freopen, kjb_popen, or
 *     kjb_fdopen.  Furthermore the library assumes that all files opened with
 *     one of these routines will be closed with kjb_fclose.   Furthermore, the
 *     KJB library file IO routines assume that stdin, stdout, and stderr will
 *     not be closed with fclose (kjb_fclose is OK). In short, if one makes use
 *     of the IO portion of the library, then kjb_fopen and friends should be
 *     used.
 *
 *     Using file pointers obtained from kjb_fopen with non KJB library
 *     routines is less critical, but can lead to minor problems such as
 *     incorrect paging. Of course, when there is no non-KJB library routine
 *     for the given task, then there is no problem.
 *
 * Returns:
 *     A regular FILE* handle on success, and NULL on failure.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

FILE* kjb_fopen(const char* input_fd_name, const char* mode)
{
    FILE* fp;


    if (input_fd_name == NULL)
    {
        set_error("Failed to open a NULL file.");
        return NULL;
    }

    fp = kjb_fopen_2(input_fd_name, mode);

    /*
     * Additional paths to try on failure would be implemented here. Right now
     * there are none that are useful to us, but it seems likely that some will
     * be added.
    */
#if 0 /* was ifdef OBSOLETE_BUT_KEEP_AS_EXAMPLE_CODE */
    /*
    // We put this here, after the error message has been set, so that the error
    // message is for the first attempt.
    //
    // Currenly we only do the exchange if the user has entered a complete path
    // as this is the only practical case.
    */
    if (    (fp == NULL)
         && (fs_exchange_usr_and_net)
         && (input_fd_name[ 0 ] == '/'))
    {
        char exchange_path[ MAX_FILE_NAME_SIZE ];


        exchange_path[ 0 ] = '\0';

        if (HEAD_CMP_EQ(input_fd_name, "/usr/local-"))
        {
            BUFF_CPY(exchange_path, "/net/local-");
        }
        else if (HEAD_CMP_EQ(input_fd_name, "/net/local-"))
        {
            BUFF_CPY(exchange_path, "/usr/local-");
        }

        if (exchange_path[ 0 ] != '\0')
        {
            const char* path_pos = input_fd_name + 11;

            BUFF_CAT(exchange_path, path_pos);

            fp = kjb_fopen_2(exchange_path, mode);

            if (fp == NULL)
            {
                add_error("Also tried %q.", exchange_path);
            }
        }
    }
#endif

    return fp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* kjb_fopen_2 is not re-entrant. */
static FILE* kjb_fopen_2(const char* input_fd_name, const char* mode)
{
    FILE* fp;
    char  system_dependent_mode[ 100 ];
    char  unix_mode[ 100 ];
    char gz_fd_name[ MAX_FILE_NAME_SIZE ];
    char Z_fd_name[ MAX_FILE_NAME_SIZE ];
    char error_on_entry[ 5000 ];
    char fopen_error[ 5000 ];
    int  mode_res = NOT_SET;


    gz_fd_name[ 0 ] = '\0';
    Z_fd_name[ 0 ] = '\0';


    fopen_error[ 0 ] = '\0';

    /* Save error because cannot simply add errors. */
    error_on_entry[ 0 ] = '\0';
    kjb_get_error(error_on_entry, sizeof(error_on_entry));

    push_error_action(SET_ERROR_ON_ERROR);

    mode_res = process_file_open_mode(mode,
                                      system_dependent_mode,
                                      sizeof(system_dependent_mode),
                                      unix_mode,
                                      sizeof(unix_mode));

    if (mode_res == ERROR)
    {
        fp = NULL;
    }
    else
    {
        fp = kjb_fopen_3(input_fd_name, mode);
    }

    if (fp == NULL)
    {
        fopen_error[ 0 ] = '\0';
        kjb_get_error(fopen_error, sizeof(fopen_error));
    }

    if (    (fp == NULL)
         && (mode_res != ERROR)
         && (fs_uncompress_files)
         && (get_mode_for_cache(unix_mode) == MODE_IS_READ)
       )
    {
        BUFF_CPY(gz_fd_name, input_fd_name);
        BUFF_CAT(gz_fd_name, ".gz");

        fp = kjb_fopen_3(gz_fd_name, mode);
    }

    if (    (fp == NULL)
         && (mode_res != ERROR)
         && (fs_uncompress_files)
         && (get_mode_for_cache(unix_mode) == MODE_IS_READ)
       )
    {
        BUFF_CPY(Z_fd_name, input_fd_name);
        BUFF_CAT(Z_fd_name, ".Z");

        fp = kjb_fopen_3(Z_fd_name, mode);
    }

    if (fp == NULL)
    {
        str_set_error(fopen_error);

        if (gz_fd_name[ 0 ] != '\0')
        {
            add_error("Also tried %s.", gz_fd_name);
        }

        if (Z_fd_name[ 0 ] != '\0')
        {
            add_error("Also tried %s.", Z_fd_name);
        }

        kjb_get_error(fopen_error, sizeof(fopen_error));
    }

    str_set_error(error_on_entry);

    pop_error_action();

    str_set_error(fopen_error);

    return fp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* kjb_fopen_3 is not re-entrant. */
static FILE* kjb_fopen_3(const char* input_fd_name, const char* mode)
{
    FILE* fp = NULL;
    char  system_dependent_mode[ 100 ];
    char  unix_mode[ 100 ];
    char  expanded_path[ MAX_FILE_NAME_SIZE ];
    char  fd_name[ MAX_FILE_NAME_SIZE ];
    size_t        expanded_path_len;
    char          uncompressed_temp_name[ MAX_FILE_NAME_SIZE ];
    int result = NO_ERROR;


    uncompressed_temp_name[ 0 ] = '\0';

    ERN(process_file_open_mode(mode,
                               system_dependent_mode,
                               sizeof(system_dependent_mode),
                               unix_mode,
                               sizeof(unix_mode)));

    ERN(BUFF_EXPAND_PATH(expanded_path, input_fd_name));

    /* If we are doing something other than read, then there is no special
    // treatement at this stage.
    */
    if (get_mode_for_cache(unix_mode) != MODE_IS_READ)
    {
        BUFF_CPY(fd_name, expanded_path);
    }
    /* We are reading someithing which exists. We will decompress it if the
    // appropriate option is set, and it has a compression suffix. Otherwise,
    // we will just read it.
    */
    else if (get_path_type(expanded_path) == PATH_IS_REGULAR_FILE)
    {
        expanded_path_len = strlen(expanded_path);

        if (    (fs_uncompress_files)
             && (expanded_path_len > 3)
             && (    (    (expanded_path[ expanded_path_len - 3 ] == '.')
                       && (expanded_path[ expanded_path_len - 2 ] == 'g')
                       && (expanded_path[ expanded_path_len - 1 ] == 'z')
                     )
                  ||
                     (    (expanded_path[ expanded_path_len - 2 ] == '.')
                       && (expanded_path[ expanded_path_len - 1 ] == 'Z')
                     )
                )
           )
        {
            char        command[ 20 + 2 * MAX_FILE_NAME_SIZE ];
            const char* suffix = (expanded_path[ expanded_path_len - 1 ] == 'z')
                                                                    ? ".gz" : ".Z";
            char        compressed_temp_name[ MAX_FILE_NAME_SIZE ];

            compressed_temp_name[ 0 ] = '\0';

            BUFF_GET_TEMP_FILE_NAME(uncompressed_temp_name);
            BUFF_CPY(compressed_temp_name, uncompressed_temp_name);
            BUFF_CAT(compressed_temp_name, suffix);

            ERN(kjb_sprintf(command, sizeof(command), "/bin/cp %s %s",
                            expanded_path, compressed_temp_name));
            ERN(kjb_system(command));

            result = kjb_sprintf(command, sizeof(command), "gunzip %s",
                                 compressed_temp_name);

            if (result != ERROR)
            {
                verbose_pso(5, "Uncompressing a copy of %s as %s.\n",
                            expanded_path, uncompressed_temp_name);
                result = kjb_system(command);
            }

            if (result != ERROR)
            {
                verbose_pso(5, "Uncompression seems to have succeeded.\n");
                BUFF_CPY(fd_name, uncompressed_temp_name);
            }
            else
            {
                (void)kjb_unlink(compressed_temp_name);  /* Just in case. */
            }

        }
        else
        {
            BUFF_CPY(fd_name, expanded_path);
        }
    }
    else
    {
        BUFF_CPY(fd_name, expanded_path);
    }

    if (result != ERROR)
    {
        fp = fopen(fd_name, system_dependent_mode);
    }

    if (fp == NULL)
    {
        char mode_for_error_message[ 100 ];


        get_mode_for_error_message(unix_mode, mode_for_error_message,
                                   sizeof(mode_for_error_message));

        set_error("Unable to open file %q for %s.%S",
                  fd_name, mode_for_error_message);
    }

    if (fp != NULL)
    {
        if (fs_disable_dir_open && (fp_get_path_type(fp) == PATH_IS_DIRECTORY))
        {
            set_error("%q is a directory.", expanded_path);
            (void)fclose(fp);
            fp = NULL;
        }
        else if (cache_file_name(fp, expanded_path, input_fd_name,
                                 uncompressed_temp_name,
                                 unix_mode)
                 == ERROR)
        {
            (void)fclose(fp);
            fp = NULL;
        }
    }

    if (    (fp == NULL)
         && (uncompressed_temp_name[ 0 ] != '\0')
       )
    {
        kjb_unlink(uncompressed_temp_name);
    }

    return fp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_freopen
 *
 * A replacement for freopen
 *
 * See kjb_fopen for details
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *     A regular FILE* handle on success, and NULL on failure.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

FILE* kjb_freopen(const char* fd_name, const char* mode, FILE* stream)
{
    FILE* fp;
    char  system_dependent_mode[ 100 ];
    char  unix_mode[ 100 ];
    char  expanded_path[ MAX_FILE_NAME_SIZE ];


    ERN(process_file_open_mode(mode,
                               system_dependent_mode,
                               sizeof(system_dependent_mode),
                               unix_mode,
                               sizeof(unix_mode)));

    ERN(BUFF_EXPAND_PATH(expanded_path, fd_name));

    fp = freopen(expanded_path, system_dependent_mode, stream);

    if (fp == NULL)
    {
        char mode_for_error_message[ 100 ];


        get_mode_for_error_message(unix_mode, mode_for_error_message,
                                   sizeof(mode_for_error_message));

        set_error("Unable to open file %q for %s.%S",
                  fd_name, mode_for_error_message);
    }
    else
    {
        if (cache_file_name(fp, expanded_path, fd_name, (char*)NULL, unix_mode) == ERROR)
        {
            fclose(fp);
            fp = NULL;
        }
    }

    return fp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_fdopen
 *
 * Replacement for fdopen
 *
 * See kjb_fopen for details.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *     A regular FILE* handle on success, and NULL on failure.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

/*ARGSUSED*/
FILE* kjb_fdopen(FILE* old_fp, const char* mode)
{
    FILE* new_fp;
    char  system_dependent_mode[ 100 ];
    char  unix_mode[ 100 ];
    int   old_file_des;
    int   new_file_des;


    ERN(initialize_cache());

    old_file_des = fileno(old_fp);

    if (    (old_file_des < MAX_NUM_NAMED_FILES)
         && (fs_cached_mode[ old_file_des ] == MODE_IS_NOT_SET)
       )
    {
        set_bug(
              "kjb_fdopen sent a file descriptor not opened with KJB library.");
        return NULL;
    }

    ERN(process_file_open_mode(mode,
                               system_dependent_mode,
                               sizeof(system_dependent_mode),
                               unix_mode,
                               sizeof(unix_mode)));

    new_file_des = dup(fileno(old_fp));

    if (new_file_des == EOF)
    {
        /*
         * FIX
         *
         * Should check for failure due to mismatch of modes, and call
         * set_bug (not set_error) in this case.
        */

        set_error("Unable to duplicate a file descriptor for %F.%S", old_fp);
        return NULL;
    }

    new_fp = fdopen(new_file_des, system_dependent_mode);

    if (new_fp == NULL)
    {
        set_error("Unable to open a stream for a duplicate descriptor of %F.%S",
                  old_fp);
        return NULL;
    }

    if (    (new_file_des < MAX_NUM_NAMED_FILES)
         && (old_file_des < MAX_NUM_NAMED_FILES)
       )
    {
        fs_cached_fnames[ new_file_des ] =
                                kjb_strdup(fs_cached_fnames[ old_file_des ]);

        fs_cached_user_fnames[ new_file_des ] =
                                kjb_strdup(fs_cached_user_fnames[ old_file_des ]);

        fs_cached_file_reference_count[ new_file_des ] = 1;
        fs_cached_fp[ new_file_des ] = new_fp;

        /*
        // New mode will be "compatible" with the old, but I don't think it is
        // gaurenteed to be identical;
        */

        fs_cached_mode[ new_file_des ] = get_mode_for_cache(unix_mode);
    }

    return new_fp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                                kjb_fclose
 *
 * Replacement for fclose
 *
 * This function should be used to close files opened with a KJB library routine
 * such as kjb_fopen. File pointers obtained elsewhere should NOT be sent to
 * kjb_fclose. If kjb_fclose is sent a null file pointer, then this routine is a
 * successful no-op.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *    ERROR on failure (with error message set), and NO_ERROR on success.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

int kjb_fclose(FILE* fp)
{
    int result = NO_ERROR;
    int file_des;


    if (fp == NULL)
    {
        return NO_ERROR;
    }

    file_des = fileno(fp);

    /*
    // If we need to close one of stdin, stdout, stderr, then we should make
    // routines called kjb_fclose(stdin), etc, so the following error check can
    // remain.
    */
    if (file_des < 3)
    {
        set_bug("Attempt to close file with descriptor %d (%D).", file_des, 
                file_des);
        return ERROR;
    }

    if (fclose(fp) == EOF)
    {
        set_error("Close of file %F failed.%S", fp);
        result = ERROR;
    }

    if (uncache_file_name(file_des) == ERROR)
    {
        result = ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
#ifndef __C2MAN__



/* =============================================================================
 *                                kjb_popen
 *
 * A front end for popen
 *
 * The routine kjb_popen should be used in place of popen for streams that are
 * that are used with the KJB library.   kjb_popen implements additional
 * features such as more comprehensive diagnostics.
 *
 * The current implementation uses popen, so it shares the problem that the
 * open can succeed even if the execution of the command fails.
 *
 * Streams opened with kjb_popen MUST be closed with kjb_pclose.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Warning:
 *     Many KJB library routines requiring a file pointer will assume that this
 *     pointer came from one of kjb_fopen, kjb_freopen, kjb_popen, or
 *     kjb_fdopen, Furthermore the library assumes that all files opened with
 *     one of these routines will be closed with kjb_fclose. Furthermore, the
 *     KJB library file IO routines assume that stdin, stdout, and stderr will
 *     not be closed with fclose (kjb_fclose is OK). In short, if one makes use
 *     of the IO portion of the library, then kjb_fopen and friends should be
 *     used.
 *
 *     Using file pointers obtained from kjb_fopen with non KJB library
 *     routines is less critical, but can lead to minor problems such as
 *     incorrect paging. Of course, when there is no non-KJB library routine
 *     for the given task, then there is no problem.
 *
 * -----------------------------------------------------------------------------
*/

FILE* kjb_popen(const char* cmd, const char* mode)
{
    FILE* fp;
    char  system_dependent_mode[ 100 ];
    char  unix_mode[ 100 ];
    char  name_to_cache[ 1000 ];


    ERN(process_file_open_mode(mode,
                               system_dependent_mode,
                               sizeof(system_dependent_mode),
                               unix_mode,
                               sizeof(unix_mode)));

    BUFF_CPY(name_to_cache, "pipe to ");
    BUFF_CAT(name_to_cache, cmd);

    fp = popen(cmd, system_dependent_mode);

    if (fp == NULL)
    {
        char mode_for_error_message[ 100 ];

        get_mode_for_error_message(unix_mode, mode_for_error_message,
                                   sizeof(mode_for_error_message));

        set_error("Unable to open pipe to %q with mode %q.%S",
                  cmd, mode_for_error_message);
    }
    else
    {
        if (cache_file_name(fp, (char*)NULL, name_to_cache, (char*)NULL, unix_mode) == ERROR)
        {
            (void)fclose(fp);
            fp = NULL;
        }
    }

    return fp;
}

#endif   /* NOT building documentation */
#endif   /* TEST */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

/*
// Currently only used in test programs.
*/

/* =============================================================================
 *                                kjb_pclose
 *
 * Replacement for pclose
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * This function should be used to close streams opened with kjb_popen File
 * pointers obtained elsewhere should NOT be sent to kjb_pclose.
 *
 * -----------------------------------------------------------------------------
*/

int kjb_pclose(FILE* fp)
{
    int result;
    int file_des;


    result = NO_ERROR;


    file_des = fileno(fp);

    if (pclose(fp) == EOF)
    {
        set_error("Close of file %F failed.%S", fp);
        result = ERROR;
    }

    if (uncache_file_name(file_des) == ERROR)
    {
        result = ERROR;
    }

    return result;
}

#endif   /* TEST */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      uncache_file_name
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * -----------------------------------------------------------------------------
*/

static int uncache_file_name(int file_des)
{


    if ((file_des < 0) || (file_des >= MAX_NUM_NAMED_FILES))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (fs_cached_file_reference_count[ 0 ] == NOT_SET)
    {
        set_bug("uncache_file_name called when cache was invalid.");
        return ERROR;
    }
    else if (fs_cached_mode[ file_des ] == MODE_IS_NOT_SET)
    {
        set_bug("uncache_file_name recieved fp not opened by KJB library");
        return ERROR;
    }
    else
    {
        fs_cached_file_reference_count[ file_des ] --;
    }

    if (fs_cached_file_reference_count[ file_des ] == 0)
    {
        kjb_free(fs_cached_fnames[ file_des ] );
        fs_cached_fnames[ file_des ] = NULL;

        kjb_free(fs_cached_user_fnames[ file_des ] );
        fs_cached_user_fnames[ file_des ]=NULL;

        if (fs_cached_uncompressed_fnames[ file_des ] != NULL)
        {
            EPE(kjb_unlink(fs_cached_uncompressed_fnames[ file_des ]));
            kjb_free(fs_cached_uncompressed_fnames[ file_des ] );
            fs_cached_uncompressed_fnames[ file_des ]=NULL;
        }

        fs_cached_mode[ file_des ] = MODE_IS_NOT_SET;
        fs_cached_fp[ file_des ]   = NULL;

        fs_cached_isatty[ file_des ] = NOT_SET;

#ifdef OLD_SYSV_NON_BLOCKING
        fs_cached_is_blocking[ file_des ] = FALSE;
#endif
    }
#ifdef TEST
    else if (fs_cached_file_reference_count[ file_des ] > 0)
    {
        kjb_fprintf(stderr, "Reference count for %d reduced to %d.\n", file_des,
                    fs_cached_file_reference_count[ file_des ]);
    }
    else
    {
        kjb_fprintf(stderr, "Reference count for %d reduced to %d.\n", file_des,
                    fs_cached_file_reference_count[ file_des ]);

        kjb_fprintf(stderr,
                    "Presumably an attempt to close the same stream twice.\n");

        kjb_abort();
    }
#endif

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      get_mode_for_error_message
 * -----------------------------------------------------------------------------
*/

/*
// We assume that we are being sent a unix style mode. EG, the "b" designators
// have been removed where appropriate.
*/

static void get_mode_for_error_message(char* mode, char* buff, size_t len)
{


    if (STRCMP_EQ(mode, "r"))
    {
        kjb_strncpy(buff, "reading", len);
    }
    else if (STRCMP_EQ(mode, "w"))
    {
        kjb_strncpy(buff, "writing", len);
    }
    else if (STRCMP_EQ(mode, "w+"))
    {
        kjb_strncpy(buff, "seek/write", len);
    }
    else if (STRCMP_EQ(mode, "r+"))
    {
        kjb_strncpy(buff, "read/write", len);
    }
    else if (STRCMP_EQ(mode, "a+"))
    {
        kjb_strncpy(buff, "read/append", len);
    }
    else if (STRCMP_EQ(mode, "a"))
    {
        kjb_strncpy(buff, "append", len);
    }
    else
    {
        kjb_strncpy(buff, mode, len);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                         cache_file_name
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * -----------------------------------------------------------------------------
*/

static int cache_file_name(FILE* fp, const char* expanded_path, const char* user_file_name, const char* uncompressed_fd_name, const char* mode)
{
    int          file_des;
#ifdef VMS
    char         buff[ 200 ];
    char*        buff_ptr;
#else
    char         real_path [ MAXPATHLEN ];
#endif

    if (fp == NULL)
    {
        set_bug("Null fp passed to cache_file_name.");
        return ERROR;
    }

    file_des = fileno(fp);

    if (file_des >= MAX_NUM_NAMED_FILES)
    {
        set_error("Number of open files cannot exceed %d.",
                  MAX_NUM_NAMED_FILES);
        return ERROR;
    }

    ERE(initialize_cache());

#ifdef OLD_SYSV_NON_BLOCKING
    fs_cached_is_blocking[ file_des ] = TRUE;
#endif

    /*
     * It is possible to have more than one stream attached to
     * an file descriptor, and in this case the reference
     * count will be greater than 0. This does not
     * happen in any code as of 10/02/96.
    */

    if (fs_cached_file_reference_count[file_des] == 0)
    {
        /*
         * If this is a re-open, then we are in some sense,
         * "re-naming" the stream. The system has done the close
         * on the previous stream for us, and thus the cleanup code
         * in kjb_fclose has not been executed. If the strings are not
         * null for any other reason, then there is a mistake. (?)
        */
        if (fs_cached_fnames[ file_des ] != NULL)
        {
            kjb_free( fs_cached_fnames[ file_des ] );
        }

        if (fs_cached_user_fnames[ file_des ] != NULL)
        {
            kjb_free( fs_cached_user_fnames[ file_des ] );
        }

        if (kjb_isatty(fileno(fp)))
        {
            fs_cached_fnames[ file_des ] = kjb_strdup("the computer terminal");
            fs_cached_user_fnames[ file_des ] =
                kjb_strdup("the computer terminal");
            fs_cached_isatty[ file_des ] = TRUE;
            fs_cached_uncompressed_fnames[ file_des ] = NULL;
        }
        else
        {
            fs_cached_isatty[ file_des ] = FALSE;
#ifdef VMS
            buff_ptr = getname(file_des, buff);
            fs_cached_fnames[file_des ] = kjb_strdup(buff);
#else
            if (expanded_path != NULL)
            {
                ERE(BUFF_REAL_PATH(expanded_path, real_path));
                fs_cached_fnames[file_des ] = kjb_strdup(real_path);
            }
            else
            {
                fs_cached_fnames[file_des ] = kjb_strdup(user_file_name);
            }
#endif
            fs_cached_user_fnames[file_des ] = kjb_strdup(user_file_name);

            if (    (uncompressed_fd_name != NULL)
                 && (uncompressed_fd_name[ 0 ] != '\0')
               )
            {
                fs_cached_uncompressed_fnames[ file_des ] =
                                       kjb_strdup(uncompressed_fd_name);
            }
        }

        fs_cached_fp[ file_des ] = fp;
        fs_cached_mode[ file_des ] = get_mode_for_cache(mode);

    }
#ifdef TEST
    else
    {
        kjb_fprintf(stderr,
                    "Duplicate descriptor (%d) noted in cache_file_name.\n",
                    file_des);
        kjb_fprintf(stderr, "This occured while attempting to cache %s\n",
                    user_file_name);
    }
#endif

    fs_cached_file_reference_count[ file_des ] ++;

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                        initialize_cache
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * -----------------------------------------------------------------------------
*/

/*
// Important note: We assume that if the functions in this file are being
// used at all, then they are not being mixed with the standard ones in
// devious ways. Specifically, we assume that stdin, stdout, and stderr,
// have NOT been closed with fclose (kjb_fclose is OK).
*/

static int initialize_cache(void)
{
    static int   cache_is_ready = FALSE;
    int          i;


    if ( ! cache_is_ready)
    {
        fs_cached_file_reference_count[ 0 ] = 1;
        fs_cached_mode[ 0 ] = MODE_IS_READ;
        fs_cached_fp[ 0 ] = stdin;
#ifdef OLD_SYSV_NON_BLOCKING
        fs_cached_is_blocking[ 0 ] = TRUE;
#endif

        /*
        // All allocated storage is tested for NULL together once all three
        // default file descriptors have been dealt with.
        */

        if (kjb_isatty_guts(fileno(stdin)))
        {
            fs_cached_isatty[ 0 ]      = TRUE;
            fs_cached_fnames[ 0 ]      = kjb_strdup("the computer terminal");
            fs_cached_user_fnames[ 0 ] = kjb_strdup("the computer terminal");
        }
        else
        {
            fs_cached_isatty[ 0 ]      = FALSE;
            fs_cached_fnames[ 0 ]      = kjb_strdup("standard input");
            fs_cached_user_fnames[ 0 ] = kjb_strdup("standard input");
        }

        fs_cached_uncompressed_fnames[ 0 ] = NULL;

        fs_cached_file_reference_count[ 1 ] = 1;
        fs_cached_mode[ 1 ]                 = MODE_IS_WRITE;
        fs_cached_fp[ 1 ]                   = stdout;
#ifdef OLD_SYSV_NON_BLOCKING
        fs_cached_is_blocking[ 1 ]          = TRUE;
#endif

        if (kjb_isatty_guts(fileno(stdout)))
        {
            fs_cached_isatty[ 1 ]      = TRUE;
            fs_cached_fnames[ 1 ]      = kjb_strdup("the computer terminal");
            fs_cached_user_fnames[ 1 ] = kjb_strdup("the computer terminal");
        }
        else
        {
            fs_cached_isatty[ 1 ]      = FALSE;
            fs_cached_fnames[ 1 ]      = kjb_strdup("standard output");
            fs_cached_user_fnames[ 1 ] = kjb_strdup("standard output");
        }

        fs_cached_uncompressed_fnames[ 1 ] = NULL;

        fs_cached_file_reference_count[ 2 ] = 1;
        fs_cached_mode[ 2 ]                 = MODE_IS_WRITE;
        fs_cached_fp[ 2 ]                   = stderr;

#ifdef OLD_SYSV_NON_BLOCKING
        fs_cached_is_blocking[ 2 ]          = TRUE;
#endif

        if (kjb_isatty_guts(fileno(stderr)))
        {
            fs_cached_isatty[ 2 ]      = TRUE;
            fs_cached_fnames[ 2 ]      = kjb_strdup("the computer terminal");
            fs_cached_user_fnames[ 2 ] = kjb_strdup("the computer terminal");
        }
        else
        {
            fs_cached_isatty[ 2 ]      = FALSE;
            fs_cached_fnames[ 2 ]      = kjb_strdup("standard error");
            fs_cached_user_fnames[ 2 ] = kjb_strdup("standard error");
        }


        /*
        // This checks that we got memory when calling kjb_strdup.
        */
        if (    (fs_cached_fnames[ 0 ]      == NULL)
             || (fs_cached_user_fnames[ 0 ] == NULL)
             || (fs_cached_fnames[ 1 ]      == NULL)
             || (fs_cached_user_fnames[ 1 ] == NULL)
             || (fs_cached_fnames[ 2 ]      == NULL)
             || (fs_cached_user_fnames[ 2 ] == NULL)
           )
        {
            return ERROR;
        }

        for(i = 3; i<MAX_NUM_NAMED_FILES; i++)
        {
            fs_cached_fnames[ i ]               = NULL;
            fs_cached_user_fnames[ i ]          = NULL;
            fs_cached_file_reference_count[ i ] = 0;
            fs_cached_mode[ i ]                 = MODE_IS_NOT_SET;
            fs_cached_fp[ i ]                   = NULL;
            fs_cached_isatty[ i ]               = NOT_SET;
#ifdef OLD_SYSV_NON_BLOCKING
            fs_cached_is_blocking[ i ]          = TRUE;
#endif
            fs_cached_uncompressed_fnames[ i ] = NULL;
        }

        cache_is_ready = TRUE;

#ifdef TRACK_MEMORY_ALLOCATION
        add_cleanup_function(free_cached_file_names);
#endif
        return NO_ERROR;
    }
    else if (fs_cached_file_reference_count[ 0 ] == NOT_SET)
    {
        set_bug("File name cache has been invalidated.");
        return ERROR;
    }
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                         get_mode_for_cache
 * -----------------------------------------------------------------------------
*/

/*
// We assume that we are being sent a unix style mode. EG, the "b" designators
// have been removed where appropriate.
*/

static Mode get_mode_for_cache(const char* mode)
{


    if (STRCMP_EQ(mode, "r"))
    {
        return MODE_IS_READ;
    }
    else if (STRCMP_EQ(mode, "w"))
    {
        return MODE_IS_WRITE;
    }
    else if (STRCMP_EQ(mode, "w+"))
    {
        return MODE_IS_WRITE_PLUS;
    }
    else if (STRCMP_EQ(mode, "r+"))
    {
        return MODE_IS_READ_PLUS;
    }
    else if (STRCMP_EQ(mode, "a+"))
    {
        return MODE_IS_APPEND_PLUS;
    }
    else if (STRCMP_EQ(mode, "a"))
    {
        return MODE_IS_APPEND;
    }
    else
    {
        return MODE_IS_NOT_SET;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_realpath
 *
 * This wraps the standard Posix system call realpath().  The first argument is
 * the input pathname, and the second and third arguments are used for output,
 * to which a canonicalized path is written.  That means directories like "."
 * and ".." are removed from the path, extra slashes are cleaned up, and
 * symbolic links are eliminated.  E.g., input "/usr/lib/../bin/" would
 * yield "/usr/bin" as output.
 *
 * C programmers: please consider using macro BUFF_REAL_PATH instead of this
 * function.  C++ programmers may prefer to use kjb::realpath.
 *
 * Returns:
 *      This returns ERROR or NO_ERROR as appropriate.
 *
 * Macros:
 *      BUFF_REAL_PATH
 *
 * Documenter:
 *      Andrew Predoehl
 *
 * Index: I/O, standard library
 * -----------------------------------------------------------------------------
*/

/* ALL */   int kjb_realpath(const char* fd_name, char* path_buff, size_t max_len)
/* ALL */
/* ALL */
/* ALL */
/* ALL */   {
#ifdef UNIX
/* UNIX */      char* path_res;
/* UNIX */      char  resolved_path[ MAXPATHLEN ];
/* UNIX */      char  expanded_name[ MAX_FILE_NAME_SIZE ];
#ifdef SGI_NEXT
/* SGI_NEXT */  char  absolute_path[ MAX_FILE_NAME_SIZE ];
#endif
#endif

#ifdef MAX_SIZE_T_BUFF_SIZE
/* ALL */       if (max_len > MAX_SIZE_T_BUFF_SIZE)
/* ALL */       {
/* ALL */           SET_ARGUMENT_BUG();
/* ALL */           return ERROR;
/* ALL */       }
#endif
/* ALL */       if ((max_len == 0) || (fd_name == NULL) || (path_buff == NULL))
/* ALL */       {
/* ALL */           SET_ARGUMENT_BUG();
/* ALL */           return ERROR;
/* ALL */       }
/* ALL */
/* ALL */       *path_buff = '\0';
/* ALL */
#ifdef UNIX
/*
 * On some systems, EG AIX, realpath does not touch the result if there is
 * failure. However, since we want to use the result on other systems
 * even in the case of failure, we want to make sure that we don't have
 * garbage.
 *
 * It should be noted that we are assuming that realpath either puts
 * something useful in resolved_path, or does not touch  it. A little
 * dangerous perhaps (FIX ?)
 */

/* UNIX */      resolved_path[ 0 ] = '\0';
/* UNIX */
/* UNIX */      ERE(BUFF_EXPAND_PATH(expanded_name, fd_name));
/* UNIX */
#ifdef SGI_NEXT
/* Sgi's realpath is slow, but use it for now.                 */
/* Also, if requires READ access to all directories. Oh well.  */

/* SGI_NEXT */
/* SGI_NEXT */  if (*expanded_name == '/')
/* SGI_NEXT */  {
/* SGI_NEXT */      path_res = realpath(expanded_name, resolved_path);
/* SGI_NEXT */  }
/* SGI_NEXT */  else
/* SGI_NEXT */  {
/* SGI_NEXT */      NRE(getwd(absolute_path));
/* SGI_NEXT */      BUFF_CAT(absolute_path, "/");
/* SGI_NEXT */      BUFF_CAT(absolute_path, expanded_name);
/* SGI_NEXT */      path_res = realpath(absolute_path, resolved_path);
/* SGI_NEXT */  }
#else
/* UNIX */      path_res = realpath(expanded_name, resolved_path);
#endif
/* UNIX */
/* UNIX */      /* We copy the resolved path on BOTH success and failure */
/* UNIX */      /* as even on failure it contains valuable information.  */
/* UNIX */      /* The system version of "realpath" contains the absolute */
/* UNIX */      /* path of the unresolved componant. Normally this is the */
/* UNIX */      /* path which x would have if x was created exactly now, */
/* UNIX */      /* assuming that x could be created now. */
/* UNIX */
/* UNIX */      if (max_len < (strlen(resolved_path) + 1))
/* UNIX */      {
/* UNIX */          SET_BUFFER_OVERFLOW_BUG();
/* UNIX */          return ERROR;
/* UNIX */      }
/* UNIX */      else
/* UNIX */      {
/* UNIX */          char* resolved_path_pos = resolved_path;
/* UNIX */
/* UNIX */          if (HEAD_CMP_EQ(resolved_path, "/.automount/"))
/* UNIX */          {
/* UNIX */              resolved_path_pos += strlen("/.automount");
/* UNIX */          }
/* UNIX */          kjb_strncpy(path_buff, resolved_path_pos, max_len);
/* UNIX */      }
/* UNIX */
/* UNIX */      if (path_res != NULL)
/* UNIX */      {
/* UNIX */          return NO_ERROR;
/* UNIX */      }
/* UNIX */      else
/* UNIX */      {
/* UNIX */          set_error("Unable to find %q.%S", fd_name);
/* UNIX */          return ERROR;
/* UNIX */      }
#else
/* default */   if (max_len < strlen(fd_name)+1)
/* default */   {
/* default */       SET_BUFFER_OVERFLOW_BUG();
/* default */       return ERROR;
/* default */   }
/* default */   else
/* default */   {
/* default */       kjb_strncpy(path_buff, fd_name, max_len);
/* default */       return NO_ERROR;
/* default */   }
#endif
/* ALL */   }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                        expand_path
 * -----------------------------------------------------------------------------
*/

/* ALL */   static int expand_path(char* exanded_path, const char* path, size_t max_len)
/* ALL */
/* ALL */
/* ALL */
/* ALL */   {
#ifdef UNIX
/* UNIX */      const char*    path_pos;
/* UNIX */      struct passwd* pw;
/* UNIX */      char           logname_buff[ 200 ];
#endif
/* ALL */       size_t         exanded_path_len;
/* ALL */
/* ALL */
#ifdef MAX_SIZE_T_BUFF_SIZE
/* ALL */       if (max_len > MAX_SIZE_T_BUFF_SIZE)
/* ALL */       {
/* ALL */           SET_ARGUMENT_BUG();
/* ALL */           return ERROR;
/* ALL */       }
#endif
/* ALL */       if ((max_len == 0) || (path == NULL) || (exanded_path == NULL))
/* ALL */       {
/* ALL */           SET_ARGUMENT_BUG();
/* ALL */           return ERROR;
/* ALL */       }
/* ALL */
#ifdef UNIX
/* UNIX */      if (*path == '~')
/* UNIX */      {
/* UNIX */          path_pos = path;
/* UNIX */          path_pos++;
/* UNIX */
/* UNIX */          if ((*path_pos == '/') || (*path_pos == '\0'))
/* UNIX */          {
/* UNIX */              ERE(BUFF_GET_USER_ID(logname_buff));
/* UNIX */          }
/* UNIX */          else
/* UNIX */          {
/* UNIX */              BUFF_CONST_GEN_GET_TOKEN(&path_pos, logname_buff, "/");
/* UNIX */          }
/* UNIX */
/* UNIX */          pw = getpwnam(logname_buff);
/* UNIX */
/* UNIX */          if (pw == NULL)
/* UNIX */          {
/* UNIX */              set_error("No user %q", logname_buff);
/* UNIX */              return ERROR;
/* UNIX */          }
/* UNIX */
/* UNIX */          exanded_path_len = strlen(pw->pw_dir) +
/* UNIX */                                        strlen(path_pos) + 1;
/* UNIX */          if (max_len < exanded_path_len)
/* UNIX */          {
/* UNIX */              SET_BUFFER_OVERFLOW_BUG();
/* UNIX */              return ERROR;
/* UNIX */          }
/* UNIX */
/* UNIX */          kjb_strncpy(exanded_path, pw->pw_dir, max_len);
/* UNIX */          kjb_strncat(exanded_path, path_pos, max_len);
/* UNIX */      }
/* UNIX */      else
/* UNIX */      {
/* UNIX */          exanded_path_len = strlen(path) + 1;
/* UNIX */
/* UNIX */          if (max_len < exanded_path_len)
/* UNIX */          {
/* UNIX */              SET_BUFFER_OVERFLOW_BUG();
/* UNIX */              return ERROR;
/* UNIX */          }
/* UNIX */
/* UNIX */          kjb_strncpy(exanded_path, path, max_len);
/* UNIX */      }
/* UNIX */
/* UNIX */      return NO_ERROR;
#else
/* default */   exanded_path_len = strlen(path) + 1;
/* default */
/* default */   if (max_len < exanded_path_len)
/* default */   {
/* default */       SET_BUFFER_OVERFLOW_BUG();
/* default */       return ERROR;
/* default */   }
/* default */   else
/* default */   {
/* default */       kjb_strncpy(exanded_path, path, max_len);
/* default */       return NO_ERROR;
/* default */   }
#endif
/* ALL */   }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                get_fd_name
 *
 * Gets the name of a file  or device associated with a stream
 *
 * The name of the file or device associated with descripter des is copied into
 * the buffer fd_name.  Max_len gives the size of the buffer which will not be
 * overflowed.   The name returned is the "real" name of the file, and thus
 * "~", "../", etc are expanded. Also, if the file was uncompressed behind the
 * scenes, then the uncompressed version is provided. The routine
 * get_user_fd_name can be used to find the name of the file as passed to
 * kjb_fopen.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Note:
 *   The file must have been opened with kjb_fopen.
 *
 * Returns:
 *    On failure an error message is set, and ERROR is returned. Otherwise
 *    NO_ERROR is retuned.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

int get_fd_name(int des, char* fd_name, size_t max_len)
{
    int result;


#ifdef MAX_SIZE_T_BUFF_SIZE
    if (max_len > MAX_SIZE_T_BUFF_SIZE)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
#endif

    if (max_len == 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(initialize_cache());

    result = NO_ERROR;

    /*
    // Have to be able to handle descriptors that did not come from kjb_fopen
    // and friends, as it could have come from opening a pipe or other similar
    // source, and we do not have replacements for these opens.
    */

    if (des >= MAX_NUM_NAMED_FILES)
    {
        kjb_strncpy(fd_name, "<unknown>",  max_len);
    }
    else if (fs_cached_uncompressed_fnames[ des ] != NULL)
    {
        kjb_strncpy(fd_name, fs_cached_uncompressed_fnames[ des ], max_len);
    }
    else if (fs_cached_fnames[ des ] != NULL)
    {
        kjb_strncpy(fd_name, fs_cached_fnames[ des ], max_len);
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                get_user_fd_name
 *
 * Gets the name of a file associated with a stream
 *
 * The name of the file or device associated with desciptor in the parameter des
 * is copied into the buffer fd_name.  Max_len gives the size of the buffer
 * which will not be overflowed.  The name returned is the name as specified to
 * kjb_fopen and thus "~", "../", etc will not expanded if that is how the file
 * was originally provided. 
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Note:
 *   The file must have been opened with kjb_fopen.
 *
 * Returns:
 *    On failure an error message is set, and ERROR is returned. Otherwise
 *    NO_ERROR is retuned.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

int get_user_fd_name(int des, char* fd_name, size_t max_len)
{
    int result;


#ifdef MAX_SIZE_T_BUFF_SIZE
    if (max_len > MAX_SIZE_T_BUFF_SIZE)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
#endif

    if (max_len == 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(initialize_cache());

    result = NO_ERROR;

    /*
    // Have to be able to handle descriptors that did not come from kjb_fopen
    // and friends, as it could have come from opening a pipe or other similar
    // source, and we do not have replacements for these opens.
    */

    if ((des < MAX_NUM_NAMED_FILES) && (fs_cached_fnames[ des ] != NULL))
    {
        kjb_strncpy(fd_name, fs_cached_user_fnames[ des ], max_len);
    }
    else
    {
        kjb_strncpy(fd_name, "<unknown>",  max_len);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_fseek
 *
 * Moves the file pointer
 *
 * kjb_fseek is very similar to the standard fseek, except that error reporting
 * is made consistent with the other library routines, and the messages are
 * more meaningfull.
 *
 * Returns:
 *    On failure an error message is set, and ERROR is returned. Otherwise
 *    NO_ERROR is retuned.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

int kjb_fseek(FILE* fp, long int offset, int mode)
{
    int seek_res;


    seek_res = fseek(fp, offset, mode);

    if (seek_res == 0)
    {
        return NO_ERROR;
    }
    else
    {
        set_error("File positioning failed.%S");
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_ftell
 *
 * Returns the file offset
 *
 * kjb_ftell is very similar to the standard ftell, except that error reporting
 * is made consistent with the other library routines, and the error messages
 * are more meaningfull.
 *
 * Bugs:
 *     This function has not been vetted for being re-entrant / thread safe.
 *
 * Returns:
 *    On failure an error message is set, and ERROR is returned. Otherwise
 *    the file position is retuned.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

long kjb_ftell(FILE* fp)
{
    struct stat file_stats;
    long        result;


    if (kjb_isatty(fileno(fp)))
    {
        set_error("Attempt to find file position of a terminal.");
        return ERROR;
    }

    if (fstat((int)fileno(fp), &file_stats) == -1)
    {
#ifdef TEST
        set_error("Fstat system called failed for %F.%S", fp);
#else
        set_error("Can't position %F.%S", fp);
#endif
        return ERROR;
    }

#ifdef UNIX
    if (S_ISSOCK(file_stats.st_mode))
    {
        set_error("Attempt to find file position of a socket.");
        return ERROR;
    }
    if (S_ISFIFO(file_stats.st_mode))
    {
        set_error("Attempt to find file position of a pipe.");
        return ERROR;
    }
#else
#ifdef MS_OS
    if (file_stats.st_mode && S_IFCHR)
    {
        set_error("Attempt to find file position of a device.");
        return ERROR;
    }
#endif
#endif


    result = ftell(fp);

    if (result == -1)
    {
        set_error("Can't position %F.%S", fp);
        return ERROR;
    }
    else
    {
        return result;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_fputs
 *
 * Writes a null terminated string to a stream
 *
 * This is the KJB library version of fputs.   This routine works together with
 * the rest of the KJB library to implement paging and flushing of all files
 * when stderr is written to.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Warning:
 *    The that the arguments are REVERSED from fputs(3). In the KJB library
 *    routines we tend to put the file pointer first.
 *
 * Returns:
 *    On success the number of bytes written is return. On failure, ERROR is
 *    returned with an error message being set.
 *
 * Index: output, I/O, paging
 *
 * -----------------------------------------------------------------------------
*/

long kjb_fputs
(
     FILE* fp        /* File pointer */  ,
     const char* str /* String to write to file */
)
{
    IMPORT volatile Bool halt_all_output;
    IMPORT volatile Bool halt_term_output;
    long                fputs_res = NO_ERROR;


    if (halt_all_output) return NO_ERROR;

    if (fp == stderr)
    {
        ERE(kjb_fflush((FILE*)NULL));
    }

    if ((fp != stdout) || (fs_enable_stdout))
    {
        if (kjb_isatty(fileno(fp)))
        {
            if ( ! halt_term_output)
            {
                fputs_res = term_puts(str);
            }
        }
        else
        {
            fputs_res = fputs(str, fp);

            if (fputs_res == EOF)
            {
                set_error("Write to %F failed.%S\n", fp);
                fputs_res = ERROR;
            }
        }

    }

    if (    ((fp == stdout) && (fs_stdout_shadow_fp != NULL))
         || ((fp == stderr) && (fs_stderr_shadow_fp != NULL))
       )
    {
        FILE* shadow_fp         = (fp == stdout) ? fs_stdout_shadow_fp : fs_stderr_shadow_fp;
        int   shadow_res = fputs(str, shadow_fp);

        if (shadow_res == EOF)
        {
            if (fputs_res == ERROR)
            {
                add_error("Write to %F also failed.%S", shadow_fp);
            }
            else
            {
                set_error("Write to %F failed.%S", shadow_fp);
                fputs_res = ERROR;
            }
        }
    }

    if (fputs_res == ERROR)
    {
        return ERROR;
    }
    else 
    {
        size_t len = strlen(str);

#ifdef REALLY_TEST
        /*
         * Yup, this does happen. So the call to strlen() seems needed if we
         * want to reliably return the number of bytes on all platforms.
        */
        if ((unsigned long)fputs_res != len)
        {
            TEST_PSO(("Return from fputs() not the number of bytes we are trying to write\n"));
            TEST_PSO(("The string length is %lu, the return is %ld\n", len, fputs_res));
            TEST_PSO(("The string was %q.\n", str));
            TEST_PSO(("fputs() is not gaurenteed to return tbe number of bytes.\n"));
            TEST_PSO(("We are simply tracking to see when it does not.\n"));
        }
#endif
        /* Parnoid! */
        if (len == LONG_MAX)
        {
            set_error("%ul too large for return value of kjb_puts().", len);
            add_error("The write itself appears succussful.");
            return ERROR;
        }
        else
        {
            if (fp_get_path_type(fp) != PATH_IS_REGULAR_FILE) 
            {
                kjb_fflush(fp); 
            }
            return len;
        }
    }

    return fputs_res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_fgetc
 *
 * Reads a character from a stream
 *
 * kjb_getc is very similar to the standard getc, except that error reporting
 * is made consistent with the other library routines, and the error messages
 * are more meaningfull.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *     On success, the character read is returned. On end of file, EOF is
 *     returned. If the system does not restart the read after an interupt, and
 *     the read was interupted, then INTERRUPTED is returned.   On more serious
 *     failures an error message is set, and ERROR is returned.
 *
 * Index: input, I/O
 *
 * ==> This routine is not used much, as term_getc is usually what is meant.
 * -----------------------------------------------------------------------------
*/

int kjb_fgetc(FILE* fp)
{
#ifndef errno  /* GNU defines errno as a macro, leading to warning messages. */
    IMPORT int errno;
#endif
    int        save_errno;
    int        c;


    if (kjb_isatty(fileno(fp)))
    {
        return term_getc();
    }

    /*
    // FIX (someday).
    //
    // This use of errno is a bit of a HACK.
    */

    /* If we don't have error, we want to restore errno. */
    save_errno = errno;
    errno = 0;
    c = fgetc(fp);

    if  (c == EOF)
    {
        if (errno == EINTR)
        {
            return INTERRUPTED;
        }
        else if (errno != 0)
        {
            set_error("Read from file failed.%S");
            return ERROR;
        }
        else
        {
            errno = save_errno;
            return EOF;
        }
    }
    else
    {
        errno = save_errno;
        return c;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_fputc
 *
 * Write a character to a stream
 *
 * This function serves the purpose of fputc from C stdio, but it is wrapped
 * according to library convention.  Note the order of input arguments is
 * reversed!
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *     If unsuccessful, the return value is ERROR.  On a successful write to an
 *     open terminal, the return value is the number of characters written
 *     (i.e., one).  If the terminal being addressed has been halted, or if
 *     the output stream is to a file that is not a terminal, the return value
 *     is NO_ERROR.
 *
 * Documenter:
 *     Andrew Predoehl
 *
 * Index:  I/O, output, standard library
 *
 * -----------------------------------------------------------------------------
*/

int kjb_fputc(FILE* fp, int c)
{
    IMPORT volatile Bool halt_all_output;
    IMPORT volatile Bool halt_term_output;
    int                 res;
    char                char_buff[ 1 ];
    int                 result         = NO_ERROR;


    if (halt_all_output) return NO_ERROR;

    if (fp == stderr)
    {
        ERE(kjb_fflush((FILE*)NULL));
    }

    if ((fp != stdout) || (fs_enable_stdout))
    {
        if (kjb_isatty(fileno(fp)))
        {
            if (halt_term_output) return NO_ERROR;

            char_buff[ 0 ] = c;
            result = term_put_n_chars(char_buff, 1);
        }
        else
        {
            res = fputc(c, fp);

            if (res == EOF)
            {
                set_error("Write to %F failed.%S", fp);
                result = ERROR;
            }
        }
    }

    /* If we are shadowing the output, write to the shadow file now. */
    if (    ((fp == stdout) && (fs_stdout_shadow_fp != NULL))
         || ((fp == stderr) && (fs_stderr_shadow_fp != NULL))
       )
    {
        FILE* shadow_fp         = (fp == stdout) ? fs_stdout_shadow_fp : fs_stderr_shadow_fp;
        int   shadow_res = fputc(c, shadow_fp);

        if (shadow_res == EOF)
        {
            if (result == ERROR)
            {
                add_error("Write to %F also failed.%S", shadow_fp);
            }
            else
            {
                set_error("Write to %F failed.%S", shadow_fp);
                result = ERROR;
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              pso
 *
 * Writes a formatted string to stdout.
 *
 * The formating options are similar to the standard ones, except that the
 * standard ones are not very standard! Thus the KJB library  routine provides
 * some semblence of platform independent IO. Also, there are additional
 * formatting options available (see kjb_fprintf for details).
 *
 * This routine works together with the rest of the KJB library to implement
 * paging and flushing of all files when stderr is written to.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Macros:
 *    This routine is similar is spirit to the system macro "printf", and
 *    hence an alternate name "kjb_printf" is provided.
 *
 * Returns:
 *    ERROR on failure and the number of characters printed on success.
 *
 * Related:
 *    kjb_fprintf
 *
 * Index: output, I/O, paging
 *
 * -----------------------------------------------------------------------------
*/

/*PRINTFLIKE1*/
long pso(const char* format_str, ...)
{
    int     result;
    va_list ap;

    va_start(ap, format_str);

    result = kjb_vfprintf(stdout, format_str, ap);

    va_end(ap);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                              p_stderr
 *
 * Writes a formatted string to stderr.
 *
 * The formating options are similar to the standard ones, except that the
 * standard ones are not very standard! Thus the KJB library  routine provides
 * some semblence of platform independent IO. Also, there are additional
 * formatting options available (see kjb_fprintf for details).
 *
 * This routine works together with the rest of the KJB library to implement
 * paging and flushing of all files when stderr is written to.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Notes:
 *    This routine used to called "pse" in analogy with "pso", but it was
 *    found to be too easy to type the wrong one. Hence the less usual case
 *    was changed to a more obtuse name.
 *
 * Returns:
 *    ERROR on failure and the number of characters printed on success.
 *
 * Related:
 *    kjb_fprintf
 *
 * Index: output, I/O, paging
 *
 * -----------------------------------------------------------------------------
*/

/*PRINTFLIKE1*/
long p_stderr(const char* format_str, ...)
{
    int     result;
    va_list ap;

    va_start(ap, format_str);

    result = kjb_vfprintf(stderr, format_str, ap);

    va_end(ap);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_fprintf
 *
 * Writes a formatted string to a stream.
 *
 * The formating options are similar to the standard ones, except that the
 * standard ones are not very standard! Thus the KJB library routine provides
 * some semblence of platform independent IO.
 *
 * This routine works together with the rest of the KJB library to implement
 * 1) paging (controlled via user options) and 2) flushing of all files when
 * stderr is written to. This second feature reduces the confusion that
 * sometimes occur when one is expecting output from stdout before an error
 * message, but might not see it because it is still in the output buffer. We
 * always flush stdout before writting to stderr. 
 *
 * There are additional formatting options available.
 *
 * %S takes no arguement. It produces the string "System error message is: "
 * followed by the error message from the last unsuccessful system call.  
 *
 * %F takes a file pointer, and prints the file name as specified by the user
 * (or internal to the code). 
 *
 * %D does the same with a file descriptor. %P is similar to %F, except that the
 * complete path is provided instead. 
 *
 * The option %t (truncate) is similar to %s, exept that if a field width is
 * given, either by a numeric constant or a *, then if that field width would be
 * overrun, then we form a string of the form "<str> ...".  "t" is short for
 * trucated. 
 *
 * The option %q is similar to %t, except that quotes are added arround the
 * string. 
 *
 * Finally, the option %R converts an integer to the return enum symbols defined
 * as part of Return_status, or simply the integer itself, if the integer is not
 * one of those values. For example, if the integers has the value ERROR, then
 * "ERROR" is printed. This options is obviously mostly for debugging.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *    ERROR on failure and the number of characters printed on success.
 *
 * Related:
 *    pso
 *
 * Index: output, I/O, paging
 *
 * -----------------------------------------------------------------------------
*/

/*PRINTFLIKE2*/
long kjb_fprintf(FILE* fp, const char* format_str, ...)
{
    int     result;
    va_list ap;
 
    va_start(ap, format_str);

    result = kjb_vfprintf(fp, format_str, ap);

    va_end(ap);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                pdo
 *
 * Writes formatted string to a process specific file
 *
 * This routine writes formatted string file "debug-<pid>.output". It is
 * ocasionally useful when the amount of debugging output is large.  The details
 * of its use can be deduced from pso() or p_stderr().
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Related:
 *    pso, kjb_fprintf
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/

long pdo(const char* format_str, ...)
{
    va_list      ap;
    char         debug_file_name[ MAX_FILE_NAME_SIZE ];
    int          result;
    /* File static
         FILE* fs_debug_fp;
    */


    if (fs_debug_fp == NULL)
    {
        ERE(kjb_sprintf(debug_file_name, sizeof(debug_file_name),
                        "debug-%ld.output", (long)MY_PID));
        NRE(fs_debug_fp = kjb_fopen(debug_file_name, "w"));
        add_cleanup_function(close_debug_file);
    }

    va_start(ap, format_str);

    result =  kjb_vfprintf(fs_debug_fp, format_str, ap);

    va_end(ap);

    if (result != ERROR)
    {
        result = kjb_fflush(fs_debug_fp);
    }

    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                         close_debug_file
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * -----------------------------------------------------------------------------
*/

static void close_debug_file(void)
{


    EPE(kjb_fclose(fs_debug_fp));  /* Print error, as this is a void cleanup fn. */
    fs_debug_fp = NULL;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_vfprintf
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * -----------------------------------------------------------------------------
*/

/*
// FIX   We should looking at sprintf's return.
*/

long kjb_vfprintf(FILE* fp, const char* format_str, va_list ap)
{
    IMPORT volatile Bool halt_all_output;
    IMPORT volatile Bool halt_term_output;
    char                new_format_str[ MAX_FORMAT_STRING_SIZE ];
    const char*         format_str_pos;
    char*               new_format_str_pos;
    int                 format_item;
    char                type_char;
    int                 num_precision_args;
    int                 precision_arg1;
    int                 precision_arg2;
    int                 short_flag;
    int                 long_flag;
#ifdef LONG_DOUBLE_OK
    int                 long_double_flag;
#endif
    long                res                                      = 0;
    long                byte_count                               = 0;
    size_t              char_count;
    char                output_buff[ MAX_FORMAT_STRING_SIZE ];
    char                trunc_buff[ 1000 ];
    int                 comma_flag;
    int                 justify_flag;
    int                 field_width_flag;
    int                 precision_flag;
    int                 field_width_arg_flag;
    int                 field_width;
    size_t              field_size;
    int                 temp_field_width;


    if (halt_all_output || (halt_term_output && kjb_isatty(fileno(fp))))
    {
        return NO_ERROR;
    }

    /*
    // Moved to kjb_fputs and kjb_fwrite, which are the only ways this routine
    // outputs?
    //
    //  if (fp == stderr)
    //  {
    //      ERE(kjb_fflush((FILE*)NULL));
    //  }
    */

    if (*format_str == '\0') return NO_ERROR;

    while (*format_str != '\0')
    {
        format_item = FALSE;

        char_count = 0;
        format_str_pos = format_str;

        while ((!format_item) &&
               (*format_str_pos != '\0'))
        {
            if (*format_str_pos == '%')
            {
                format_item = TRUE;
            }
            else
            {
                format_str_pos++;
                char_count++;
            }
        }

        if (char_count != 0)
        {
            ERE(res = kjb_fwrite(fp, (const void*)format_str, char_count));
            byte_count += res;
            format_str += char_count;
        }

        if (format_item)  /* Can't be at string end if true */
        {
            short_flag = FALSE;
            long_flag = FALSE;
#ifdef LONG_DOUBLE_OK
            long_double_flag = FALSE;
#endif
            num_precision_args = 0;
            justify_flag = field_width_flag = precision_flag = FALSE;
            comma_flag = field_width_arg_flag = FALSE;

            new_format_str_pos = new_format_str;
            *new_format_str_pos = *format_str;
            format_str++;
            new_format_str_pos++;

            /*
             * FIX: Currently we accept stuff like "-+  -  +". Not serious,
             *      as this stuff just gets passed down to sprintf anyway.
             *      BUT we DONT look at sprintf's return!
             */
            while (    (*format_str == '-')
                       || (*format_str == ' ')
                       || (*format_str == '0')
                       || (*format_str == '#')
                       || (*format_str == '+')
                       || (*format_str == ',')
                  )
            {
                if (*format_str == '-')
                {
                    justify_flag = TRUE;
                }

                if (*format_str == ',')
                {
                    comma_flag = TRUE;
                }
                else
                {
                    *new_format_str_pos = *format_str;
                    new_format_str_pos++;
                }

                format_str++;
            }

            if (*format_str == '*')
            {
                *new_format_str_pos = *format_str;
                new_format_str_pos++;
                format_str++;

                num_precision_args++;
                field_width_flag = TRUE;
                field_width_arg_flag = TRUE;
            }
            else
            {
                if (isdigit((int)(*format_str)))
                {
                    field_width_flag = TRUE;
                }

                while (isdigit((int)(*format_str)))
                {
                    *new_format_str_pos = *format_str;
                    new_format_str_pos++;
                    format_str++;
                }
            }

            if (*format_str == '.')
            {
                *new_format_str_pos = *format_str;
                new_format_str_pos++;
                format_str++;

                if (*format_str == '*')
                {
                    *new_format_str_pos = *format_str;
                    new_format_str_pos++;
                    format_str++;

                    num_precision_args++;
                    precision_flag = TRUE;
                }
                else
                {
                    if (isdigit((int)(*format_str)))
                    {
                        precision_flag = TRUE;
                    }

                    while (isdigit((int)(*format_str)))
                    {
                        *new_format_str_pos = *format_str;
                        new_format_str_pos++;
                        format_str++;
                    }
                }
            }

            if (*format_str == 'l')
            {
                long_flag = TRUE;
                *new_format_str_pos = *format_str;
                new_format_str_pos++;
                format_str++;
            }
            else if (*format_str == 'h')
            {
                short_flag = TRUE;
                *new_format_str_pos = *format_str;
                new_format_str_pos++;
                format_str++;
            }
            else if (*format_str == 'L')
            {
#ifdef LONG_DOUBLE_OK
                long_double_flag = TRUE;
                *new_format_str_pos = *format_str;
                new_format_str_pos++;
#endif
                format_str++;
            }

            if (*format_str == '\0')
            {
                SET_FORMAT_STRING_BUG();
                return ERROR;
            }

            type_char = *format_str;
            format_str++;

            if ((type_char == 'd') || (type_char == 'i'))
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

                if (num_precision_args == 0)
                {
                    if (short_flag)
                    {
                        /*
                        // Untested since changing "short"to "int" due to gcc
                        // complaining that there may be a problem because short
                        // gets promoted to int. Fair enough, but I don't know
                        // whether this is a correct solution across all
                        // platforms.
                        */

#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       va_arg(ap, int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       va_arg(ap, int));
#endif 
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       va_arg(ap, long));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       va_arg(ap, long));
#endif 
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       va_arg(ap, int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       va_arg(ap, int));
#endif 
                    }
                }
                else if (num_precision_args == 1)
                {
                    precision_arg1 = va_arg(ap, int);

                    if (short_flag)
                    {
                        /*
                        // Untested since changing "short"to "int" due to gcc
                        // complaining that there may be a problem because short
                        // gets promoted to int. Fair enough, but I don't know
                        // whether this is a correct solution across all
                        // platforms.
                        */

#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, int));
#endif 
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, long));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, long));
#endif 
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, int));
#endif 
                    }
                }
                else if (num_precision_args == 2)
                {
                    precision_arg1 = va_arg(ap, int);
                    precision_arg2 = va_arg(ap, int);

                    if (short_flag)
                    {
                        /*
                        // Untested since changing "short"to "int" due to gcc
                        // complaining that there may be a problem because short
                        // gets promoted to int. Fair enough, but I don't know
                        // whether this is a correct solution across all
                        // platforms.
                        */

#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, int));
#endif 
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, long));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, long));
#endif 
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, int));
#endif 
                    }
                }
                else
                {
                    SET_CANT_HAPPEN_BUG();
                    return ERROR;
                }

                if (res == EOF)
                {
                    set_error("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                /*
                // FIX
                //
                // Currently, commas will not be inserted into anything other
                // than non-justified positive numbers. Note that commas do not
                // properly work with precision arguments!
                //
                // Note: This code must be synched with unsigned case, and
                //       kjb_vsprintf.
                 */
                if (comma_flag && all_digits(output_buff))
                {
                    char buff_copy[ 100 ];
                    char* buff_copy_pos = buff_copy;
                    int  len = strlen(output_buff);
                    int first_chunk = (len - 1) % 3 + 1;
                    int i;

                    i = 0;

                    while (i<first_chunk)
                    {
                        *buff_copy_pos = output_buff[ i ];
                        buff_copy_pos++;
                        i++;
                    }

                    while (i<len)
                    {
                        *buff_copy_pos++ = ',';
                        *buff_copy_pos++ = output_buff[ i++ ];
                        *buff_copy_pos++ = output_buff[ i++ ];
                        *buff_copy_pos++ = output_buff[ i++ ];
                    }

                    *buff_copy_pos = '\0';

                    BUFF_CPY(output_buff, buff_copy);
                }

                ERE(res = kjb_fputs(fp, output_buff));
            }
            else if (    (type_char == 'o') || (type_char == 'u')
                      || (type_char == 'x') || (type_char == 'X')
                    )
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

                if (num_precision_args == 0)
                {
                    if (short_flag)
                    {
                        /*
                        // Untested since changing "short"to "int" due to gcc
                        // complaining that there may be a problem because short
                        // gets promoted to int. Fair enough, but I don't know
                        // whether this is a correct solution across all
                        // platforms.
                        */

#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       va_arg(ap, unsigned int));
#endif 
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       va_arg(ap, unsigned long));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       va_arg(ap, unsigned long));
#endif 
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       va_arg(ap, unsigned int));
#endif 
                    }
                }
                else if (num_precision_args == 1)
                {
                    precision_arg1 = va_arg(ap, int);

                    if (short_flag)
                    {
                        /*
                        // Untested since changing "short"to "int" due to gcc
                        // complaining that there may be a problem because short
                        // gets promoted to int. Fair enough, but I don't know
                        // whether this is a correct solution across all
                        // platforms.
                        */

#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned int));
#endif 
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned long));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned long));
#endif 
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, unsigned int));
#endif 
                    }
                }
                else if (num_precision_args == 2)
                {
                    precision_arg1 = va_arg(ap, int);
                    precision_arg2 = va_arg(ap, int);

                    if (short_flag)
                    {
                        /*
                        // Untested since changing "short"to "int" due to gcc
                        // complaining that there may be a problem because short
                        // gets promoted to int. Fair enough, but I don't know
                        // whether this is a correct solution across all
                        // platforms.
                        */

#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned int));
#endif 
                    }
                    else if (long_flag)
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned long));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned long));
#endif 
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned int));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, unsigned int));
#endif 
                    }
                }
                else
                {
                    SET_CANT_HAPPEN_BUG();
                    return ERROR;
                }

                if (res == EOF)
                {
                    set_error("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                /*
                // FIX
                //
                // Currently, commas will not be inserted into anything other
                // than non-justified positive numbers. Note that commas do not
                // properly work with precision arguments!
                 */
                if ((type_char == 'u') && comma_flag && all_digits(output_buff))
                {
                    char buff_copy[ 100 ];
                    char* buff_copy_pos = buff_copy;
                    int  len = strlen(output_buff);
                    int first_chunk = (len - 1) % 3 + 1;
                    int i;

                    i = 0;

                    while (i<first_chunk)
                    {
                        *buff_copy_pos = output_buff[ i ];
                        buff_copy_pos++;
                        i++;
                    }

                    while (i<len)
                    {
                        *buff_copy_pos++ = ',';
                        *buff_copy_pos++ = output_buff[ i++ ];
                        *buff_copy_pos++ = output_buff[ i++ ];
                        *buff_copy_pos++ = output_buff[ i++ ];
                    }

                    *buff_copy_pos = '\0';

                    BUFF_CPY(output_buff, buff_copy);
                }

                ERE(res = kjb_fputs(fp, output_buff));
            }
            else if (    (type_char == 'e') || (type_char == 'f')
                         || (type_char == 'g') || (type_char == 'E')
                         || (type_char == 'G')
                    )
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

                if (num_precision_args == 0)
                {
#ifdef LONG_DOUBLE_OK
                    if (long_double_flag)
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       va_arg(ap, long_double));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       va_arg(ap, long_double));
#endif 
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       va_arg(ap, Always_double));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       va_arg(ap, Always_double));
#endif 
                    }
#else

#ifdef MS_OS        /* snprintf() seems to be missing. CHECK current!  */
                    res = sprintf(output_buff,
                                   new_format_str,
                                   va_arg(ap, Always_double));
#else
                    res = snprintf(output_buff,
                                   sizeof(output_buff),
                                   new_format_str,
                                   va_arg(ap, Always_double));
#endif 

#endif
                }
                else if (num_precision_args == 1)
                {
                    precision_arg1 = va_arg(ap, int);

#ifdef LONG_DOUBLE_OK
                    if (long_double_flag)
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, long_double));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, long_double));
#endif 
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, Always_double));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, Always_double));
#endif 
                    }
#else

#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                    res = sprintf(output_buff,
                                   new_format_str,
                                   precision_arg1,
                                   va_arg(ap, Always_double));
#else
                    res = snprintf(output_buff,
                                   sizeof(output_buff),
                                   new_format_str,
                                   precision_arg1,
                                   va_arg(ap, Always_double));
#endif 

#endif
                }
                else if (num_precision_args == 2)
                {
                    precision_arg1 = va_arg(ap, int);
                    precision_arg2 = va_arg(ap, int);

#ifdef LONG_DOUBLE_OK
                    if (long_double_flag)
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, long_double));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, long_double));
#endif 
                    }
                    else
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, Always_double));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, Always_double));
#endif 
                    }
#else

#ifdef MS_OS        /* snprintf() seems to be missing. CHECK current!  */
                    res = sprintf(output_buff,
                                   new_format_str,
                                   precision_arg1,
                                   precision_arg2,
                                   va_arg(ap, Always_double));
#else
                    res = snprintf(output_buff,
                                   sizeof(output_buff),
                                   new_format_str,
                                   precision_arg1,
                                   precision_arg2,
                                   va_arg(ap, Always_double));
#endif 

#endif
                }
                else
                {
                    SET_CANT_HAPPEN_BUG();
                    return ERROR;
                }

                if (res == EOF)
                {
                    set_error("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                ERE(res = kjb_fputs(fp, output_buff));
            }
            else if (type_char == 's')
            {
                if ((justify_flag) || (precision_flag) || (field_width_flag))
                {
                    *new_format_str_pos = type_char;
                    new_format_str_pos++;
                    *new_format_str_pos = '\0';

                    /*
                    // FIX
                    //
                    // Should check length of va_arg(ap, char*) and precision
                    // args where applicable in order to check for buffer
                    // over-run.
                     */
                    if (num_precision_args == 0)
                    {
#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       va_arg(ap, char*));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       va_arg(ap, char*));
#endif 
                    }
                    else if (num_precision_args == 1)
                    {
                        precision_arg1 = va_arg(ap, int);

#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, char*));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       va_arg(ap, char*));
#endif 
                    }
                    else if (num_precision_args == 2)
                    {
                        precision_arg1 = va_arg(ap, int);
                        precision_arg2 = va_arg(ap, int);

#ifdef MS_OS            /* snprintf() seems to be missing. CHECK current!  */
                        res = sprintf(output_buff,
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, char*));
#else
                        res = snprintf(output_buff,
                                       sizeof(output_buff),
                                       new_format_str,
                                       precision_arg1,
                                       precision_arg2,
                                       va_arg(ap, char*));
#endif 
                    }

                    if (res == EOF)
                    {
                        set_error("Format error in %q:%S", new_format_str);
                        return ERROR;
                    }

                    ERE(res = kjb_fputs(fp, output_buff));
                }
                else
                {
                    ERE(res = kjb_fputs(fp, va_arg(ap, char *)));
                }
            }
            else if (type_char == 'D')
            {
                BUFF_GET_USER_FD_NAME(va_arg(ap, int), output_buff);
                ERE(res = kjb_fputs(fp, output_buff));
            }
            else if (type_char == 'R')
            {
                get_status_string(va_arg(ap, int), output_buff,
                                  sizeof(output_buff));
                ERE(res = kjb_fputs(fp, output_buff));
            }
            else if (type_char == 'F')
            {
                BUFF_GET_USER_FD_NAME(fileno(va_arg(ap, FILE *)), output_buff);
                ERE(res = kjb_fputs(fp, output_buff));
            }
            else if (type_char == 'P')
            {
                BUFF_GET_FD_NAME(fileno(va_arg(ap, FILE *)), output_buff);
                ERE(res = kjb_fputs(fp, output_buff));
            }
            else if (type_char == 'S')
            {
#ifdef KJB_HAVE_STRERROR
                const char* sys_mess = strerror(errno);

                if (sys_mess != NULL)
                {
                    ERE(res = kjb_fputs(fp, "\nSystem error message is: "));
                    byte_count += res;

                    ERE(res = kjb_fputs(fp, sys_mess));
                    byte_count += res;
                }
#else
                const char*  sys_mess;
#ifndef errno  /* GNU defines errno as a macro, leading to warning messages. */
                IMPORT int   errno;
#endif
                IMPORT int   sys_nerr;
                IMPORT SYS_ERRLIST_TYPE sys_errlist[];

                if (    (errno != NO_SYSTEM_ERROR)
                        && (errno < sys_nerr)
                   )
                {
                    ERE(res = kjb_fputs(fp, "\nSystem error message is: "));
                    byte_count += res;

                    sys_mess = sys_errlist[errno];

                    ERE(res = kjb_fputs(fp, sys_mess));
                    byte_count += res;
                }
#endif
            }
            else if ((type_char == 'q') || (type_char == 't'))
            {
                field_width = sizeof(trunc_buff);

                if (num_precision_args == 1)
                {
                    temp_field_width = va_arg(ap, int);

                    if (temp_field_width < field_width)
                    {
                        field_width = temp_field_width;
                    }
                }
                else if (field_width_flag)
                {
                    *new_format_str_pos = '\0';

                    if (ss1pi(new_format_str + 1, &temp_field_width) == ERROR)
                    {
                        SET_FORMAT_STRING_BUG();
                        return ERROR;
                    }

                    ASSERT(temp_field_width >= 0);

                    if (temp_field_width < field_width)
                    {
                        field_width = temp_field_width;
                    }
                }

                if (type_char == 't')
                {
                    if (field_width < 5)
                    {
                        field_size = 5;
                    }
                    else field_size = field_width;

                    str_trunc_cpy(trunc_buff, va_arg(ap, char *), field_size);
                }
                else
                {
                    if (field_width < 7)
                    {
                        field_size = 7;
                    }
                    else field_size = field_width;


                    trunc_quote_cpy(trunc_buff, va_arg(ap, char *), field_size);
                }

                ERE(res = kjb_fputs(fp, trunc_buff));
            }
            else if (type_char == 'c')
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

                if (field_width_arg_flag)
                {
                    precision_arg1 = va_arg(ap, int);

#ifdef MS_OS        /* snprintf() seems to be missing. CHECK current!  */
                    res = sprintf(output_buff,
                                   new_format_str,
                                   precision_arg1,
                                   va_arg(ap, int));
#else
                    res = snprintf(output_buff,
                                   sizeof(output_buff),
                                   new_format_str,
                                   precision_arg1,
                                   va_arg(ap, int));
#endif 
                }
                else
                {
#ifdef MS_OS        /* snprintf() seems to be missing. CHECK current!  */
                    res = sprintf(output_buff,
                                   new_format_str,
                                   va_arg(ap, int));
#else
                    res = snprintf(output_buff,
                                   sizeof(output_buff),
                                   new_format_str,
                                   va_arg(ap, int));
#endif 
                }

                if (res == EOF)
                {
                    set_error("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                ERE(res = kjb_fputs(fp, output_buff));
            }
            else if (type_char == 'p')
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

#ifdef MS_OS    /* snprintf() seems to be missing. CHECK current!  */
                res = sprintf(output_buff, new_format_str, va_arg(ap, void*));
#else
                res = snprintf(output_buff, sizeof(output_buff), 
                               new_format_str, va_arg(ap, void*));

#endif 
                if (res == EOF)
                {
                    set_error("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                ERE(res = kjb_fputs(fp, output_buff));
            }
            else if (type_char == 'n')
            {
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

#ifdef MS_OS    /* snprintf() seems to be missing. */
                res = sprintf(output_buff, new_format_str, va_arg(ap, int*));
#else
                res = snprintf(output_buff, sizeof(output_buff), new_format_str,
                               va_arg(ap, int*));

#endif 
                if (res == EOF)
                {
                    set_error("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                ERE(res = kjb_fputs(fp, output_buff));
            }
            else if (type_char == '%')
            {
#ifdef HOW_IT_WAS_11_12_24 
                *new_format_str_pos = type_char;
                new_format_str_pos++;
                *new_format_str_pos = '\0';

#ifdef MS_OS    /* snprintf() seems to be missing. CHECK current!  */
                res = sprintf(output_buff, new_format_str);
#else
                res = snprintf(output_buff, sizeof(output_buff), new_format_str);

#endif 
                
                if (res == EOF)
                {
                    set_error("Format error in %q:%S", new_format_str);
                    return ERROR;
                }

                ERE(res = kjb_fputs(fp, output_buff));
#else
                ERE(res = kjb_fputs(fp, "%"));
#endif 
            }
            else
            {
                SET_FORMAT_STRING_BUG();
                return ERROR;
            }

            byte_count += res;
        }
    }

    return byte_count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
* =============================================================================
 *                              is_file
 *
 * Determines if the argument is a valid file
 *
 * This routines determines whether the arguement is a valid file.  It is the
 * intention that in the case of a symbolic link, this routine determines
 * whether what is pointed to by the link exists, and if so, is a file.  (This
 * works on linux).
 *
 * Returns:
 *     TRUE if "path" is a file, FALSE otherwise.
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int is_file(const char* path)
{

    return (get_path_type(path) == PATH_IS_REGULAR_FILE);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
* =============================================================================
 *                              is_directory
 *
 * Determines if the argument is a valid directory
 *
 * This routines determines whether the arguement is a valid directory. It is
 * the intention that in the case of a symbolic link, this routine determines
 * whether what is pointed to by the link exists, and if so, is a directory.
 * (This works on linux).
 *
 * Returns:
 *     TRUE if "path" is a directory, FALSE otherwise.
 *
 * Index: directorys, standard library
 *
 * -----------------------------------------------------------------------------
*/

int is_directory(const char* path)
{
    /*
    char real_path[ MAX_FILE_NAME_SIZE ];

    dbs(path);
    BUFF_REAL_PATH(path, real_path);
    dbs(real_path);

    dbi(get_path_type(path));
    dbi(get_path_type(real_path));
    */

    return (get_path_type(path) == PATH_IS_DIRECTORY);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
* =============================================================================
 *                              fp_get_path_type
 *
 * Determines type of path
 *
 * This routines determines the path type (file, directory, ...). It also is a
 * reasonable way to find out if a path exists at all.
 *
 * Returns:
 *    One of { PATH_DOES_NOT_EXIST,  PATH_IS_REGULAR_FILE, PATH_IS_DIRECTORY,
 *             PATH_IS_LINK, PATH_IS_PIPE, PATH_IS_SOCKET, PATH_IS_BLOCK_SPECIAL,
 *             PATH_IS_CHARACTER_SPECIAL,  PATH_IS_UNCLASSIFIED, PATH_ERROR }
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

Path_type fp_get_path_type(FILE* fp)
{
    struct stat path_stats;


    if (fstat(fileno(fp), &path_stats) == -1)
    {
#ifdef TEST
        set_error("Fstat system called failed for %F.%S", fp);
#else
        set_error("Can't determine the size of %F.%S", fp);
#endif
        return PATH_ERROR;
    }
    else
    {
        return get_path_type_guts(&path_stats);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                get_path_type
 *
 * Determines type of path
 *
 * This routines determines the path type (file, directory, ...). It also is a
 * reasonable way to find out if a path exists at all.
 *
 * Returns:
 *    One of { PATH_DOES_NOT_EXIST,  PATH_IS_REGULAR_FILE, PATH_IS_DIRECTORY,
 *             PATH_IS_LINK, PATH_IS_PIPE, PATH_IS_SOCKET,PATH_IS_BLOCK_SPECIAL,
 *             PATH_IS_CHARACTER_SPECIAL,  PATH_IS_UNCLASSIFIED, PATH_ERROR }
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

Path_type get_path_type(const char* path)
{
    struct stat path_stats;
    char        real_path[ MAX_FILE_NAME_SIZE ];


    if (BUFF_REAL_PATH(path, real_path) == ERROR)
    {
        BUFF_CPY(real_path, path);
    }

    if (stat(real_path, &path_stats) == -1)
    {
#ifndef errno  /* GNU defines errno as a macro, leading to warning messages. */
        IMPORT int  errno;
#endif

        if (errno == ENOENT)
        {
            return PATH_DOES_NOT_EXIST;
        }
        else
        {
            set_error("Problem accessing %s.%S", path);
            return PATH_ERROR;
        }
    }
    else
    {
        return get_path_type_guts(&path_stats);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 * STATIC                         get_path_type_guts
 * -----------------------------------------------------------------------------
*/

static Path_type get_path_type_guts(struct stat *path_stats_ptr)
{

    if (S_ISREG(path_stats_ptr->st_mode))
    {
        return PATH_IS_REGULAR_FILE;
    }
    else if (S_ISDIR(path_stats_ptr->st_mode))
    {
        return PATH_IS_DIRECTORY;
    }
#ifdef S_ISLNK
    else if (S_ISLNK(path_stats_ptr->st_mode))
    {
        return PATH_IS_LINK;
    }
#endif 
#ifdef S_ISFIFO
    else if (S_ISFIFO(path_stats_ptr->st_mode))
    {
        return PATH_IS_PIPE;
    }
#endif 
#ifdef S_ISSOCK
    else if (S_ISSOCK(path_stats_ptr->st_mode))
    {
        return PATH_IS_SOCKET;
    }
#endif 
#ifdef S_ISBLK
    else if (S_ISBLK(path_stats_ptr->st_mode))
    {
        return PATH_IS_BLOCK_SPECIAL;
    }
#endif 
    else if (S_ISCHR(path_stats_ptr->st_mode))
    {
        return PATH_IS_CHARACTER_SPECIAL;
    }
    else
    {
        return PATH_IS_UNCLASSIFIED;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                get_file_size
 *
 * Determines the size of a file in bytes
 *
 * Related:
 *     fp_get_byte_size
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure, with an appropriate error message
 *     being set.
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int get_file_size(const char* file_name, off_t *size_ptr)
{
    struct stat file_stats;
    char        real_path[ MAX_FILE_NAME_SIZE ];


    if (BUFF_REAL_PATH(file_name, real_path) == ERROR)
    {
        BUFF_CPY(real_path, file_name);
    }

    if (stat(real_path, &file_stats) == -1)
    {
#ifdef TEST
        set_error("Stat system called failed for %s.%S", file_name);
#else
        set_error("Can't determine the size of %s.%S", file_name);
#endif
        return ERROR;
    }

#ifdef UNIX
    if (S_ISSOCK(file_stats.st_mode))
    {
        set_error("Attempt to determine file size of a socket.");
        return ERROR;
    }
    else if (S_ISFIFO(file_stats.st_mode))
    {
        set_error("Attempt to determine file size of a pipe.");
        return ERROR;
    }
#else
#ifdef MS_OS
    if (file_stats.st_mode && S_IFCHR)
    {
        set_error("Attempt to determine file size of a device.");
        return ERROR;
    }
#endif
#endif

    *size_ptr = file_stats.st_size;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                fp_get_byte_size
 *
 * Determines the size of a file in bytes.
 *
 * This uses that fstat(2) system call, so the FILE structure *fp is not
 * altered by this function call.  In particular, the file offset is not
 * altered (in contrast to that other popular way to find file size, using
 * fseek and ftell).  Output size is returned in location *size_ptr.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Related:
 *     get_file_size
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure, with an appropriate error message
 *     being set.
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int fp_get_byte_size(FILE* fp, off_t *size_ptr)
{
    struct stat file_stats;


    if (kjb_isatty(fileno(fp)))
    {
        set_error("Attempt to determine file size of a terminal.");
        return ERROR;
    }

    if (fstat(fileno(fp), &file_stats) == -1)
    {
#ifdef TEST
        set_error("Fstat system called failed for %F.%S", fp);
#else
        set_error("Can't determine the size of %F.%S", fp);
#endif
        return ERROR;
    }

#ifdef UNIX
    if (S_ISSOCK(file_stats.st_mode))
    {
        set_error("Attempt to determine file size of a socket.");
        return ERROR;
    }
    else if (S_ISFIFO(file_stats.st_mode))
    {
        set_error("Attempt to determine file size of a pipe.");
        return ERROR;
    }
#else
#ifdef MS_OS
    if (file_stats.st_mode && S_IFCHR)
    {
        set_error("Attempt to determine file size of a device.");
        return ERROR;
    }
#endif
#endif

    *size_ptr = file_stats.st_size;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                get_file_age
 *
 * Determines the number of seconds since a file has been modified
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure, with an appropriate error message
 *     being set.
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int get_file_age(const char* file_name, time_t *age_ptr)
{
    struct stat file_stats;
    time_t current_time;


    if (stat(file_name, &file_stats) == -1)
    {
        set_error("Can't get modification time of %s.%S", file_name);
        return ERROR;
    }

    if (time(&current_time) == TIME_ERROR)
    {
        set_error("Attempt to get current time failed.");
        return ERROR;
    }

    *age_ptr =  current_time - file_stats.st_mtime;

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                stream_younger
 *
 * Prints filename on stdout if it is young enoough
 *
 * This routine echos a file name on standard output if the file is younger than
 * the age_limit parameter in seconds.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure, with an appropriate error message
 *     being set.
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int stream_younger(time_t age_limit, const char* file_name)
{
    time_t file_age;

    ERE(get_file_age(file_name, &file_age));

    if (file_age < age_limit)
    {
        ERE(put_line(file_name));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*
 * =============================================================================
 *                                stream_older
 *
 * Prints filename on stdout if it is old enoough
 *
 * This routine echos a file name on standard output if the file is older than
 * the age_limit parameter in seconds.
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure, with an appropriate error message
 *     being set.
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int stream_older(time_t age_limit, const char* file_name)
{
    time_t file_age;

    ERE(get_file_age(file_name, &file_age));

    if (file_age > age_limit)
    {
        ERE(put_line(file_name));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                get_file_mod_time
 *
 * Returns the time when a file was modified
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure, with an appropriate error message
 *     being set.
 *
 * Index: files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int get_file_mod_time(const char* file_name, time_t *mod_time_ptr)
{
    struct stat file_stats;


    if (stat(file_name, &file_stats) == -1)
    {
        set_error("Can't get modification time of %s.%S", file_name);
        return ERROR;
    }

    *mod_time_ptr =  file_stats.st_mtime;

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

/* AMP:  for some reason, print_open_files was being ignored by c2man.
 * so I am adding an extra zig zag below, in the hope that its former absence
 * was the one and only little thing confusing c2man here.
 */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                print_open_files
 *
 * Prints the names of all files that are currently open
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Notes:
 *    This routine is only available in the development version of the library.
 *
 *    Only files opened with kjb_fopen will be listed.
 *
 * Index: files, debugging
 *
 * -----------------------------------------------------------------------------
*/

int print_open_files(FILE* fp)
{
    int          i;


    ERE(initialize_cache());

    ERE(kjb_fprintf(fp, "\nOpen files for process %ld:\n", (long)MY_PID));

    for (i=0; i<MAX_NUM_NAMED_FILES; i++)
    {
        if (fs_cached_file_reference_count[ i ] > 0)
        {
            ERE(kjb_fprintf(fp, "    %-3d %s (%d ref).\n", i,
                            fs_cached_user_fnames[ i ],
                            fs_cached_file_reference_count[ i ]));
        }
    }

    ERE(kjb_fputs(fp, "\n"));

    return kjb_fflush(fp);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

/*
 * =============================================================================
 * STATIC                         free_cached_file_names
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * -----------------------------------------------------------------------------
*/

static void free_cached_file_names(void)
{
    int          i;
#ifdef TEST
    int          num_open_files = 0;
#endif


    /*
    // Mimic print_open_files here, since it is problematic calling it at the
    // end of the program, while things are being dismantled--especially since
    // paging is not a bad feature of print_open_files. Also, we want a slightly
    // different behaviour. We only want to hear about open files if there are
    // more than the standard ones.
    */
#ifdef TEST
    for (i=0; i<MAX_NUM_NAMED_FILES; i++)
    {
        if (fs_cached_user_fnames[ i ] != NULL) num_open_files++;
    }

    if (num_open_files > 3)
    {
        printf("\nOpen files other than 0,1,2 for process %ld:\n", (long)MY_PID);
    }
#endif

    for (i=0; i<MAX_NUM_NAMED_FILES; i++)
    {
        kjb_free(fs_cached_fnames[ i ]);
        fs_cached_fnames[ i ] = NULL;

        if (fs_cached_user_fnames[ i ] != NULL)
        {
#ifdef TEST
            if (i >= 3)
            {
                printf("    %-3d %s (%d ref).\n", i, fs_cached_user_fnames[ i ],
                       fs_cached_file_reference_count[ i ]);
            }
#endif
            kjb_free(fs_cached_user_fnames[ i ]);
            fs_cached_user_fnames[ i ] = NULL;
        }

        kjb_free(fs_cached_uncompressed_fnames[ i ]);
        fs_cached_uncompressed_fnames[ i ] = NULL;

        fs_cached_file_reference_count[ i ] = NOT_SET;   /* Invalidation */
    }

#ifdef TEST
    if (num_open_files > 5)
    {
        puts("\n");
        fflush(stdout);
    }
#endif

}


#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                         process_file_open_mode
 * -----------------------------------------------------------------------------
*/

static int process_file_open_mode
(
    const char* mode,
    char*       system_dependent_mode,
    size_t      system_dependent_mode_max_len,
    char*       unix_mode,
    size_t      unix_mode_max_len
)
{


#ifdef MAX_SIZE_T_BUFF_SIZE
    if (system_dependent_mode_max_len > MAX_SIZE_T_BUFF_SIZE)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
#endif

    if (system_dependent_mode_max_len == 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    --system_dependent_mode_max_len;  /* Account for room needed by null. */

    while (    (system_dependent_mode_max_len > 0)
            && (unix_mode_max_len > 0)
            && (*mode != '\0')
          )
    {
#ifdef MS_OS
        /* Straight copy.  */
        *system_dependent_mode = *mode;
        system_dependent_mode++;
        system_dependent_mode_max_len--;

#else   /* Case not MS_OS follows */

        /* Copy, but skip 'b's.   */

        if (*mode != 'b')
        {
            *system_dependent_mode = *mode;
            system_dependent_mode++;
            system_dependent_mode_max_len--;
        }
#endif

        /* Copy, but skip 'b's.   */

        if (*mode != 'b')
        {
            *unix_mode = *mode;
            unix_mode++;
            unix_mode_max_len--;
        }

        mode++;
    }

    *system_dependent_mode = '\0';
    *unix_mode = '\0';

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/*
 * =============================================================================
 *                                kjb_isatty
 *
 * Returns whether a file descriptor is a terminal 
 *
 * kjb_isatty is very similar to the isatty, except that error reporting
 * is made consistent with the other library routines, and the error messages
 * are more meaningfull.
 *
 * Bugs:
 *     This function has not been vetted for being re-entrant / thread safe. 
 *
 *     See also isatty(2) for bugs as this routine basically wraps that one. 
 *
 * Returns:
 *    On failure an error message is set, and ERROR is returned. Otherwise
 *    the file position is retuned.
 *
 * Index: I/O
 *
 * -----------------------------------------------------------------------------
*/

int kjb_isatty(int file_des)
{
    int result;


    if (file_des < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(initialize_cache());

    if (file_des < MAX_NUM_NAMED_FILES)
    {
        if (KJB_IS_SET(fs_cached_isatty[ file_des ]))
        {
            return fs_cached_isatty[ file_des ];
        }
    }

    result = kjb_isatty_guts(file_des);

    if (file_des < MAX_NUM_NAMED_FILES)
    {
        fs_cached_isatty[ file_des ] = result;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 * STATIC                         kjb_isatty_guts
 *
 * This function is not re-entrant (i.e., not thread-safe) for SUN4.
 *
 * -----------------------------------------------------------------------------
*/

/*
 * Very old code. It can be swtiched to SUN5 for testing, but SUN5 has the
 * standard SYS5 version of isatty(), which is what we use most of the time.
*/

#ifdef SUN4

#ifdef KJB_CPLUSPLUS
    /* I have never tried C++ on SUN4, but this has been tested on SUN5. */
    extern "C" {
#endif
    extern char* ctermid_r(char *);
    extern char* ttyname_r(int, char *, int);
#ifdef KJB_CPLUSPLUS
               }
#endif

static int kjb_isatty_guts(int file_des)
{
    static char  control_ttyname_buff[ L_ctermid ] = { '\0' };
    static int   first_time                        = TRUE;
    char         ttyname_buff[ MAXPATHLEN ];
    char*        ttyname_ptr;
    char*        tty_name_pos;
    char         path_piece_one[ MAXPATHLEN ];
    char         path_piece_two[ MAXPATHLEN ];
#ifdef SUN5 /* Only used when testing by changing the SUN4 def to SUN */
    char         path_piece_three[ MAXPATHLEN ];
#endif


    if (first_time)
    {
#ifdef SUN5 /* Only used when testing by changing the SUN4 def to SUN */
        if (ctermid_r(control_ttyname_buff) == NULL)
#else
            if (ctermid(control_ttyname_buff) == NULL)
#endif
            {
                control_ttyname_buff[ 0 ] = '\0';
            }
    }

#ifdef SUN5 /* Only used when testing by changing the SUN4 def to SUN */
    ttyname_ptr = ttyname_r(file_des, ttyname_buff, sizeof(ttyname_buff));
#else
    ttyname_ptr = ttyname(file_des);
#endif

    if (ttyname_ptr == NULL)
    {
        return FALSE;
    }

#ifdef SUN5 /* Only used when testing by changing the SUN4 def to SUN */
    BUFF_CPY(ttyname_buff, ttyname_ptr);
#endif

    if (STRCMP_EQ(control_ttyname_buff, ttyname_buff))
    {
        return TRUE;
    }

    /*
    // Find out if it is a pseudo-terminal. If so, then call it a terminal
    // (most terminals -- IE networked ones -- are pseudo-terminals).
    */

    tty_name_pos = ttyname_buff;

    if (*tty_name_pos != '/')
    {
        return FALSE;
    }

    tty_name_pos++;

    BUFF_GEN_GET_TOKEN(&tty_name_pos, path_piece_one, "/");
    BUFF_GEN_GET_TOKEN(&tty_name_pos, path_piece_two, "/");

#ifdef SUN5 /* Only used when testing by changing the SUN4 def to SUN */

    BUFF_GEN_GET_TOKEN(&tty_name_pos, path_piece_three, "/");

    if (    (    (STRCMP_EQ(path_piece_one, "dev"))
              && (STRCMP_EQ(path_piece_two, "pts"))
              && (path_piece_three[ 0 ] != '\0')
              && (all_digits(path_piece_three))
            )
         ||
            (    (STRCMP_EQ(path_piece_one, "dev"))
              && (STRCMP_EQ(path_piece_two, "console"))
            )
         ||
            (    (STRCMP_EQ(path_piece_one, "devices"))
              && (STRCMP_EQ(path_piece_one, "pseudo"))
              && (HEAD_CMP_EQ(path_piece_three, "pts"))
            )
       )
    {
        return TRUE;
    }
#else
#if 0 /* was ifdef SUN4
         -- disabled because we are already inside an ifdef-SUN4 section */
    if (    (STRCMP_EQ(path_piece_one, "dev"))
         &&
            (    (HEAD_CMP_EQ(path_piece_two, "ttyp"))
              || (STRCMP_EQ(path_piece_two, "console"))
            )
       )
    {
        return TRUE;
    }
#endif
#endif

    return FALSE;
}

#else  /* All others except for SUN4 -- to be updated as needed.  */

/*
// Go with the system supplied version until alternates are desired.
//
// NeXt controlling terminal can be found out by using ttyslot() and the file
// /etc/ttys. Pseudo-tty's is done much like SUN4 (as it is a BSD system).
//
// At one point MS_16_BIT_OS with Microsoft compiler version 5 had isatty(
// <printer> ) returning TRUE. I don't know if this will be the case with a late
// model Borland Compiler.
*/

static int kjb_isatty_guts(int file_des)
{


    return isatty(file_des);

}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              print_underlined
 *
 * Prints a string as underlined
 *
 * The method to print underlined text is the same as that used by nroff to
 * format text which is underlined both on the printed page, and on the
 * screen. (Some tools, on some systems, convert this to bold).
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *    The number of characters output, which is three for every character in
 *    the input string sucessfully dealt with. This is a bit confusing, since
 *    the return is not general the length of buff, but it is the more "proper"
 *    return. The output should be properly paged if the output is the
 *    terminal. (I have not tested this, however, as the usual use of this
 *    routine is to build man pages).
 *
 * ==> FIX  (test the routine more thouroughly, including the point raised
 * ==>       above).
 *
 * -----------------------------------------------------------------------------
*/

long print_underlined(FILE* fp, const char* buff)
{
    IMPORT volatile Bool halt_all_output;
    IMPORT volatile Bool halt_term_output;
    int                 result           = NO_ERROR;
    int                 count            = 0;
    int                 cache_is_ttty    = kjb_isatty(fileno(fp));


    kjb_clear_error();

    /*
    // For each character in buff, we output "_", "" (ctl-H), and the
    // charcter.  The main complicating factors are paging and error handling.
    // We use the standard library fputc for the cases where we do not want the
    // characters counted for paging, specifically "_", and "".
    */
    while (    (*buff) && ( !halt_all_output )
            && !(halt_term_output && cache_is_ttty)
          )
    {
        if (fputc('_', fp) == -1)
        {
            result = ERROR;
            break;
        }

        count++;

        if (fputc('', fp) == -1)
        {
            result = ERROR;
            break;
        }

        count++;

        if (cache_is_ttty)
        {
            if (term_put_n_chars(buff, 1) != 1)
            {
                result = ERROR;
            }
        }
        else
        {
            if (fputc(*buff, fp) == -1)
            {
                result = ERROR;
                break;
            }
        }

        buff++;
        count++;
    }

    if (result == ERROR)
    {
        insert_error("Underlined output to %F failed.%S.", fp);
        return result;
    }
    else
    {
        return count;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                start_stdout_shadow
 *
 * Begins stdout shadowing to a file
 *
 * This routine begins stdout shadowing to the file with name file_name. If
 * stdout shadowing is already in place, the destination is switched over to
 * file_name. More bluntly, there can be only one stdout shadow file.
 *
 * When shadowing is in place, lines that are written to standard output are
 * also appended to the shadow file.
 *
 * This routine will fail if the shadow file cannot be opend for appending.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure, with an appropriate error message
 *     being set.
 *
 * Index: I/O, files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int start_stdout_shadow(const char* file_name)
{

    stop_stdout_shadow();

    fs_stdout_shadow_fp = kjb_fopen(file_name, "a");

    if (fs_stdout_shadow_fp == NULL)
    {
        add_error("Error occured trying to open a shadow log for standard output.");
        return ERROR;
    }

#if 0 /* was ifdef HOW_IT_WAS_05_01_29 */
    ERE(add_cleanup_function(stop_stdout_shadow));
#endif

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                stop_stdout_shadow
 *
 * Stops shadowing of stdout
 *
 * This routine stops shadowing of stdout, closing the shadow file. If shadowing
 * is not currently in place, this routine has no effect.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Index: I/O, files, standard library
 *
 * -----------------------------------------------------------------------------
*/

void stop_stdout_shadow(void)
{

    (void)kjb_fclose(fs_stdout_shadow_fp);
    fs_stdout_shadow_fp = NULL;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                start_stderr_shadow
 *
 * Begins stderr shadowing to a file
 *
 * This routine begins stderr shadowing to the file with name file_name. If
 * stderr shadowing is already in place, the destination is switched over to
 * file_name. More bluntly, there can be only one stderr shadow file.
 *
 * When shadowing is in place, lines that are written to standard error are
 * also appended to the shadow file.
 *
 * This routine will fail if the shadow file cannot be opend for appending.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Returns:
 *     NO_ERROR on success, ERROR on failure, with an appropriate error message
 *     being set.
 *
 * Index: I/O, files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int start_stderr_shadow(const char* file_name)
{

    stop_stderr_shadow();

    fs_stderr_shadow_fp = kjb_fopen(file_name, "a");

    if (fs_stderr_shadow_fp == NULL)
    {
        add_error("Error occured trying to open a shadow log for standard error.");
        return ERROR;
    }

#if 0 /* was ifdef HOW_IT_WAS_05_01_29 */
    ERE(add_cleanup_function(stop_stderr_shadow));
#endif

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                stop_stderr_shadow
 *
 * Stops shadowing of stderr
 *
 * This routine stops shadowing of stderr, closing the shadow file. If shadowing
 * is not currently in place, this routine has no effect.
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Index: I/O, files, standard library
 *
 * -----------------------------------------------------------------------------
*/

void stop_stderr_shadow(void)
{

    (void)kjb_fclose(fs_stderr_shadow_fp);
    fs_stderr_shadow_fp = NULL;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                enable_stdout
 *
 * Enable writing to standard output.
 *
 * This cancels the effect of disable_stdout().
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Index: I/O, standard library
 *
 * -----------------------------------------------------------------------------
*/
void enable_stdout(void)
{
    fs_enable_stdout = TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*
 * =============================================================================
 *                                disable_stdout
 *
 * Disable writing to standard output.
 *
 * Immediately after calling this function, all writes to standard output are
 * suppressed, provided one uses KJB library output routines.
 * You can cancel the effect with enable_stdout().
 *
 * This function is not re-entrant (i.e., not thread-safe).
 *
 * Index: I/O, standard library
 *
 * -----------------------------------------------------------------------------
*/
void disable_stdout(void)
{
    fs_enable_stdout = FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_glob
 *
 * Wrapper for glob() with optional filtering
 *
 * This routine is a wrapper for glob() with optional filtering. It thus deals
 * with system dependent aspects to glob(), and puts the result into the KJB
 * library type "Word_list". If filter_fn() is NOT null, the reults are further
 * pruned based on whether it evaluates to TRUE. A typical value for filter_fn()
 * is is_file(). 
 *
 * The matches are placed in the list of strings data structure (*dir_files_pp)
 * which is created and/or resized as needed.  The number of matches is
 * available in (*dir_files_pp)->num_words. If there are no matches, or if
 * patern is NULL, then a Word_list of zero words is created. This is considered
 * a successful return (NO_ERROR). 
 *
 * Returns: 
 *    On success kjb_glob returns NO_ERROR.  On failure ERROR is returned and an
 *    error message is set.
 *
 * Index: I/O, files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int kjb_glob(Word_list** dir_files_pp, const char* pattern,
             int filter_fn(const char* path))
{
#ifdef UNIX
    glob_t pglob;
    int result = NO_ERROR;
    int flags = GLOB_ERR;
    int glob_res;

    if (pattern == NULL)
    {
        return get_target_word_list(dir_files_pp, 0);
    }

    glob_res = glob(pattern, flags, NULL, &pglob);

    if (glob_res == GLOB_NOMATCH)
    {
        ERE(get_target_word_list(dir_files_pp, 0));
    }
    else if (glob_res != 0)
    {
        set_error("Unable to execute matching on %s.%S",
                  pattern);
        result = ERROR;
    }
    else
    {
        int    word_count = 0;
        char** words;
        unsigned int   i;
        int    filter_res = NO_ERROR;

        result = get_target_word_list(dir_files_pp,
                                      (int)pglob.gl_pathc);

        if (result != ERROR)
        {
            words = (*dir_files_pp)->words;

            for (i = 0; i < pglob.gl_pathc; i++)
            {
                if (filter_fn == NULL)
                {
                    filter_res = TRUE;
                }
                else
                {
                    filter_res = filter_fn(pglob.gl_pathv[ i ]);
                    UNTESTED_CODE();
                }

                if (filter_res == ERROR)
                {
                    NOTE_ERROR();
                    result = ERROR;
                    break;
                }
                else if (filter_res)
                {
                    if ((words[ word_count ] = kjb_strdup(pglob.gl_pathv[ i ])) == NULL)
                    {
                        NOTE_ERROR();
                        result = ERROR;
                        break;
                    }

                    word_count++;
                }
            }

            (*dir_files_pp)->num_words = word_count;
        }
    }

    globfree(&pglob);

    return result;
#else /* Case NOT UNIX follows. */
    UNTESTED_CODE(); 
    set_error("Unable to 'glob' some paths.");
    add_error("Calling a system function currently only implementated in unix.");
    return ERROR;
#endif 
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                kjb_simple_glob
 *
 * Interface to kjb_glob() for a comon cases
 *
 * This routine is an alternative interface to glob(), where the pattern is
 * being built up of [beg_pattern]*[end_pattern], and the values of what is
 * matched to the "*" is required. Either beg_pattern and/or end_pattern can be
 * NULL.  If star_str_pp is not NULL, then the values matched to the the "*" are
 * placed in (*star_str_pp).  (This routine is normally called with star_str_pp
 * not NULL, otherwise kjb_glob would tyically be used).  Otherwise, this
 * routine is similar to kjb_glob(). 
 *
 * Returns: 
 *    On success kjb_simple_glob returns NO_ERROR.  On failure ERROR is returned
 *    and an error message is set.
 *
 * Index: I/O, files, standard library
 *
 * -----------------------------------------------------------------------------
*/

int kjb_simple_glob(Word_list** dir_files_pp, Word_list** star_str_pp,
                    const char* beg_pattern, const char* end_pattern,
                    int filter_fn(const char* path))
{
    char glob_pattern[ 10000 ];
    int num_words, i;
    char** words;
    int beg_pattern_len = (beg_pattern == NULL) ? 0  : strlen(beg_pattern);
    int end_pattern_len = (end_pattern == NULL) ? 0  : strlen(end_pattern);
    int word_len;
    char* word_ptr;


    glob_pattern[ 0 ] = '\0';

    if (beg_pattern != NULL)
    {
        BUFF_CAT(glob_pattern, beg_pattern);
    }

    BUFF_CAT(glob_pattern, "*");

    if (end_pattern != NULL)
    {
        BUFF_CAT(glob_pattern, end_pattern);
    }

    ERE(kjb_glob(dir_files_pp, glob_pattern, filter_fn));

    if (star_str_pp == NULL) return NO_ERROR;

    num_words = (*dir_files_pp)->num_words;
    words = (*dir_files_pp)->words;

    ERE(get_target_word_list(star_str_pp, num_words));

    for (i = 0; i < num_words; i++)
    {
        word_ptr = words[ i ];

        word_ptr += beg_pattern_len;

        NRE((*star_str_pp)->words[ i ] = kjb_strdup(word_ptr));

        word_len = strlen((*star_str_pp)->words[ i ]);
        (*star_str_pp)->words[ i ][ word_len - end_pattern_len ] = '\0';
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

