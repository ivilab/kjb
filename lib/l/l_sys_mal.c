
/* $Id: l_sys_mal.c 21655 2017-08-05 14:54:36Z kobus $ */

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
/*
 * $Id: l_sys_mal.c 21655 2017-08-05 14:54:36Z kobus $
 */

#include "l/l_gen.h"     /* Only safe as first include in a ".c" file. */
#include "l/l_parse.h"
#include "l/l_sys_mal.h"
#include "l/l_string.h"

#ifdef SUN5
#    ifndef __GNUC__
#        define MALLINFO
#    endif
#endif

#ifdef SUN4
#    define MALLINFO
#endif

/*
// Works if we add "-lmalloc", but then we get preemption messages.
// Not very important either way.
//
#ifdef SGI
#    define MALLINFO
#endif
//
*/

#ifdef AIX
#    define MALLINFO
#endif

#ifdef MALLINFO
/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#    include <malloc.h>
#endif
#endif

#ifdef NO_MALLINFO
#    undef MALLINFO
#endif

#ifdef MS_16_BIT_OS
/*Some versions of makedepend cannot be trusted to honor the "-Y" option. */
#ifndef MAKE_DEPEND
#include <alloc.h>
#endif
#endif

#ifdef __C2MAN__
#ifndef TRACK_MEMORY_ALLOCATION_OR_C2MAN
#define TRACK_MEMORY_ALLOCATION_OR_C2MAN
#endif
#endif

#ifdef TRACK_MEMORY_ALLOCATION
#ifndef TRACK_MEMORY_ALLOCATION_OR_C2MAN
#define TRACK_MEMORY_ALLOCATION_OR_C2MAN
#endif
#endif

#ifdef TRACK_MEMORY_ALLOCATION_OR_C2MAN 

#   include "l/l_io.h"
#   include "l/l_sort.h"

#ifdef TEST
#   include "l/l_sys_scan.h"
#   include "l/l_sys_rand.h"
#endif 

#endif 


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION_OR_C2MAN 

#ifdef TEST
    static double fs_simulated_heap_failure_frequency = DBL_NOT_SET;
#endif

#   define MAX_NUM_POINTERS             5000000
#   define MAX_NUM_POINTERS_2            500000
#   define MAX_ALLOCATION_FILE_NAME_LEN      30
#   define MALLOC_OVER_RUN_SIZE              48

    typedef struct Sorted_pointers
    {
        long ptr;
        int  index;
    }
    Sorted_pointers;

    static int fs_heap_checking_enable = TRUE;
    static int fs_initialization_checking_enable = TRUE;
    static int fs_num_pointers = 0;
    static void*       fs_pointers_to_allocated[ MAX_NUM_POINTERS ];
    static Malloc_size fs_amount_allocated[ MAX_NUM_POINTERS ];
    static char        fs_allocation_file_name[ MAX_NUM_POINTERS ][ MAX_ALLOCATION_FILE_NAME_LEN ];
    static int         fs_allocation_line_number[ MAX_NUM_POINTERS ];
    static int         fs_allocation_pid[ MAX_NUM_POINTERS ];
    static void*       fs_pointer_to_watch     = NULL;
    static int         fs_fast_lookup_is_valid = FALSE;
    static Sorted_pointers fs_sorted_pointers[ MAX_NUM_POINTERS ];
    static char  fs_start_heap_check_2_file_name[ MAX_FILE_NAME_SIZE ];
    static int   fs_start_heap_check_2_line_number = NOT_SET;
    static int fs_heap_checking_enable_2 = FALSE;
    static int fs_heap_checking_skip_2 = FALSE;
    static int fs_num_pointers_2 = 0;
    static void*       fs_pointers_to_allocated_2[ MAX_NUM_POINTERS_2 ];
    static Malloc_size fs_amount_allocated_2[ MAX_NUM_POINTERS ];
    static char        fs_allocation_file_name_2[ MAX_NUM_POINTERS_2 ][ MAX_ALLOCATION_FILE_NAME_LEN ];
    static int         fs_allocation_line_number_2[ MAX_NUM_POINTERS_2 ];
    static int         fs_allocation_pid_2[ MAX_NUM_POINTERS_2 ];
    static int         fs_fast_lookup_is_valid_2 = FALSE;
    static Sorted_pointers fs_sorted_pointers_2[ MAX_NUM_POINTERS ];
#else
    static int             fs_heap_checking_enable = FALSE;
    static int             fs_initialization_checking_enable = FALSE;

#endif    /* --- #ifdef TRACK_MEMORY_ALLOCATION ----------------------------- */


#ifdef MALLINFO
    static int fs_mallinfo_call_ok = FALSE;
#endif

/*
 * Not used at the moment, but we may want to ask for aborts even if we are not
 * interactive.
*/
static int fs_heap_check_failure_forces_abort = FALSE;

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION
static void save_pointer_info
(
    void*       ptr,
    Malloc_size num_bytes,
    const char* file_name,
    int         line_number,
    int         pid
);

static void save_pointer_info_2
(
    void*       ptr,
    Malloc_size num_bytes,
    const char* file_name,
    int         line_number,
    int         pid
);

static void set_pointer_info      
(
    void*       ptr,
    Malloc_size num_bytes,
    const char* file_path,
    int         line_number,
    int         pid,
    int         ptr_index
);

static void set_pointer_info_2    
(
    void*       ptr,
    Malloc_size num_bytes,
    const char* file_path,
    int         line_number,
    int         pid,
    int         ptr_index
);

static int  lookup_pointer_index  (const void* ptr);
static int  lookup_pointer_index_2(const void* ptr);

#endif

/* ------------------------------------------------------------------------- */

int set_heap_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if ((lc_option[ 0 ] == '\0') || match_pattern(lc_option, "heap-checking"))
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("heap-checking = %s\n",
                    fs_heap_checking_enable ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Heap checking is %s.\n",
                    fs_heap_checking_enable ? "enabled" : "disabled" ));
        }
        else
        {
            int temp_boolean_value;

            ERE(temp_boolean_value = get_boolean_value(value));

            if (temp_boolean_value)
            {
                set_error("Heap checking can only be disabled.");
                return ERROR;
            }

            fs_heap_checking_enable = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if ((lc_option[ 0 ] == '\0') || match_pattern(lc_option, "heap-check-failure-forces-abort"))
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("heap-check-failure-forces-abort = %s\n",
                    fs_heap_check_failure_forces_abort ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {

            ERE(pso("Heap check failure %s abort.\n",
                    fs_heap_check_failure_forces_abort ? "forces" : "does not force" ));
        }
        else
        {
            int temp_boolean_value;

            ERE(temp_boolean_value = get_boolean_value(value));

            fs_heap_check_failure_forces_abort = temp_boolean_value;

        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "initialization-checking")
          || match_pattern(lc_option, "init-checking")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("initialization-checking = %s\n",
                    fs_initialization_checking_enable ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Initialization checking is %s.\n",
                    fs_initialization_checking_enable ? "enabled" : "disabled" ));
        }
        else
        {
            int temp_boolean_value;

            ERE(temp_boolean_value = get_boolean_value(value));

            if (temp_boolean_value)
            {
                set_error("Initialization checking can only be disabled.");
                return ERROR;
            }

            fs_initialization_checking_enable = temp_boolean_value;

        }
        result = NO_ERROR;
    }

#ifdef TRACK_MEMORY_ALLOCATION
#ifdef TEST
    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "heap-failure-simulation-frequency")
         || match_pattern(lc_option, "heap-failure-frequency")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_simulated_heap_failure_frequency >= 0.0)
            {
                ERE(pso("heap-failure-simulation-frequency = %.5f\n",
                        fs_simulated_heap_failure_frequency));
            }
            else
            {
                ERE(pso("heap-failure-simulation-frequency = off\n"));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_simulated_heap_failure_frequency >= 0.0)
            {
                ERE(pso("Storage allocation failure is simulated with "));
                ERE(pso("a frequency of %.5f\n",
                        fs_simulated_heap_failure_frequency));
            }
            else
            {
                ERE(pso("Storage allocation failure is not simulated.\n"));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_simulated_heap_failure_frequency = DBL_NOT_SET;
        }
        else
        {
            double temp_double;

            ERE(ss1d(value, &temp_double));
            fs_simulated_heap_failure_frequency = temp_double;

        }
        result = NO_ERROR;
    }
#endif
#endif

    if ((lc_option[ 0 ] == '\0') || match_pattern(lc_option, "use-memcpy"))
    {
        IMPORT int kjb_use_memcpy;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("use-memcpy = %s\n",
                    kjb_use_memcpy ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Memcpy %s used when possible for performance.\n",
                    kjb_use_memcpy ? "is" : "is not" ));
        }
        else
        {
            int temp_boolean_value;

            ERE(temp_boolean_value = get_boolean_value(value));

            kjb_use_memcpy = temp_boolean_value;

        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

void disable_heap_checking_for_thread_use(void)
{
    if (fs_heap_checking_enable)
    {
        warn_pso("Disabling heap_checking in anticipation of thread use.\n"); 
        warn_pso("This is because heap checking is not yet thread safe.\n"); 

        fs_heap_checking_enable = FALSE;
    }
}

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

void disable_heap_checking(void)
{
    fs_heap_checking_enable = FALSE;
}

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef OBSELETE
#ifdef TRACK_MEMORY_ALLOCATION

/* =============================================================================
 *                             reset_heap_checking_for_fork
 *
 * Resets heap checking for the child process
 *
 * This routine should only be called in very special circumntances. Currently,
 * it is only used by kjb_fork(). Since the fork copies all information, pending
 * checked memory from the parent is inherited. So we reset the counter with
 * this function. .
 *
 * Returns:
 *    No return (void). 
 *
 * Index: memory allocation, debugging
 *
 * -----------------------------------------------------------------------------
*/

/* 
 * Kobus July 2016. This version was not quite right either. We have now gone to
 * recording the PID of the allocating process, so that we can report what we
 * want. This function is not OBSELETE. 
 *
 * Kobus, May 2014. Apparantly, this was either unfinished or wrong. So this
 * version of the function is relatively new, as is its call from kjb_fork(). 
*/
void reset_heap_checking_for_fork(void)
{

    fs_num_pointers = 0;
    fs_num_pointers_2 = 0;

    fs_pointer_to_watch     = NULL;
    fs_fast_lookup_is_valid = FALSE;
    fs_start_heap_check_2_line_number = NOT_SET;
    fs_heap_checking_enable_2 = FALSE;
    fs_heap_checking_skip_2 = FALSE;
    fs_fast_lookup_is_valid_2 = FALSE;
}

#endif 
#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

void skip_heap_check_2(void)
{
    if (fs_heap_checking_skip_2)
    {
        /*
         * We need to push and pop these !!!
        */
        set_bug("Nested SKIP/CONTIUE heap check 2");
    }
    fs_heap_checking_skip_2 = TRUE;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

void continue_heap_check_2(void)
{
    fs_heap_checking_skip_2 = FALSE;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             memory_used
 *
 * Returns the amount of heap storage used
 *
 * This routine returns the amount of heap storage used provided that this
 * facility is available on the current platform. If this facility is not
 * available, then ERROR is returned, with an error message being set.
 *
 * Note that the amount of storage used is always greater than the number of
 * bytes allocated, sometimes by a substantial amount. This is because memory is
 * not returned to the system when it is freed; freed memory is simply available
 * for re-use by the allocating process.
 *
 * Returns:
 *    The amount of heap storage used in bytes, if available, otherwise ERROR
 *    is returned, with an error message being set.
 *
 * Index: memory allocation, debugging
 *
 * -----------------------------------------------------------------------------
*/

#ifdef MALLINFO

/* MALLINFO */ int memory_used(unsigned long* total_bytes_used_ptr)
/* MALLINFO */ {
/* MALLINFO */     struct mallinfo malloc_info;
/* MALLINFO */
/* MALLINFO */
/* MALLINFO */     if (fs_mallinfo_call_ok == FALSE)
/* MALLINFO */     {
/* MALLINFO */         set_error("System information on heap storage is not available.");
/* MALLINFO */         return ERROR;
/* MALLINFO */     }
/* MALLINFO */     else
/* MALLINFO */     {
/* MALLINFO */         malloc_info = mallinfo();
/* MALLINFO */         *total_bytes_used_ptr = malloc_info.arena;
/* MALLINFO */         return NO_ERROR;
/* MALLINFO */     }
/* MALLINFO */ }

#else

/* default  */ int memory_used(unsigned long* total_bytes_used_ptr)
/* default  */ {
/* default  */
/* default  */     *total_bytes_used_ptr = 0;
/* default  */
/* default  */     set_error("System information on heap storage is not available.");
/* default  */
/* default  */     return ERROR;
/* default  */ }

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                 kjb_malloc
 *
 * Allocates memory from the heap.
 *
 * This routine is a replacement for malloc. It provides some error checking in
 * the case of development code (compiled with -DTEST), some platform
 * independence with respect to malloc's behaviour, and error reporting
 * consistent with the KJB library.
 *
 * If TRACK_MEMORY_ALLOCATION is defined (on unix, this is normally defined if
 * TEST is #defined) then this routine is #define'd to be debug_kjb_malloc,
 * which is the version available in the development library. In development
 * code, memory is tracked so that memory leaks can be found more easily.
 * Furthermore, all memory free'd is checked that it was allocated by a KJB
 * library routine, and that it has not already been freed (this is a common
 * source of nasty bugs). Finally, memory is checked for overruns. Most of these
 * problems are spotted when the code calls kjb_free. If a problem is spotted,
 * then you are notified, and asked if you want to abort. If you then abort,
 * then you simply need to go up a few stack frames with the debugger to find ut
 * where you were free'ing something that you should not be, or what was freed
 * that was over-run. The memory leak check is simply a report supplied at
 * program exit of the memory not freed, and where it was allocated. Since it is
 * often more useful that the allocation locatation is specified with respect
 * to higher level operations (such as creating a matrix), some routines such as
 * create_matrix will set it up so that the location reported is the matrix
 * creation call, not the call to kjb_malloc.
 *
 * The routine kjb_free should be used to free memory allocated by this routine.
 * kjb_free is normally accessed through the macro kjb_free.
 *
 * Macros:
 *    Normally kjb_malloc should be accessed through macros to provide the
 *    casts.  The available macros are:
 *
 *        KJB_MALLOC(x), TYPE_MALLOC(t), N_TYPE_MALLOC(t,n), VOID_MALLOC(x),
 *        UCHAR_MALLOC(x), BYTE_MALLOC(x), STR_MALLOC(x), SHORT_MALLOC(x),
 *        INT_MALLOC(x), LONG_MALLOC(x), FLT_MALLOC(x), DBL_MALLOC(x),
 *        INT16_MALLOC(x), UINT16_MALLOC(x), INT32_MALLOC(x), UINT32_MALLOC(x),
 *        DBL_MALLOC(x)
 *
 *    The use of these is relatively obvious, except possibly N_TYPE_MALLOC,
 *    which takes a type name as the first argument, and the number that you
 *    want as the second.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the memory, just like malloc.
 *
 * Index: memory allocation, debugging
 *
 * -----------------------------------------------------------------------------
 */

#ifdef TRACK_MEMORY_ALLOCATION
void* debug_kjb_malloc(Malloc_size num_bytes,
                       const char* file_name, int line_number)
{
    void* malloc_res;
    unsigned int  i;
    unsigned long total_bytes_used;
#ifdef TEST
    IMPORT int kjb_debug_level;
    IMPORT int kjb_fork_depth; 
#endif 


    if (num_bytes == 0) num_bytes = 1;

    if (    (fs_heap_checking_enable)
         || ((fs_heap_checking_enable_2) && (! fs_heap_checking_skip_2))
       )
    {
        num_bytes += MALLOC_OVER_RUN_SIZE;
    }

#ifdef MALLOC_SIZE_IS_64_BITS
    /*
     * This catches the error that a negative integer was passed with
     * improper cast.
    */
    if (num_bytes > INT64_MAX)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
#else
#if 0 /* ifdef DEF_OUT */
    /* This is a bit risky because asking for a bit more than 2G is now
     * reasonable. However, it does not seem to succeed on linux (you can get
     * about 2.6 GB, but it not contiguous).
    */
#ifdef MALLOC_SIZE_IS_32_BITS
    /*
     * This catches the error that a negative integer was passed with
     * improper cast.
    */
    if (num_bytes > INT32_MAX)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
#endif
#endif
#endif

#ifdef MAX_MALLOC_SIZE
    /*
     * This is almost never used, but has been used in the past to ensure that
     * code runs on architectures with limited allocation sizes.
    */
    if (num_bytes > MAX_MALLOC_SIZE)
    {
        set_error("Memory allocation of %lu request exceeds %lu bytes.",
                  (unsigned long)num_bytes,
                  (unsigned long)MAX_MALLOC_SIZE);
        return NULL;
    }
#endif

#ifdef TEST
    if (    (fs_simulated_heap_failure_frequency >= 0.0)
         && (fs_simulated_heap_failure_frequency > kjb_rand_2())
       )
    {
        set_error("Memory allocation failed due to simulated failure.");
        return NULL;
    }
#endif

#ifdef MS_16_BIT_OS
    /* I have no idea if this is the right call anymore. */
    malloc_res = (void*)farmalloc((unsigned long)num_bytes);
#else
    malloc_res = (void*)malloc(num_bytes);
#endif

#ifdef TEST
    if (kjb_debug_level > 10)
    {
        fprintf(stderr, "Process %ld (fork depth %d) has allocated %lx (IL4RT).\n",
                (long)MY_PID, kjb_fork_depth, (unsigned long)malloc_res);
    }
#endif 
    if (malloc_res == NULL)
    {
        Error_action save_error_action = get_error_action();

        set_error("Memory allocation failed trying to allocate %,lu bytes.",
                  (unsigned long)num_bytes);

        /* FIXME 
         * Very suspect, because currently, set_error_action() allocates. 
        */
        SUSPECT_CODE(); 
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);

        if (memory_used(&total_bytes_used) != ERROR)
        {
            add_error("%,ld bytes already allocated.", total_bytes_used);
        }
        else if (get_allocated_memory(&total_bytes_used) != ERROR)
        {
            add_error("An estimated %,lu bytes are already allocated.",
                      total_bytes_used);
        }

        set_error_action(save_error_action);
    }
#ifdef TEST
    else if ((unsigned long)malloc_res < 256)
    {
        Error_action save_error_action = get_error_action();

        set_error("Memory allocation failed trying to allocate %,lu bytes.",
                   (unsigned long)num_bytes);

        add_error("Unusual return value (%p) from malloc.",
                  malloc_res);

        /* FIXME 
         * Very suspect, because currently, set_error_action() allocates. 
        */
        SUSPECT_CODE(); 
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);

        if (memory_used(&total_bytes_used) != ERROR)
        {
            add_error("%,ld bytes already allocated.", total_bytes_used);
        }
        else if (get_allocated_memory(&total_bytes_used) != ERROR)
        {
            add_error("An estimated %,lu bytes already allocated.", total_bytes_used);
        }

        set_error_action(save_error_action);

        malloc_res = NULL;
    }
#endif
#ifdef MALLINFO
    else
    {   /* Success */

        /* 
         * As best as I can recall, if we are using mallinfo(), then it breaks unless there
         * has been at least one successful return from the storage allocator.
         * Hence this static variable. 
        */
        fs_mallinfo_call_ok = TRUE;
    }
#endif

    if (    (malloc_res != NULL)
         && (    (fs_heap_checking_enable)
              || ((fs_heap_checking_enable_2) && (! fs_heap_checking_skip_2))
            )
       )
    {
        num_bytes -= MALLOC_OVER_RUN_SIZE;

        for (i=0; i<MALLOC_OVER_RUN_SIZE; i++)
        {
            *((unsigned char*)malloc_res + num_bytes + i) = 0xff;
        }

        if (fs_heap_checking_enable)
        {
            save_pointer_info(malloc_res, num_bytes, file_name, line_number, MY_PID);
        }

        if ((fs_heap_checking_enable_2) && (! fs_heap_checking_skip_2))
        {
            save_pointer_info_2(malloc_res, num_bytes, file_name, line_number, MY_PID);
        }
    }

    if (    (malloc_res != NULL)
/*#ifdef DISABLE_CONDITIONAL_INIT */
         && (fs_initialization_checking_enable)
/*#endif */
       )
    {
        for (i=0; i<num_bytes; i++)
        {
            *((unsigned char*)malloc_res + i) = 0xff;
        }
    }

    return malloc_res;
}
        /* ==>                                 /\                             */
        /* ==>    Development code above       ||                             */
#else   /* ------------------------------------------------------------------ */
        /* ==>    Production code below        ||                             */
        /* ==>                                 \/                             */

void* kjb_malloc(Malloc_size num_bytes)
{
    void* malloc_res;


    if (num_bytes == 0) num_bytes = 1;

#ifdef MS_16_BIT_OS
    /* I have no idea if this is the right call anymore. */
    malloc_res = (void*) farmalloc((unsigned long)num_bytes);
#else
    malloc_res = (void*) malloc(num_bytes);
#endif

    if (malloc_res == NULL)
    {
        unsigned long total_bytes_used;
        Error_action save_error_action = get_error_action();

        set_error("Memory allocation failed trying to allocate %,lu bytes.",
                  (unsigned long)num_bytes);

        /* FIXME 
         * Very suspect, because currently, set_error_action() allocates. 
        */
        SUSPECT_CODE(); 
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);

        if (memory_used(&total_bytes_used) != ERROR)
        {
            add_error("%,ld bytes already allocated.", total_bytes_used);
        }

        set_error_action(save_error_action);
    }

#ifdef MALLINFO
    else
    {   /* Success */

        /* 
         * As best as I can recall, if we are using mallinfo(), then it breaks unless there
         * has been at least one successful return from the storage allocator.
         * Hence this static variable. 
        */
        fs_mallinfo_call_ok = TRUE;
    }
#endif

    return malloc_res;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                 kjb_calloc
 *
 * Allocates memory from the heap.
 *
 * This routine is a replacement for calloc. It is similar to kjb_malloc, in
 * anology with calloc. (See kjb_malloc for more information).
 *
 * The routine kjb_free should be used to free memory allocated by this routine.
 * kjb_free is normally accessed through the macro kjb_free.
 *
 * Macros:
 *    Normally kjb_calloc should be accessed through the macro KJB_CALLOC(x).
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.  On
 *    success it returns a pointer to the memory which has been zero'd, just
 *    like calloc.
 *
 * Index: memory allocation, debugging
 *
 * -----------------------------------------------------------------------------
 */

#ifdef TRACK_MEMORY_ALLOCATION
void* debug_kjb_calloc(Malloc_size num_items, Malloc_size item_size,
                       const char* file_name, int line_number)
{
    void* calloc_res;
    int   num_overrun_items = 0;
    unsigned int  i;
    unsigned long total_bytes_used;


    if (num_items == 0) num_items = 1;
    if (item_size == 0) item_size = 1;

    if (    (fs_heap_checking_enable)
         || ((fs_heap_checking_enable_2) && (! fs_heap_checking_skip_2))
       )
    {
        num_overrun_items = ((MALLOC_OVER_RUN_SIZE-1)/item_size)+1;
        ASSERT(num_overrun_items*item_size >= MALLOC_OVER_RUN_SIZE);
        num_items += num_overrun_items;
    }

#ifdef MALLOC_SIZE_IS_64_BITS
    /*
     * This catches the error that a negative integer was passed with
     * improper cast.
    */
    if (    (num_items > INT64_MAX) || (item_size > INT64_MAX)
         || (num_items * item_size > INT64_MAX)
       )
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
#else
#if 0 /* ifdef DEF_OUT */
    /* This is a bit risky because asking for a bit more than 2G is now
     * reasonable. However, it does not seem to succeed on linux (you can get
     * about 2.6 GB, but it not contiguous).
    */
#ifdef MALLOC_SIZE_IS_32_BITS
    /*
     * This catches the error that a negative integer was passed with
     * improper cast.
    */
    if (    (num_items > INT32_MAX) || (item_size > INT32_MAX)
         || (num_items * item_size > INT32_MAX)
       )
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
#endif
#endif
#endif

#ifdef MAX_MALLOC_SIZE
    /*
     * This is almost never used, but has been used in the past to ensure that
     * code runs on architectures with limited allocation sizes.
    */
    if (num_bytes > MAX_MALLOC_SIZE)
    {
        set_error("Memory allocation of %,lu request exceeds %,lu.",
                  (unsigned long)num_bytes,
                  (unsigned long)MAX_MALLOC_SIZE);
        return NULL;
    }
#endif

#ifdef TEST
    if (    (fs_simulated_heap_failure_frequency >= 0.0)
         && (fs_simulated_heap_failure_frequency > kjb_rand_2())
       )
    {
        set_error("Memory allocation failed due to simulated failure.");
        return NULL;
    }
#endif

#ifdef MS_16_BIT_OS
    /* I have no idea if this is the right call anymore. */
    calloc_res = (void*)farcalloc((unsigned long)num_items,
                                  (unsigned long)item_size);
#else
    calloc_res = (void*)calloc(num_items, item_size);
#endif

    if (calloc_res == NULL)
    {
        Error_action save_error_action = get_error_action();

        set_error("Memory allocation failed trying to allocate %,lu bytes.",
                  (unsigned long)(num_items * item_size));

        /* FIXME 
         * Very suspect, because currently, set_error_action() allocates. 
        */
        SUSPECT_CODE(); 
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);

        if (memory_used(&total_bytes_used) != ERROR)
        {
            add_error("%,ld bytes already allocated.", total_bytes_used);
        }
        else if (get_allocated_memory(&total_bytes_used) != ERROR)
        {
            add_error("An estimated %,lu bytes have been already allocated.",
                      total_bytes_used);
        }

        set_error_action(save_error_action);
    }
#ifdef UNIX
#ifdef TEST
    else if ((unsigned long)calloc_res < 256)
    {
        set_bug("Unusual return value (%p) from calloc.", calloc_res);
        calloc_res = NULL;
    }
#endif    /* TEST */
#endif

#ifdef MALLINFO
    else
    {   
        /* Success */

        /* 
         * As best as I can recall, if we are using mallinfo(), then it breaks unless there
         * has been at least one successful return from the storage allocator.
         * Hence this static variable. 
        */
        fs_mallinfo_call_ok = TRUE;
    }
#endif

    if (    (calloc_res != NULL)
         && (    (fs_heap_checking_enable)
              || ((fs_heap_checking_enable_2) && (! fs_heap_checking_skip_2))
            )
       )
    {
        num_items -= num_overrun_items;

        for (i=0; i<MALLOC_OVER_RUN_SIZE; i++)
        {
            *((unsigned char*)calloc_res + item_size * num_items + i) = 0xff;
        }

        if (fs_heap_checking_enable)
        {
            save_pointer_info(calloc_res, item_size * num_items, file_name,
                              line_number, MY_PID);
        }

        if ((fs_heap_checking_enable_2) && (! fs_heap_checking_skip_2))
        {
            save_pointer_info_2(calloc_res, item_size * num_items, file_name,
                                line_number, MY_PID);
        }
    }

    return calloc_res;
}
        /* ==>                                 /\                             */
        /* ==>    Development code above       ||                             */
#else   /* ------------------------------------------------------------------ */
        /* ==>    Production code below        ||                             */
        /* ==>                                 \/                             */

void* kjb_calloc(Malloc_size num_items, Malloc_size item_size)
{
    void* calloc_res;
    unsigned long total_bytes_used;


    if (num_items == 0) num_items = 1;
    if (item_size == 0) item_size = 1;

#ifdef MS_16_BIT_OS
    /* I have no idea if this is the right call anymore. */
    calloc_res = (void*) farcalloc((unsigned long)num_items,
                                   (unsigned long)item_size);
#else
    calloc_res = (void*) calloc(num_items, item_size);
#endif

    if (calloc_res == NULL)
    {
        Error_action save_error_action = get_error_action();

        set_error("Memory allocation failed trying to allocate %,lu bytes.",
                  (unsigned long)(num_items * item_size));

        /* FIXME 
         * Very suspect, because currently, set_error_action() allocates. 
        */
        SUSPECT_CODE(); 
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);

        if (memory_used(&total_bytes_used) != ERROR)
        {
            add_error("%,ld bytes already allocated.", total_bytes_used);
        }

        set_error_action(save_error_action);
    }

#ifdef MALLINFO
    else
    {   
        /* Success */

        /* 
         * As best as I can recall, if we are using mallinfo(), then it breaks unless there
         * has been at least one successful return from the storage allocator.
         * Hence this static variable. 
        */
        fs_mallinfo_call_ok = TRUE;
    }
#endif


    return calloc_res;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION  ...   #else  ....    */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                 kjb_realloc
 *
 * Reallocates memory from the heap.
 *
 * This routine is a replacement for realloc. It provides some error checking in
 * the case of development code (compiled with -DTEST), some platform
 * independence with respect to remalloc's behaviour, and error reporting
 * consistent with the KJB library.
 *
 * If TRACK_MEMORY_ALLOCATION is defined (on unix, this is normally defined if
 * TEST is #defined) then this routine is #define'd to be debug_kjb_realloc,
 * which is the version available in the development library. In development
 * code, memory is tracked so that memory leaks can be found more easily.
 * Furthermore, all memory free'd is checked that it was allocated by a KJB
 * library routine, and that it has not already been freed (this is a common
 * source of nasty bugs). Finally, memory is checked for overruns. Most of these
 * problems are spotted when the code calls kjb_free. If a problem is spotted,
 * then you are notified, and asked if you want to abort. If you then abort,
 * then you simply need to go up a few stack frames with the debugger to find ut
 * where you were free'ing something that you should not be, or what was freed
 * that was over-run. The memory leak check is simply a report supplied at
 * program exit of the memory not freed, and where it was allocated. Since it is
 * often more useful that the allocation location is specified with respect
 * to higher level operations (such as creating a matrix), some routines such as
 * create_matrix will set it up so that the location reported is the matrix
 * creation call, not the call to kjb_realloc.
 *
 * The routine kjb_free should be used to free memory allocated by this routine.
 * kjb_free is normally accessed through the macro kjb_free.
 *
 * Macros:
 *    Normally kjb_realloc should be accessed through macros to provide the
 *    casts.  The available macros are:
 *
 *        KJB_REALLOC(x), TYPE_REALLOC(t), N_TYPE_REALLOC(t,n), VOID_REALLOC(x),
 *        UCHAR_REALLOC(x), BYTE_REALLOC(x), STR_REALLOC(x), SHORT_REALLOC(x),
 *        INT_REALLOC(x), LONG_REALLOC(x), FLT_REALLOC(x), DBL_REALLOC(x),
 *        INT16_REALLOC(x), UINT16_REALLOC(x), INT32_REALLOC(x), UINT32_REALLOC(x),
 *        DBL_REALLOC(x)
 *
 *    The use of these is relatively obvious, except possibly N_TYPE_REALLOC,
 *    which takes a type name as the first argument, and the number that you
 *    want as the second.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the memory, just like malloc.
 *
 * Index: memory allocation, debugging
 *
 * -----------------------------------------------------------------------------
 */
 
#ifdef TRACK_MEMORY_ALLOCATION 
void* debug_kjb_realloc(void* ptr, Malloc_size num_bytes,
                        const char* file_name, int line_number)
{
    void* realloc_res;
    unsigned int  i;
    unsigned long total_bytes_used;
    int ptr_index = NOT_SET;
    int ptr_index_2 = NOT_SET;
    int prev_num_bytes = NOT_SET; 


    /* realloc(ptr) is legal when ptr is NULL, and is equivalent to malloc().*/
    if ( ptr==NULL )
    {
        return debug_kjb_malloc( num_bytes, file_name, line_number );
    }

    if (fs_heap_checking_enable)
    {
        ptr_index = lookup_pointer_index(ptr);

        if (ptr_index == NOT_SET)
        {
            set_error("Attempting to realloc a pointer that has not been allocated."); 
            add_error("Called via line %ld in file %s.",
                      line_number, file_name); 
            return NULL; 
        }

        prev_num_bytes = fs_amount_allocated[ ptr_index ];
    }

    if ((fs_heap_checking_enable_2) && (! fs_heap_checking_skip_2))
    {
        ptr_index_2 = lookup_pointer_index_2(ptr);

        /*
         * It seems to me that reallocating a pointer allocated outside the
         * check 2 area should be OK. 
        */
    }

    /* Note:  the following line deviates from ISO C semantics, which says that
     * realloc with num_bytes==0 is equivalent to a free().  However, does
     * anyone (intentionally) use realloc that way?  If so, then we might want
     * to change it; otherwise the following seems reasonable:
     */
    if (num_bytes == 0) num_bytes = 1;

    if (    (fs_heap_checking_enable)
         || ((fs_heap_checking_enable_2) && (! fs_heap_checking_skip_2))
       )
    {
        num_bytes += MALLOC_OVER_RUN_SIZE;
    }

#ifdef MALLOC_SIZE_IS_64_BITS
    /*
     * This catches the error that a negative integer was passed with
     * improper cast.
    */
    if (num_bytes > INT64_MAX)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
#else
#if 0 /* ifdef DEF_OUT */
    /* This is a bit risky because asking for a bit more than 2G is now
     * reasonable. However, it does not seem to succeed on linux (you can get
     * about 2.6 GB, but it not contiguous).
    */
#ifdef MALLOC_SIZE_IS_32_BITS
    /*
     * This catches the error that a negative integer was passed with
     * improper cast.
    */
    if (num_bytes > INT32_MAX)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
#endif
#endif
#endif

#ifdef MAX_MALLOC_SIZE
    /*
     * This is almost never used, but has been used in the past to ensure that
     * code runs on architectures with limited allocation sizes.
    */
    if (num_bytes > MAX_MALLOC_SIZE)
    {
        set_error("Memory allocation of %lu request exceeds %lu bytes.",
                  (unsigned long)num_bytes,
                  (unsigned long)MAX_MALLOC_SIZE);
        return NULL;
    }
#endif

#ifdef TEST
    if (    (fs_simulated_heap_failure_frequency >= 0.0)
         && (fs_simulated_heap_failure_frequency > kjb_rand_2())
       )
    {
        set_error("Memory allocation failed due to simulated failure.");
        return NULL;
    }
#endif

#ifdef MS_16_BIT_OS
    /* I have no idea if this is the right call anymore. */
    realloc_res = (void*)farrealloc(ptr, (unsigned long)num_bytes);
#else
    realloc_res = (void*)realloc(ptr, num_bytes);
#endif

    if (realloc_res == NULL)
    {
        Error_action save_error_action = get_error_action();

        set_error("Memory allocation failed trying to reallocate %,lu bytes.",
                  (unsigned long)num_bytes);

        /* FIXME 
         * Very suspect, because currently, set_error_action() allocates. 
        */
        SUSPECT_CODE(); 
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);

        if (memory_used(&total_bytes_used) != ERROR)
        {
            add_error("%,ld bytes already allocated.", total_bytes_used);
        }
        else if (get_allocated_memory(&total_bytes_used) != ERROR)
        {
            add_error("An estimated %,lu bytes are already allocated.",
                      total_bytes_used);
        }

        set_error_action(save_error_action);
    }
#ifdef TEST
    else if ((unsigned long)realloc_res < 256)
    {
        Error_action save_error_action = get_error_action();

        set_error("Memory allocation failed trying to reallocate %,lu bytes.",
                   (unsigned long)num_bytes);

        add_error("Unusual return value (%p) from realloc.",
                  realloc_res);

        /* FIXME 
         * Very suspect, because currently, set_error_action() allocates. 
        */
        SUSPECT_CODE(); 
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);

        if (memory_used(&total_bytes_used) != ERROR)
        {
            add_error("%,ld bytes already allocated.", total_bytes_used);
        }
        else if (get_allocated_memory(&total_bytes_used) != ERROR)
        {
            add_error("An estimated %,lu bytes already allocated.", total_bytes_used);
        }

        /* FIXME 
         * Very suspect, because currently, set_error_action() allocates. 
        */
        SUSPECT_CODE(); 
        set_error_action(save_error_action);

        realloc_res = NULL;
    }
#endif

    if (    (realloc_res != NULL)
         && (    (fs_heap_checking_enable)
              || ((fs_heap_checking_enable_2) && (! fs_heap_checking_skip_2))
            )
       )
    {
        num_bytes -= MALLOC_OVER_RUN_SIZE;

        for (i=0; i<MALLOC_OVER_RUN_SIZE; i++)
        {
            *((unsigned char*)realloc_res + num_bytes + i) = 0xff;
        }

        if (fs_heap_checking_enable)
        {
            set_pointer_info(realloc_res, num_bytes, file_name, line_number, MY_PID, ptr_index);
        }

        if (    (fs_heap_checking_enable_2) 
             && (! fs_heap_checking_skip_2)
             && (KJB_IS_SET(ptr_index_2))
           )
        {
            UNTESTED_CODE(); 
            set_pointer_info(realloc_res, num_bytes, file_name, line_number, MY_PID, ptr_index_2);
        }
    }

    if (    (realloc_res != NULL)
/*#ifdef DISABLE_CONDITIONAL_INIT */
         && (fs_initialization_checking_enable)
/*#endif */
         && (KJB_IS_SET(prev_num_bytes))
       )
    {
        for (i=prev_num_bytes; i<num_bytes; i++)
        {
            *((unsigned char*)realloc_res + i) = 0xff;
        }
    }

    return realloc_res;
}
        /* ==>                                 /\                             */
        /* ==>    Development code above       ||                             */
#else   /* ------------------------------------------------------------------ */
        /* ==>    Production code below        ||                             */
        /* ==>                                 \/                             */

void* kjb_realloc(void* ptr, Malloc_size num_bytes)
{
    void* realloc_res;


    /* realloc(ptr) is legal when ptr is NULL, and is equivalent to malloc().*/
    if ( ptr==NULL )
    {
        return kjb_malloc( num_bytes );
    }

    if (num_bytes == 0) num_bytes = 1;

#ifdef MS_16_BIT_OS
    /* I have no idea if this is the right call anymore. */
    realloc_res = (void*) farrealloc(ptr, (unsigned long)num_bytes);
#else
    realloc_res = (void*) realloc(ptr, num_bytes);
#endif

    if (realloc_res == NULL)
    {
        unsigned long total_bytes_used;
        Error_action save_error_action = get_error_action();

        set_error("Memory allocation failed trying to reallocate %,lu bytes.",
                  (unsigned long)num_bytes);

        /* FIXME 
         * Very suspect, because currently, set_error_action() allocates. 
        */
        SUSPECT_CODE(); 
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);

        if (memory_used(&total_bytes_used) != ERROR)
        {
            add_error("%,ld bytes already allocated.", total_bytes_used);
        }

        set_error_action(save_error_action);
    }

    return realloc_res;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void save_pointer_info(void* ptr, Malloc_size num_bytes,
                              const char* file_name, int line_number, 
                              int pid)
{
    int i;
#ifdef TEST
    IMPORT int kjb_debug_level;
    IMPORT int kjb_fork_depth; 
#endif 

    fs_fast_lookup_is_valid = FALSE;

    if (fs_num_pointers == 0)
    {
        /* Double call is OK. */
        arrange_cleanup();

        for (i=0; i<MAX_NUM_POINTERS; i++)
        {
            fs_pointers_to_allocated[ i ] = NULL;
        }
    }
    else if (fs_num_pointers == MAX_NUM_POINTERS)
    {
#ifdef TEST
        if (kjb_debug_level > 5)
        {
            /* Too dangerous to use KJB IO at this point.  */
            fprintf(stderr,
                    "<<TEST>> Process %ld (fork depth %d) compacting \"checked\" pointers because table is full ...",
                    (long)MY_PID, kjb_fork_depth);
        }
#endif

        fs_num_pointers = 0;

        for (i=0; i<MAX_NUM_POINTERS; i++)
        {
            if (fs_pointers_to_allocated[ i ] != NULL)
            {
                fs_pointers_to_allocated[ fs_num_pointers ] =
                                                    fs_pointers_to_allocated[ i ];

                fs_amount_allocated[ fs_num_pointers ] = fs_amount_allocated[ i ];

                BUFF_CPY(fs_allocation_file_name[ fs_num_pointers ],
                         fs_allocation_file_name[ i ]);

                fs_allocation_line_number[ fs_num_pointers ] =
                                                   fs_allocation_line_number[ i ];

                fs_allocation_pid[ fs_num_pointers ] = fs_allocation_pid[ i ];

                fs_num_pointers++;
            }
        }

#ifdef LOOKS_UNECESSARY
        for (i = fs_num_pointers; i<MAX_NUM_POINTERS; i++)
        {
            fs_pointers_to_allocated[ i ] = NULL;
        }
#endif

#ifdef TEST
        if (kjb_debug_level > 5)
        {
            fprintf(stderr, " done (IL4RR).\n\n");
        }
#endif
    }

    if (fs_num_pointers < MAX_NUM_POINTERS)
    {
        set_pointer_info(ptr, num_bytes, file_name, line_number, pid, fs_num_pointers);
        fs_num_pointers++;
    }
    else if (fs_num_pointers == MAX_NUM_POINTERS)
    {
        /*
        // Too dangerous to use KJB IO at this point.
        */
        fprintf(stderr, "\nMax num \"checked\" pointers exceeded.\n");
        fprintf(stderr, "Pointer checking is now disabled.\n");

        /*
        //  Make it so that pointer checking is disabled.
        */
        fs_num_pointers++;
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void set_pointer_info(void* ptr, Malloc_size num_bytes,
                             const char* file_path, int line_number,
                             int pid, int ptr_index)
{
    char file_path_copy[ MAX_PATH_SIZE ];
    char file_path_last_element[ MAX_PATH_SIZE ];


    ASSERT_IS_NON_NEGATIVE_INT(ptr_index);
    ASSERT_IS_LESS_INT(ptr_index, MAX_NUM_POINTERS); 

    fs_pointers_to_allocated[ ptr_index ]  = (void*)ptr;
    fs_amount_allocated[ ptr_index ]       = num_bytes;
    fs_allocation_line_number[ ptr_index ] = line_number;
    fs_allocation_pid[ ptr_index ] = pid;

    BUFF_CPY(file_path_copy, file_path); 
    BUFF_GEN_GET_LAST_TOKEN(file_path_copy, file_path_last_element, "/");

    BUFF_TRUNC_CPY(fs_allocation_file_name[ ptr_index ], 
                   file_path_last_element);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void save_pointer_info_2(void* ptr, Malloc_size num_bytes,
                                const char* file_name, int line_number,
                                int pid)
{
    int i;
#ifdef TEST
    IMPORT int kjb_debug_level;
    IMPORT int kjb_fork_depth; 
#endif 


    if ((! fs_heap_checking_enable_2) || (fs_heap_checking_skip_2))
    {
        SET_CANT_HAPPEN_BUG();
        return;
    }

    fs_fast_lookup_is_valid_2 = FALSE;

    if (fs_num_pointers_2 == 0)
    {
        /* Unlike save_pointer_info, no need to arrange cleanup. */
        /*
        arrange_cleanup();
        */

        for (i=0; i<MAX_NUM_POINTERS_2; i++)
        {
            fs_pointers_to_allocated_2[ i ] = NULL;
        }
    }
    else if (fs_num_pointers_2 == MAX_NUM_POINTERS_2)
    {
#ifdef TEST
        if (kjb_debug_level > 5)
        {
            /* Too dangerous to use KJB IO at this point.  */
            fprintf(stderr,
                    "\nProcess %ld (fork depth %d) compacting \"checked\" pointers (2) because table is full ...",
                    (long)MY_PID, kjb_fork_depth);
        }
#endif

        fs_num_pointers_2 = 0;

        for (i=0; i<MAX_NUM_POINTERS_2; i++)
        {
            if (fs_pointers_to_allocated_2[ i ] != NULL)
            {
                fs_pointers_to_allocated_2[ fs_num_pointers_2 ] =
                                                    fs_pointers_to_allocated_2[ i ];

                fs_amount_allocated_2[ fs_num_pointers_2 ] = fs_amount_allocated_2[ i ];

                BUFF_CPY(fs_allocation_file_name_2[ fs_num_pointers_2 ],
                         fs_allocation_file_name_2[ i ]);

                fs_allocation_line_number_2[ fs_num_pointers_2 ] =
                                                   fs_allocation_line_number_2[ i ];

                fs_allocation_pid_2[ fs_num_pointers_2 ] = fs_allocation_pid_2[ i ];

                fs_num_pointers_2++;
            }
        }

        /*
         * July 2, 2004: This was flagged as possibly being unnecessary, and was
         * commented out. However, since there is some indication that this code
         * may have a bug, it seems better to be sure. If this code is not
         * needed, then precumably, the initializaion code for when
         * fs_num_pointers_2 is zero is not needed either?
        */
        for (i = fs_num_pointers_2; i<MAX_NUM_POINTERS_2; i++)
        {
            fs_pointers_to_allocated_2[ i ] = NULL;
        }

        if (kjb_debug_level > 5)
        {
            fprintf(stderr, " done (IL4RT).\n");
            fprintf(stderr, "    (%d pointer slots recovered).\n\n",
                    MAX_NUM_POINTERS_2 - fs_num_pointers_2);
        }
    }

    if (fs_num_pointers_2 < MAX_NUM_POINTERS_2)
    {
        set_pointer_info_2(ptr, num_bytes, file_name, line_number,
                           pid, fs_num_pointers_2);
        fs_num_pointers_2++;
    }
    else if (fs_num_pointers_2 == MAX_NUM_POINTERS_2)
    {
        /*
        // Too dangerous to use KJB IO at this point.
        */
        fprintf(stderr, "\nMax num \"checked\" pointers (2) exceeded.\n");
        fprintf(stderr, "Pointer checking (2) is now disabled.\n\n");

        /*
        //  Make it so that pointer checking is disabled.
        */
        fs_num_pointers_2++;
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void set_pointer_info_2(void* ptr, Malloc_size num_bytes,
                               const char* file_path, int line_number,
                               int pid, int ptr_index)
{
    char file_path_copy[ MAX_PATH_SIZE ];
    char file_path_last_element[ MAX_PATH_SIZE ];


    ASSERT_IS_NON_NEGATIVE_INT(ptr_index);
    ASSERT_IS_LESS_INT(ptr_index, MAX_NUM_POINTERS_2); 

    fs_pointers_to_allocated_2[ ptr_index ]  = (void*)ptr;
    fs_amount_allocated_2[ ptr_index ]       = num_bytes;
    fs_allocation_line_number_2[ ptr_index ] = line_number;
    fs_allocation_pid_2[ ptr_index ] = pid;

    BUFF_CPY(file_path_copy, file_path); 

    BUFF_GEN_GET_LAST_TOKEN(file_path_copy, file_path_last_element, "/");

    BUFF_TRUNC_CPY(fs_allocation_file_name_2[ ptr_index ], 
                   file_path_last_element);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


void null_and_float_align(char** ptr_ptr)
{

#ifdef PTR_IS_32_BITS
    while (! FLT_ALIGNED((kjb_uint32)(*ptr_ptr)))
#else
#ifdef PTR_IS_64_BITS
    while (! FLT_ALIGNED((kjb_uint64)(*ptr_ptr)))
#endif
#endif
    {
        **ptr_ptr = '\0';
        (*ptr_ptr)++;
    }
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void float_align(char** ptr_ptr)
{

#ifdef PTR_IS_32_BITS
    while (! FLT_ALIGNED((kjb_uint32)(*ptr_ptr)))
#else
#ifdef PTR_IS_64_BITS
    while (! FLT_ALIGNED((kjb_uint64)(*ptr_ptr)))
#endif
#endif
    {
        (*ptr_ptr)++;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int print_allocated_memory(FILE* fp)
{
#ifdef TRACK_MEMORY_ALLOCATION
    unsigned long  total_bytes = 0;
    unsigned long  own_total_bytes = 0;
    int    i;
    int   minimal_report = FALSE;
#ifdef TEST
    IMPORT int kjb_debug_level;
    IMPORT int kjb_fork_depth; 
#endif 
#endif
    unsigned long  heap_memory;


    /*
    // Special case, normally used at program termination.
    */
    if (fp == NULL)
    {
#ifdef TRACK_MEMORY_ALLOCATION
        minimal_report = TRUE;
#endif
        fp = stderr;
    }

#ifdef TRACK_MEMORY_ALLOCATION

    /*
    // If fs_num_pointers is greater than MAX_NUM_POINTERS the
    // table is full and heap checking is disabled.
    */
    if (fs_num_pointers <= MAX_NUM_POINTERS)
    {
        for (i=0; i<fs_num_pointers; i++)
        {
            if (fs_pointers_to_allocated[ i ] != NULL)
            {
                if (fs_allocation_pid[ i ] == MY_PID)
                {
                    own_total_bytes += fs_amount_allocated[ i ];
                }
                total_bytes += fs_amount_allocated[ i ];
            }
        }
    }

    if (    ! fs_heap_checking_enable
         || ((total_bytes == 0) && minimal_report)
       )
    {
#ifdef TEST
        /* If debug level is high enough, acknowledge that we don't have leaks. */
        if (kjb_debug_level > 2)
        {
            fprintf(fp, "\nResidual memory allocation for process %ld (fork depth %d) is %ld (IL4RT).\n",
                    (long)MY_PID, kjb_fork_depth, total_bytes);
        }
#endif 

        return NO_ERROR;
    }

    /*
    // Use fprintf as opposed to kjb_fprintf to avoid allocating any more memory
    // or doing anything that may invoke add_cleanup_function. Also, unlike
    // other messages that have PIDs, we do not use the string to ignore like
    // for regression testing (IL4RT) because we should not be leaking memory to
    // pass tests. 
    */
    fprintf(fp, "\nResidual memory allocation for process %ld (fork depth %d): \n",
            (long)MY_PID, kjb_fork_depth);

    /*
    // If fs_num_pointers is greater than MAX_NUM_POINTERS the
    // table is full and heap checking is disabled.
    */
    if (fs_num_pointers <= MAX_NUM_POINTERS)
    {
        for (i=0; i<fs_num_pointers; i++)
        {
            if (fs_pointers_to_allocated[ i ] != NULL)
            {
                if (fs_allocation_pid[ i ] == MY_PID)
                {
                    fprintf(fp, "  %16lx  %10lu    %-*s %5d\n",
                            (unsigned long)fs_pointers_to_allocated[i],
                            (unsigned long)fs_amount_allocated[ i ],
                            MAX_ALLOCATION_FILE_NAME_LEN,
                            fs_allocation_file_name[ i ],
                            fs_allocation_line_number[ i ]);
                }
#ifdef TEST
                else if (kjb_debug_level > 3)
                {
                    fprintf(fp, "Inherited from a parent (%d) : %16lx  %10lu    %-*s %5d\n",
                            (int)fs_allocation_pid[ i ],
                            (unsigned long)fs_pointers_to_allocated[i],
                            (unsigned long)fs_amount_allocated[ i ],
                            MAX_ALLOCATION_FILE_NAME_LEN,
                            fs_allocation_file_name[ i ],
                            fs_allocation_line_number[ i ]);
                }
#endif 
            }
        }
        fprintf(fp, "\nSum of allocated (excluding inherited): %ld\n", own_total_bytes);
        fprintf(fp, "Sum of allocated (total):               %ld\n\n", total_bytes);
    }

#endif

    if ((memory_used(&heap_memory)) != ERROR)
    {
#ifdef TRACK_MEMORY_ALLOCATION
        fprintf(fp, "    ");
#endif
        fprintf(fp, "Heap memory used: %lu\n\n", heap_memory);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

void start_heap_check_2(const char* file_name, int line_number)
{

    if (fs_heap_checking_enable_2)
    {
        p_stderr("Heap check 2 is already started.\n");
        return;
    }

    fs_heap_checking_enable_2 = TRUE;
    fs_heap_checking_skip_2 = FALSE;
    BUFF_CPY(fs_start_heap_check_2_file_name, file_name);
    fs_start_heap_check_2_line_number = line_number;
    fs_num_pointers_2 = 0;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

void disable_heap_check_2(void)
{


    fs_heap_checking_enable_2 = FALSE;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

/*
 * July 2, 2004: The way this routine is used, the return value was never
 * checked. So better to print messages and make the return void.
*/
void finish_heap_check_2(const char* file_name, int line_number)
{
    long  total_bytes = 0, i;
#ifdef TEST
    IMPORT int kjb_debug_level;
    IMPORT int kjb_fork_depth; 
#endif 

    UNTESTED_CODE();

    if ( ! fs_heap_checking_enable_2)
    {
        p_stderr("Cannot complete memory check 2 because it is disabled.\n");
        return;
    }

    if (fs_num_pointers_2 > MAX_NUM_POINTERS_2)
    {
        p_stderr("Cannot complete memory check 2 because the table got full.\n");
        return;
    }

    for (i=0; i<fs_num_pointers_2; i++)
    {
        if (fs_pointers_to_allocated_2[ i ] != NULL)
        {
            if (fs_allocation_pid_2[ i ] == MY_PID)
            {
                total_bytes += fs_amount_allocated_2[ i ];
            }
        }
    }

    if (total_bytes > 0)
    {
        /*
        // Use fprintf as opposed to kjb_printf to avoid allocating any more
        // memory or doing anything that may invoke add_cleanup_function.
        // Also, we do not use IL4RT because to pass tests we should not leak
        // memory. 
        */
        fprintf(stderr, "\nLeaked by pid %d in check 2 (excluding inherited allocations)\n", MY_PID);
        fprintf(stderr, "   %s:%d\n", fs_start_heap_check_2_file_name,
                fs_start_heap_check_2_line_number);
        fprintf(stderr, "   %s:%d\n\n",
                file_name, line_number);

        /*
        // If fs_num_pointers is greater than MAX_NUM_POINTERS the
        // table is full and heap checking is disabled.
        */
        if (fs_num_pointers_2 <= MAX_NUM_POINTERS_2)
        {
            for (i=0; i<fs_num_pointers_2; i++)
            {
                if (fs_pointers_to_allocated_2[ i ] != NULL)
                {
                    if (fs_allocation_pid_2[ i ] == MY_PID)
                    {
                        fprintf(stderr, "   %16lx  %10lu    %-*s %5d\n",
                                (unsigned long)fs_pointers_to_allocated_2[i],
                                (unsigned long)fs_amount_allocated_2[ i ],
                                MAX_ALLOCATION_FILE_NAME_LEN,
                                fs_allocation_file_name_2[ i ],
                                fs_allocation_line_number_2[ i ]);
                    }
                }
            }
            fprintf(stderr, "\nSum of allocated 2: %ld\n\n", total_bytes);
        }
    }

#ifdef TEST
    if (kjb_debug_level > 2)
    {
        for (i=0; i<fs_num_pointers_2; i++)
        {
            if (fs_pointers_to_allocated_2[ i ] != NULL)
            {
                if (fs_allocation_pid_2[ i ] != MY_PID)
                {
                    fprintf(stderr, "Inherited from a parent: %16lx  %10lu   %-*s %5d\n",
                            (unsigned long)fs_pointers_to_allocated_2[i],
                            (unsigned long)fs_amount_allocated_2[ i ],
                            MAX_ALLOCATION_FILE_NAME_LEN,
                            fs_allocation_file_name_2[ i ],
                            fs_allocation_line_number_2[ i ]);
                }
            }
        }
    }
#endif 

    /*
    // Reset the table.
    */
    fs_heap_checking_enable_2 = FALSE;
    BUFF_CPY(fs_start_heap_check_2_file_name, "NOT SET");
    fs_start_heap_check_2_line_number = NOT_SET;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_allocated_memory(unsigned long* total_bytes_ptr)
{
    int result = NO_ERROR;
    long unsigned total_bytes = 0;
#ifdef TRACK_MEMORY_ALLOCATION
    long i;


    /*
    // If fs_num_pointers is greater than MAX_NUM_POINTERS the
    // table is full and heap checking is disabled.
    */
    if (! fs_heap_checking_enable)
    {
        set_error("Unable to estimate allocated memory because heap checking is disabled.");
        result = ERROR;
    }
    else if (fs_num_pointers > MAX_NUM_POINTERS)
    {
        set_error("Unable to estimate allocated memory because table is full.");
        result = ERROR;
    }
    else
    {
        for (i=0; i<fs_num_pointers; i++)
        {
            if (fs_pointers_to_allocated[ i ] != NULL)
            {
                total_bytes += fs_amount_allocated[ i ];
            }
        }
    }

#else

    result = ERROR;

#endif

    *total_bytes_ptr = total_bytes;

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

int debug_print_heap_memory_used(char* file, int line)
{
    unsigned long heap_memory;


    if (memory_used(&heap_memory) != ERROR)
    {
        TEST_PSO(("Heap memory used is %,lu at %s:%d\n", heap_memory,
                 file, line));
    }

    return NO_ERROR;
}

#else

/*
// Nothing here! In the non-test case, "print_heap_memory_used" is #define'd as
// NULL.
*/

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  watch_for_free
 *
 * (Debugging) Watches for free of a pointer
 *
 * In development code, this routine checks for an inadvertant free of the
 * pointer argument. The watch is terminated by using a NULL argument. If the
 * pointer is freed before that time, then you are prompted for an abort. To
 * state the obvious, the watch should be terminated before a non-inadvertant
 * free is called. The typical use is :
 *
 * |     allocation of "ptr"
 * |     watch_for_free(ptr);
 * |     code which is suspected of freeing ptr when it should not
 * |     watch_for_free(NULL);
 * |     free of "ptr"
 *
 *
 * In production code, watch_for_free() is defined as a NULL macro, so it
 * disapears. However, normally watch_for_free() is simply inserted for a run,
 * and then removed.
 *
 * Unfortunately, only one pointer can be watched at present.
 *
 * Index: memory allocation, debugging
 *
 * -----------------------------------------------------------------------------
 */

#ifdef TRACK_MEMORY_ALLOCATION_OR_C2MAN

void watch_for_free(void* ptr)
{
    fs_pointer_to_watch = ptr;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  check_initialization
 *
 * (Debugging) Checks that memory was initialized
 *
 * In development code, this routine checks for memory which should have been
 * initialized, but was not. It can only be used for memory where consecutive
 * 0xff for block_size bytes is invalid data (as in the case of floats with
 * block_size 4 and and doubles with block_size 8 but not ints and char). If the
 * initialization checking option is not set, then this routine silently does
 * nothing
 *
 * Index: memory allocation, debugging
 *
 * -----------------------------------------------------------------------------
 */

#ifdef TRACK_MEMORY_ALLOCATION_OR_C2MAN

void check_initialization(const void* ptr,
                          size_t count, /* Number of items of size block_size to check. */
                          size_t block_size /* Size of item in bytes. */
                         )
{
    const unsigned char* uchar_ptr = (const unsigned char*)ptr;
    size_t i, j;
    int    ok;

    if ( ! fs_initialization_checking_enable) return;

    /*
     * Figured out the hard way. We have to check that the pointer is valid
     * first, as double free's can look like non-initialized memory. This check
     * is even better put in the calling code, where the structure pointer
     * (e.g., a matrix pointer), can be checked for validity first. If it is not
     * valid, then ptr and (count or block_size) will be bogus. However, we
     * still need ptr to be valid, and thus it is fair to check is also.
    */
    kjb_check_free(ptr);

    for (i=0; i<count; i++)
    {
        ok = FALSE;

        for (j=0; j<block_size; j++)
        {
            if (*uchar_ptr != 0xff)
            {
                ok = TRUE;
            }
            uchar_ptr++;
        }

        if (!ok)
        {
            set_bug("Uninitialized memory detected.");
            return;
        }
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  optimize_free
 *
 * Speeds up frees while heap checking
 *
 * If this routine is called before a code segment which does lots of frees,
 * then heap checking should go faster. If the optimization succeeds, then an
 * internal validity flag is set to true, and then the fast lookup is used until
 * the table changes.
 *
 * Index: memory allocation, debugging
 *
 * -----------------------------------------------------------------------------
 */

void optimize_free(void)
{
#ifdef TRACK_MEMORY_ALLOCATION
    int i;
    int save_num_pointers = fs_num_pointers;
#ifdef TEST
    IMPORT int kjb_debug_level;
    IMPORT int kjb_fork_depth; 
#endif 

    if ( ! fs_heap_checking_enable) return;

    /*
    // If fs_num_pointers is greater than MAX_NUM_POINTERS the
    // table is full and heap checking is disabled.
    */
    if (fs_num_pointers > MAX_NUM_POINTERS)
    {
        return;
    }

    if (fs_fast_lookup_is_valid) return;

    /* CONSTCOND */
    if (sizeof(void*) != sizeof(long))
    {
#ifdef TEST
        if (kjb_debug_level > 5)
        {
            /* Too dangerous to use KJB IO at this point.  */
            fprintf(stderr,
                    "\nProcess %ld (fork depth %d) skipping compacting \"checked\" pointers because of size of pointer is not size of long (IL4RT).\n",
                    (long)MY_PID, kjb_fork_depth);
        }
#endif
        return;
    }

#ifdef TEST
    if (kjb_debug_level > 5)
    {
        /* Too dangerous to use KJB IO at this point.  */
        fprintf(stderr,
                "<<TEST>> Process %ld (fork depth %d) compacting \"checked\" pointers for optimization ... ",
                (long)MY_PID, kjb_fork_depth);
    }
#endif

    fs_num_pointers = 0;

    for (i=0; i<save_num_pointers; i++)
    {
        if (fs_pointers_to_allocated[ i ] != NULL)
        {
            fs_pointers_to_allocated[ fs_num_pointers ] = fs_pointers_to_allocated[ i ];
            fs_amount_allocated[ fs_num_pointers ] = fs_amount_allocated[ i ];

            BUFF_CPY(fs_allocation_file_name[ fs_num_pointers ],
                     fs_allocation_file_name[ i ]);

            fs_allocation_line_number[ fs_num_pointers ] =
                                               fs_allocation_line_number[ i ];

            fs_allocation_pid[ fs_num_pointers ] = fs_allocation_pid[ i ];

            fs_num_pointers++;
        }
    }

#ifdef TEST
    if (kjb_debug_level > 5)
    {
        /* Too dangerous to use KJB IO at this point.  */
        fprintf(stderr, " done (IL4RT).\n");
        fprintf(stderr, "Setting up for sort for optimizing free.\n");
        fflush(stderr);  /* Stderr normally does not want a flush, but ... */
    }
#endif

    for (i = 0; i < fs_num_pointers; i++)
    {
        /* Clearly a bit dodgy, but note that we are currently only where if
         * sizeof(void*)==sizeof(int).
        */
        fs_sorted_pointers[ i ].ptr = (long)fs_pointers_to_allocated[ i ];
        fs_sorted_pointers[ i ].index = i;
    }

#ifdef TEST
    if (kjb_debug_level > 5)
    {
        /* Too dangerous to use KJB IO at this point.  */
        fprintf(stderr, "Process %ld (fork depth %d) sorting %d pointers to optimize free (IL4RT).\n",
                (long)MY_PID, kjb_fork_depth, fs_num_pointers);
        fflush(stderr);  /* Stderr normally does not want a flush, but ... */
    }
#endif

    if (long_sort(fs_sorted_pointers,
                 fs_num_pointers,
                 sizeof(fs_sorted_pointers[ 0 ]),
                 (size_t)0,
                 USE_CURRENT_ATN_HANDLING)
        == ERROR)
    {
        return;
    }

    fs_fast_lookup_is_valid = TRUE;

#ifdef TEST
    if (kjb_debug_level > 5)
    {
        /* Too dangerous to use KJB IO at this point.  */
        fprintf(stderr, "Done sorting. Fast lookup for frees is now valid.\n\n");
        fflush(stderr);  /* Stderr normally does not want a flush, but ... */
    }
#endif

#else

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  optimize_free_2
 *
 * Speeds up frees while heap checking
 *
 * This is like optimize_free, but is used in conjunction with
 * start_heap_checking_2 and finish_heap_checking_2.
 *
 * Index: memory allocation, debugging
 *
 * -----------------------------------------------------------------------------
 */

int optimize_free_2(void)
{
#ifdef TRACK_MEMORY_ALLOCATION
    int i;
    int save_num_pointers = fs_num_pointers_2;
#ifdef TEST
    IMPORT int kjb_debug_level;
    IMPORT int kjb_fork_depth; 
#endif 


    if ( ! fs_heap_checking_enable_2) return NO_ERROR;

    /*
    // If fs_num_pointers_2 is greater than MAX_NUM_POINTERS_2 the
    // table is full and heap checking is disabled.
    */
    if (fs_num_pointers_2 > MAX_NUM_POINTERS_2)
    {
        return ERROR;
    }

    if (fs_fast_lookup_is_valid_2) return NO_ERROR;

#ifdef TEST
    if (kjb_debug_level > 5)
    {
        /* Too dangerous to use KJB IO at this point.  */
        fprintf(stderr,
                "<<TEST>> Process %ld (fork depth %d) compacting \"checked\" pointers 2 for optimization ... ",
                (long)MY_PID, kjb_fork_depth);
    }
#endif

    fs_num_pointers_2 = 0;

    for (i=0; i<save_num_pointers; i++)
    {
        if (fs_pointers_to_allocated_2[ i ] != NULL)
        {
            fs_pointers_to_allocated_2[ fs_num_pointers_2 ] = fs_pointers_to_allocated_2[ i ];
            fs_amount_allocated_2[ fs_num_pointers_2 ] = fs_amount_allocated_2[ i ];

            BUFF_CPY(fs_allocation_file_name_2[ fs_num_pointers_2 ],
                     fs_allocation_file_name_2[ i ]);

            fs_allocation_line_number_2[ fs_num_pointers_2 ] =
                                               fs_allocation_line_number_2[ i ];

            fs_allocation_pid_2[ fs_num_pointers_2 ] =
                                               fs_allocation_pid_2[ i ];

            fs_num_pointers_2++;
        }
    }

#ifdef TEST
    if (kjb_debug_level > 5)
    {
        fprintf(stderr, " done (IL4RT).\n\n");
    }
#endif

    for (i = 0; i < fs_num_pointers_2; i++)
    {
        /* CONSTCOND */
        if (sizeof(void*) != sizeof(long))
        {
            fprintf(stderr,
                    "\nsizeof(void*) != sizeof(long) in optimize_free. ");
            fprintf(stderr, "Likely some coding adjustments are in order.\n\n");
        }

        /* Clearly a bit dodgy! */
        fs_sorted_pointers_2[ i ].ptr = (long)fs_pointers_to_allocated_2[ i ];
        fs_sorted_pointers_2[ i ].index = i;
    }

    ERE(long_sort(fs_sorted_pointers_2,
                  fs_num_pointers_2,
                  sizeof(fs_sorted_pointers_2[ 0 ]),
                  (size_t)0,
                  USE_CURRENT_ATN_HANDLING));

    fs_fast_lookup_is_valid_2 = TRUE;

    return NO_ERROR;

#else

    return ERROR;

#endif

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

void check_for_heap_overruns(const char* file_name, int line_number)
{
    static int user_has_cancelled_abort = FALSE;
    int                  ptr_index;
    int                  i;
    int                  result    = NO_ERROR;
    const void*          ptr;
    const unsigned char* temp_ptr;


    /*
    // If fs_num_pointers is greater than MAX_NUM_POINTERS the
    // table is full and heap checking is disabled.
    */
    if (fs_heap_checking_enable && (fs_num_pointers <= MAX_NUM_POINTERS))
    {
        for (ptr_index = 0; ptr_index < fs_num_pointers; ptr_index++)
        {
            int pointer_overrun = FALSE;

            ptr = fs_pointers_to_allocated[ ptr_index ];

            if (ptr == NULL) continue;

            temp_ptr = (const unsigned char*)ptr + fs_amount_allocated[ ptr_index ];

            for (i=0; i<MALLOC_OVER_RUN_SIZE; i++)
            {
                if (*temp_ptr != 0xff)
                {
                    pointer_overrun = TRUE;
                    break;
                }
                temp_ptr++;
            }

            if (pointer_overrun)
            {
                if ( ! user_has_cancelled_abort)
                {
                    IMPORT volatile Bool halt_all_output;
                    IMPORT volatile Bool halt_term_output;

                    halt_all_output = FALSE;
                    halt_term_output = FALSE;

                    fprintf(stderr,
                            "Heap checker reporting on process %ld.\n",
                            (long)MY_PID);

                    fprintf(stderr,
                            "Found storage which was overrun.\n");
                    fprintf(stderr, "Allocated on line %d of file %s.\n",
                            fs_allocation_line_number[ ptr_index ],
                            fs_allocation_file_name[ ptr_index ]);
                    fprintf(stderr, "Allocation size is %lu\n",
                            (unsigned long)fs_amount_allocated[ ptr_index ]);
                    fprintf(stderr, "Pointer in question is %p.\n", ptr);
                    fprintf(stderr, "Location of over-run is %p.\n",
                                    (const void*)((const unsigned char*)ptr +
                                              fs_amount_allocated[ ptr_index ]));
                    fprintf(stderr, "Call to check_for_heap_overruns() from %s:%d.\n\n",
                            file_name, line_number);

                }

                if ((is_interactive()) && ( ! user_has_cancelled_abort))
                {
                    static int guard = FALSE;

                    /*
                       Since yes_no_query allocates storage, make sure we do not
                       loop.
                    */

                    if (guard)
                    {
                        p_stderr("Abort called (regardless of response) ");
                        p_stderr("due to mutex failure.\n");
                        kjb_abort();
                    }

                    guard = TRUE;

                    if (yes_no_query("Abort (y/n/q) ") )
                    {
                        kjb_abort();
                    }
                    else if ( ! (yes_no_query(
                                    "Prompt for abort on future violations (y/n/q) ")
                                )
                            )
                    {
                        user_has_cancelled_abort = TRUE;
                    }

                    guard = FALSE;
                }

                result = ERROR;
            }
        }
    }

    if ((result == ERROR) && ( ! user_has_cancelled_abort))
    {
        kjb_abort();
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  kjb_free
 *
 * KJB library replacement for free
 *
 * This routine frees memory allocated by kjb_malloc or kjb_calloc, and in
 * conjunction with those routines provides error checking in the case of
 * development code as described in kjb_malloc. One important additional
 * difference between kjb_free and "free", is that it is always safe to send
 * kjb_free a NULL pointer. (In fact, it should be safe to send all KJB library
 * destruction code a NULL pointer.) It is OK to send free a NULL pointer in
 * conformaing ANSI C, but many non-ANSI C environments break if you send free a
 * NULL. Using kjb_free is one way to be sure.
 *
 * Index: memory allocation, debugging
 *
 * -----------------------------------------------------------------------------
 */

void kjb_free(void* ptr)
{
#ifdef TRACK_MEMORY_ALLOCATION
    static int prompt_for_abort = TRUE;
    int        i;
#ifdef TEST
    IMPORT int kjb_debug_level;
    IMPORT int kjb_fork_depth; 
#endif 
#endif

    if (ptr == NULL) return;

#ifdef TRACK_MEMORY_ALLOCATION
    if (    (    (fs_heap_checking_enable)
              || ((fs_heap_checking_enable_2) && (! fs_heap_checking_skip_2))
            )
         && (ptr == fs_pointer_to_watch)
       )
    {
        if (prompt_for_abort)
        {
            IMPORT volatile Bool halt_all_output;
            IMPORT volatile Bool halt_term_output;

            halt_all_output = FALSE;
            halt_term_output = FALSE;

            fprintf(stderr,
                    "Freeing of pointer being watched for free.\n");
        }

        if ((is_interactive()) && (prompt_for_abort))
        {
            static int guard = FALSE;

            /*
               Since yes_no_query allocates storage, make sure we do not
               loop.
            */

            if (guard)
            {
                fprintf(stderr, "Abort called (regardless of response) ");
                fprintf(stderr, "due to mutex failure.\n");
                kjb_abort();
            }

            guard = TRUE;

            if (yes_no_query("Abort (y/n/q) ") )
            {
                kjb_abort();
            }
            else if ( ! (yes_no_query(
                            "Prompt for abort on future violations (y/n/q) ")
                        )
                    )
            {
                prompt_for_abort = FALSE;
            }

            guard = FALSE;
        }
    }

    /*
    // If fs_num_pointers is greater than MAX_NUM_POINTERS the
    // table is full and heap checking is disabled.
    */
    if (fs_heap_checking_enable && (fs_num_pointers <= MAX_NUM_POINTERS))
    {
        int pointer_overrun = FALSE;
        int ptr_index = lookup_pointer_index(ptr); 

        if (ptr_index != NOT_SET)
        {
            unsigned char* temp_ptr = (unsigned char*)ptr +
                                                  fs_amount_allocated[ ptr_index ];

            for (i=0; i<MALLOC_OVER_RUN_SIZE; i++)
            {
                if (*temp_ptr != 0xff)
                {
                    pointer_overrun = TRUE;
                    break;
                }
                temp_ptr++;
            }
        }

        if ((ptr_index == NOT_SET) || pointer_overrun)
        {
            if (prompt_for_abort)
            {
                IMPORT volatile Bool halt_all_output;
                IMPORT volatile Bool halt_term_output;

                halt_all_output = FALSE;
                halt_term_output = FALSE;

                fprintf(stderr,
                        "Heap checker reporting on process %ld.\n",
                        (long)MY_PID);

                if (ptr_index == NOT_SET)
                {
                    dbi(MY_PID); 
                    fprintf(stderr,
                            "Freeing a ptr not allocated / already been freed.\n");
                    sleep(10);
                }
                else
                {
                    fprintf(stderr,
                            "Freeing storage which was overrun.\n");
                    fprintf(stderr, "Allocated on line %d of file %s.\n",
                            fs_allocation_line_number[ ptr_index ],
                            fs_allocation_file_name[ ptr_index ]);
                    fprintf(stderr, "Allocation size is %lu\n",
                            (unsigned long)fs_amount_allocated[ ptr_index ]);
                    fprintf(stderr, "Pointer in question is %p.\n", ptr);
                    fprintf(stderr, "Location of over-run is %p.\n",
                                    (void*)((unsigned char*)ptr +
                                              fs_amount_allocated[ ptr_index ]));
                }
            }

            if ((is_interactive()) && (prompt_for_abort))
            {
                static int guard = FALSE;

                /*
                   Since yes_no_query allocates storage, make sure we do not
                   loop.
                */

                if (guard)
                {
                    p_stderr("Abort called (regardless of response) ");
                    p_stderr("due to mutex failure.\n");
                    kjb_abort();
                }

                guard = TRUE;

                kjb_abort();
               
                if ( ! (yes_no_query("Prompt for abort on future violations (y/n/q) ")))
                {
                    prompt_for_abort = FALSE;
                }

                guard = FALSE;
            }

            return;
        }
        else
        {
            fs_pointers_to_allocated[ ptr_index ] = NULL;

            if (! fs_fast_lookup_is_valid)
            {
                while (    (fs_num_pointers > 0)
                        && (fs_pointers_to_allocated[ fs_num_pointers - 1 ] == NULL)
                      )
                {
                    fs_num_pointers--;
                }
            }
        }
    }

    /*
    // If fs_num_pointers_2 is greater than MAX_NUM_POINTERS_2 the
    // table is full and heap checking is disabled.
    */
    if (    (fs_heap_checking_enable_2)
         && (fs_num_pointers_2 <= MAX_NUM_POINTERS_2)
         && ( ! fs_heap_checking_skip_2)
       )
    {
        int pointer_overrun_2 = FALSE;
        int ptr_index_2 = lookup_pointer_index_2(ptr); 


        if (ptr_index_2 != NOT_SET)
        {
            unsigned char* temp_ptr = (unsigned char*)ptr +
                                                  fs_amount_allocated_2[ ptr_index_2 ];

            for (i=0; i<MALLOC_OVER_RUN_SIZE; i++)
            {
                if (*temp_ptr != 0xff)
                {
                    pointer_overrun_2 = TRUE;
                    break;
                }
                temp_ptr++;
            }
        }

        if ((ptr_index_2 == NOT_SET) || pointer_overrun_2)
        {
            if (prompt_for_abort)
            {
                IMPORT volatile Bool halt_all_output;
                IMPORT volatile Bool halt_term_output;

                halt_all_output = FALSE;
                halt_term_output = FALSE;

                fprintf(stderr,
                        "Heap check 2 reporting on process %ld.\n",
                        (long)MY_PID);

                if (ptr_index_2 == NOT_SET)
                {
                    fprintf(stderr,
                            "Freeing a ptr 2 which was not allocated or has already been freed.\n");
                }
                else
                {
                    fprintf(stderr,
                            "Freeing storage 2 which was overrun.\n");
                    fprintf(stderr, "Allocated on line %d of file %s.\n",
                            fs_allocation_line_number_2[ ptr_index_2 ],
                            fs_allocation_file_name_2[ ptr_index_2 ]);
                    fprintf(stderr, "Allocation size is %lu\n",
                            (unsigned long)fs_amount_allocated_2[ ptr_index_2 ]);
                    fprintf(stderr, "Pointer in question is %p.\n", ptr);
                    fprintf(stderr, "Location of over-run is %p.\n",
                                    (void*)((unsigned char*)ptr +
                                              fs_amount_allocated_2[ ptr_index_2 ]));
                }
            }

            if ((is_interactive()) && (prompt_for_abort))
            {
                static int guard = FALSE;

                /*
                   Since yes_no_query allocates storage, make sure we do not
                   loop.
                */

                if (guard)
                {
                    fprintf(stderr, "Abort called (regardless of response) ");
                    fprintf(stderr, "due to mutex failure.\n");
                    kjb_abort();
                }

                guard = TRUE;

                if (yes_no_query("Abort (y/n/q) ") )
                {
                    kjb_abort();
                }
                else if ( ! (yes_no_query(
                                "Prompt for abort on future violations (y/n/q) ")
                            )
                        )
                {
                    prompt_for_abort = FALSE;
                }

                guard = FALSE;
            }

            return;
        }
        else
        {
            fs_pointers_to_allocated_2[ ptr_index_2 ] = NULL;

            if (! fs_fast_lookup_is_valid_2)
            {
                while (    (fs_num_pointers_2 > 0)
                        && (fs_pointers_to_allocated_2[ fs_num_pointers_2 - 1 ] == NULL)
                      )
                {
                    fs_num_pointers_2--;
                }
            }
        }
    }

#ifdef TEST
    if (kjb_debug_level > 10)
    {
        fprintf(stderr, "Process %ld (fork depth %d) is freeing %lx (IL4RT).\n",
                (long)MY_PID, kjb_fork_depth, (unsigned long)ptr);
    }
#endif 
#endif

#ifdef __STDC__
    free(ptr);
#else
    free((char*)ptr);
#endif

#if 0 /* was ifdef OBSOLETE */
#ifdef SGI
#ifndef lint
    free((char*)ptr);
#endif
#else
#ifdef SUN
#ifndef __lint
    free((char*)ptr);
#endif
#else
#ifdef VMS
    free(ptr);
#else
#ifdef MS_16_BIT_OS
    free((char*)ptr);
#else
    free(ptr);
#endif
#endif
#endif
#endif
#endif
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

void kjb_check_free(const void* ptr)
{
    int ptr_index;


    if (ptr == NULL) return;

    /*
    // If fs_num_pointers is greater than MAX_NUM_POINTERS the
    // table is full and heap checking is disabled.
    */
    if (fs_heap_checking_enable && (fs_num_pointers <= MAX_NUM_POINTERS))
    {
        ptr_index = lookup_pointer_index(ptr); 

        if (ptr_index == NOT_SET)
        {
            fprintf(stderr,
                    "Heap checker reporting on process %ld.\n",
                    (long)MY_PID);
            fprintf(stderr,
                    "Checking a ptr not allocated / already been freed.\n");

            kjb_abort();
        }
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static int lookup_pointer_index(const void* ptr)
{
    int i;
    int ptr_index = NOT_SET;


    if (    (fs_heap_checking_enable) 
         && (fs_num_pointers <= MAX_NUM_POINTERS)
       )
    {
        if (fs_fast_lookup_is_valid)
        {
            int ptr_sorted_index = long_binary_search(fs_sorted_pointers,
                                                      fs_num_pointers,
                                                      sizeof(fs_sorted_pointers[ 0 ]),
                                                      offsetof(Sorted_pointers, ptr),
                                                      (long)ptr);

            if (ptr_sorted_index != NOT_FOUND)
            {
                ptr_index = fs_sorted_pointers[ ptr_sorted_index ].index;

                if (fs_pointers_to_allocated[ ptr_index ] == NULL)
                {
                    ptr_index = NOT_SET;
                }
                else
                {
                    ASSERT(fs_pointers_to_allocated[ ptr_index ] == ptr);
                }
            }
        }
        else
        {
            /*
            // We are more likely to have allocated it recently, so search
            // backwards.
            */
            for (i = fs_num_pointers - 1; i >= 0; i--)
            {
                if (ptr == fs_pointers_to_allocated[ i ])
                {
                    ptr_index = i;
                    break;
                }
             }
        }
    }

    return ptr_index;
}

#endif 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static int lookup_pointer_index_2(const void* ptr)
{
    int i;
    int ptr_index = NOT_SET;


    if (    (fs_heap_checking_enable_2)
         && (fs_num_pointers_2 <= MAX_NUM_POINTERS_2)
         && ( ! fs_heap_checking_skip_2)
       )
    {
        if (fs_fast_lookup_is_valid_2)
        {
            int ptr_sorted_index = long_binary_search(fs_sorted_pointers_2,
                                                      fs_num_pointers_2,
                                                      sizeof(fs_sorted_pointers_2[ 0 ]),
                                                      offsetof(Sorted_pointers, ptr),
                                                      (long)ptr);

            if (ptr_sorted_index != NOT_FOUND)
            {
                ptr_index = fs_sorted_pointers_2[ ptr_sorted_index ].index;

                if (fs_pointers_to_allocated_2[ ptr_index ] == NULL)
                {
                    ptr_index = NOT_SET;
                }
                else
                {
                    ASSERT(fs_pointers_to_allocated_2[ ptr_index ] == ptr);
                }
            }
        }
        else
        {
            /*
            // We are more likely to have allocated it recently, so search
            // backwards.
            */
            for (i = fs_num_pointers_2 - 1; i >= 0; i--)
            {
                if (ptr == fs_pointers_to_allocated_2[ i ])
                {
                    ptr_index = i;
                    break;
                }
             }
        }
    }

    return ptr_index;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

