/*
 * $Id: l_mt_util.c 21545 2017-07-23 21:57:31Z kobus $
 */

#include "l/l_sys_debug.h"
#include <l/l_sys_mal.h>
#include <l_mt/l_mt_pthread.h>
#include <l_mt/l_mt_util.h>


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_mt_malloc
 *
 * Threadsafe memory allocation
 *
 * This allocates memory using libkjb routines in a threadsafe way, because
 * it uses the multithread wrapper serialization lock.  Only one thread at a
 * time can malloc memory via this function.  In all other respects this
 * operates like kjb_malloc.
 *
 * Returns:
 *      NULL if allocation is unsuccessful, along with an error message.
 *      Otherwise, this returns a pointer to the memory block of the requested
 *      size.
 *
 * Related: kjb_malloc, kjb_free, kjb_mt_free
 *
 * Index: threads, memory allocation
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
void *kjb_mt_malloc(Malloc_size num_bytes)
{
    void *p;
    kjb_multithread_wrapper_serialization_lock();
    p = kjb_malloc(num_bytes);
    kjb_multithread_wrapper_serialization_unlock();
    return p;
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_mt_free
 *
 * Free heap memory, thread-safely
 *
 * This frees a block of heap memory allocated using kjb_mt_malloc,
 * kjb_malloc, or any other libkjb memory allocation function.  This works
 * like kjb_free except that the operation is threadsafe, because it
 * is serialized with the multithread wrapper serialization lock.
 * Only one thread at a time therefore can free memory via this function.
 *
 * Related: kjb_malloc, kjb_free, kjb_mt_malloc
 *
 * Index: threads, memory allocation
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
void kjb_mt_free(void *ptr)
{
    kjb_multithread_wrapper_serialization_lock();
    kjb_free(ptr);
    kjb_multithread_wrapper_serialization_unlock();
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_mt_fopen
 *
 * Open a file stream, thread-safely
 *
 * This opens a file stream using libkjb routines in a threadsafe way, because
 * it uses the multithread wrapper serialization lock.  Only one thread at a
 * time can open a file via this function.  In all other respects this
 * operates like kjb_fopen.
 *
 * Returns:
 *     Pointer to FILE if successful, otherwise NULL and an error message is
 *     set.
 *
 * Related:  kjb_fopen, kjb_mt_fclose, kjb_mt_fwrite, kjb_mt_fprintf
 *
 * Index: threads, I/O
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
FILE *kjb_mt_fopen(
    const char *input_fd_name,
    const char *mode
)
{
    FILE* fp;
    kjb_multithread_wrapper_serialization_lock();
    fp = kjb_fopen(input_fd_name, mode);
    kjb_multithread_wrapper_serialization_unlock();
    return fp;
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_mt_fread
 *
 * Read from a file stream, thread-safely
 *
 * This reads from file stream using libkjb routines in a threadsafe way,
 * because
 * it uses the multithread wrapper serialization lock.  Only one thread at a
 * time can read from a file via this function.  In all other respects this
 * operates like kjb_fread.
 *
 * Returns:
 *     Number of bytes read, or ERROR and an error message is set.
 *
 * Related:  kjb_mt_fopen, kjb_mt_fclose, kjb_mt_fwrite, kjb_mt_fprintf,
 *           kjb_fread
 *
 * Index: threads, I/O, input
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
long kjb_mt_fread(
    FILE *fp,
    void *buff,
    size_t len
)
{
    long ct;
    kjb_multithread_wrapper_serialization_lock();
    ct = kjb_fread(fp, buff, len);
    kjb_multithread_wrapper_serialization_unlock();
    return ct;
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_mt_fwrite
 *
 * Write to a file stream, thread-safely
 *
 * This writes to file stream using libkjb routines in a threadsafe way,
 * because
 * it uses the multithread wrapper serialization lock.  Only one thread at a
 * time can write to a file via this function.  In all other respects this
 * operates like kjb_fwrite.
 *
 * Returns:
 *     Number of bytes written, or ERROR and an error message is set.
 *
 * Related:  kjb_mt_fopen, kjb_mt_fclose, kjb_fwrite, kjb_mt_fprintf,
 *           kjb_mt_fread
 *
 * Index: threads, I/O, output
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
long kjb_mt_fwrite(
    FILE *fp,
    const void *line,
    size_t len
)
{
    long ct;
    kjb_multithread_wrapper_serialization_lock();
    ct = kjb_fwrite(fp, line, len);
    kjb_multithread_wrapper_serialization_unlock();
    return ct;
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_mt_fprintf
 *
 * Print to a file stream, thread-safely
 *
 * This prints formatted data to file stream using libkjb routines in a
 * threadsafe way, because it uses the multithread wrapper serialization lock.
 * Only one thread at a time can write to a file via this function.
 * In all other respects this operates like kjb_fprintf.
 *
 * Returns:
 *     Number of chars written, or ERROR and an error message is set.
 *
 * Related:  kjb_mt_fopen, kjb_mt_fclose, kjb_mt_fwrite, kjb_fprintf,
 *           kjb_mt_fread
 *
 * Index: threads, I/O, output
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
long kjb_mt_fprintf(FILE* fp, const char* format_str, ...)
{
    int result;
    va_list ap;

    kjb_multithread_wrapper_serialization_lock();
    va_start(ap, format_str);
    result = kjb_vfprintf(fp, format_str, ap);
    va_end(ap);
    kjb_multithread_wrapper_serialization_unlock();

    return result;
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                             kjb_mt_fclose
 *
 * Closes a file stream
 *
 * This closes a file stream using libkjb routines in a threadsafe way, because
 * it uses the multithread wrapper serialization lock.  Only one thread at a
 * time can close a file via this function.  In all other respects this
 * operates like kjb_fclose.
 *
 * Related:  kjb_mt_fopen, kjb_fclose, kjb_mt_fwrite, kjb_mt_fprintf,
 *           kjb_mt_fread
 *
 * Returns:
 *     NO_ERROR if successful, otherwise ERROR and an error message is set.
 *
 * Index: threads, I/O
 *
 * Author:  Andrew Predoehl
 *
 * Documenter:  Andrew Predoehl
 *
 * ----------------------------------------------------------------------------
*/
int kjb_mt_fclose(FILE *fp)
{
    int result;
    kjb_multithread_wrapper_serialization_lock();
    result = kjb_fclose(fp);
    kjb_multithread_wrapper_serialization_unlock();
    return result;
}

