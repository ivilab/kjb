
/* $Id: l_sort.c 21923 2017-11-13 22:32:03Z kobus $ */

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

#include "l/l_string.h"
#include "l/l_sys_sig.h"
#include "l/l_sort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */

#ifdef PROGRAMMER_IS_kobus
#ifdef TEST
#undef KJB_HAVE_QSORT
#endif 
#endif 
 
#ifndef KJB_HAVE_QSORT
static int quick_sort
(
    char*  array,
    int    first_index,
    int    last_index,
    size_t element_size,
    void*  temp_record_ptr,
    int    (*cmp_fn) ( const void*, const void*)
);

static int partition
(
    char*  array,
    int    first_index,
    int    last_index,
    size_t element_size,
    void*  temp_record_ptr,
    int    (*cmp_fn) ( const void*, const void*)
);

static int find_pivot
(
    char*  array,
    int    first_index,
    int    last_index,
    size_t element_size,
    int    (*cmp_fn) ( const void*, const void*)
);
#endif 

static int int_quick_sort
(
    char*  array,
    int    first_index,
    int    last_index,
    size_t element_size,
    size_t key_pos,
    void*  temp_record_ptr
);

static int int_partition
(
    char*  array,
    int    first_index,
    int    last_index,
    size_t element_size,
    size_t key_pos,
    void*  temp_record_ptr
);

static int int_find_pivot
(
    char*  array,
    int    first_index,
    int    last_index,
    size_t element_size,
    size_t key_pos
);

static long long_quick_sort
(
    char*  array,
    long   first_index,
    long   last_index,
    size_t element_size,
    size_t key_pos,
    void*  temp_record_ptr 
);

static long long_partition 
(
    char*  array,
    long   left,
    long   right,
    size_t element_size,
    size_t key_pos,
    void*  temp_record_ptr 
);

static long long_find_pivot
(
    char*  array,
    long   left,
    long   right,
    size_t element_size,
    size_t key_pos 
);

static int find_insert_position
(
    void*  array,
    int    num_elements,
    size_t element_size,
    int    (*cmp_fn) (const void*, const void*),
    void*  key_ptr
);

/* ------------------------------------------------------------------------- */

static int fs_sort_rec_comparison_count = 0;

/* ------------------------------------------------------------------------- */

/*
 * =============================================================================
 *                                kjb_sort
 *
 * Sorts an array of arbitrary elements.
 *
 * On most systems this function simply wraps a call to qsort(3), based on
 * internal macro symbol KJB_HAVE_QSORT.  If that is not possible, then this
 * calls our own implementation of quicksort, which might have a bug (see the
 * note below).
 *
 * (If during testing we discover a system lacking an implementation of qsort,
 * then KJB_HAVE_QSORT should be defined as zero in file l_sys_def.h, in the
 * block corresponding to that operating system.  At the time of writing of this
 * comment, all systems we support were believed to have qsort.)
 *
 * Our own implementation is intended to be functionally compatible with qsort.
 * A few extra features are provided at the expense of speed. If the fastest
 * possible sort is required, qsort should be faster (since this is what writers
 * of this sort of routine tend to optimize.).
 *
 * The main functional difference between kjb_sort and qsort is the handling of
 * user interupt which is specified by the last parameter. If the last
 * parameter is USE_CURRENT_ATN_HANDLING, then there is no difference.
 * However a built in handler can be specified with USE_SORT_ATN_HANDLING. With
 * this handler, if the user interupts the sorting, they will be be prompted to
 * whether or not they want to continue. If they respond affirmitively, then
 * the sorting proceeds. Otherwise, the sorting is interrupted, and INTERRUPTED
 * is returned. A third choice for the last parameter is DISABLE_ATN_HANDLING.
 * With this choice, normal attention interupts are ignored during the sort.
 *
 * For sorting on integer keys, the routine int_sort should be faster, since
 * call backs are not required for comparison, although we have not put any
 * effort into making it fast.
 *
 * TODO:  If there is a system that truly requires our homebrew sort, then
 *        we should fix the apparent bug that is manifested when running
 *        demo program "break_sort.c" in directory lib/l/test/interactive.
 *        (We have made a small change, and it is not clear if the problem is
 *        still reproducible --- Currently on the mac it seems fine.)
 *
 * The arguments are somewhat self explanitory, but one source of bugs is
 * assuming that the comparison routine can return essentially a boolean answer
 * (i.e., is_greater), but this is not the case. The comparison routine MUST
 * return zero for equality, and postive, or negative as appropriate. The
 * details on when these valences are used is one difference between the home
 * brew version and qsort, and has lead to apparent bugs identified by different
 * results, that were due to an incorrect comparison routine. 
 *
 * Returns:
 *    This function returns ERROR or NO_ERROR depending upon success. 
 *    In addition, if USE_SORT_ATN_HANDLING is used, and the sort was
 *    interrupted, INTERRUPTED is returned.
 *
 *    The locally-implemented sort previously returned the number calls to the
 *    comparison required but this is too confusing since we usually just go
 *    with qsort these days. If needed, the number of comparisons is available
 *    using the function get_last_sort_comparison_count(). 
 *
 * Index: sorting/searching
 *
 * -----------------------------------------------------------------------------
 */

#ifdef KJB_HAVE_QSORT
/*ARGSUSED*/
int kjb_sort(void* array, int num_elements, size_t element_size,
             int (*cmp_fn) (const void *, const void *),
             int __attribute__((unused)) interrupt_action)
#else
int kjb_sort(void* array, int num_elements, size_t element_size,
             int (*cmp_fn) (const void *, const void *),
             int interrupt_action)
#endif
{
#ifdef KJB_HAVE_QSORT
    if ((element_size == 0) || (num_elements < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    qsort(array, num_elements, element_size, cmp_fn);
    return NO_ERROR;

#else /* KJB_HAVE_QSORT is false, so use a homebrewed sort. */
    IMPORT volatile Bool sort_atn_flag;
    void*               temp_record_ptr;
    int                 res;

#ifdef TEST 
    dbp("\n");
    dbp("****************************************************");
    dbp("Using home brew, possibly buggy quick_sort"); 
#endif 

    if (num_elements < 2)
    {
        return NO_ERROR;
    }

    if (    (interrupt_action != USE_SORT_ATN_HANDLING)
         && (interrupt_action != DISABLE_ATN_HANDLING)
         && (interrupt_action != USE_CURRENT_ATN_HANDLING)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if ((element_size == 0) || (num_elements < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    temp_record_ptr = (void*)KJB_MALLOC(element_size);

    if (interrupt_action == USE_SORT_ATN_HANDLING)
    {
        ERE(set_atn_trap(sort_atn_fn, RESTART_AFTER_SIGNAL));
    }
    else if (interrupt_action == DISABLE_ATN_HANDLING)
    {
        ERE(set_atn_trap(SIG_IGN, RESTART_AFTER_SIGNAL));
    }

    sort_atn_flag = FALSE;
    fs_sort_rec_comparison_count = 0;

    res = quick_sort((char*)array, (int)0, num_elements - 1,
                     element_size, temp_record_ptr, cmp_fn);

    kjb_free( temp_record_ptr );

#ifdef TEST
    if (! check_sort(array, num_elements, element_size, cmp_fn))
    {
        set_bug("Check on sort failed.");
        return ERROR;
    }
    else 
    {
        dbp("Sort is OK.");
    }
#endif

    if (interrupt_action != USE_CURRENT_ATN_HANDLING)
    {
        ERE(unset_atn_trap());
    }

    return res;
#endif /* KJB_HAVE_QSORT */
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifndef KJB_HAVE_QSORT

/* =============================================================================
 * STATIC                      quick_sort
 * -----------------------------------------------------------------------------
*/

static int quick_sort(char* array, int first_index, int last_index,
                      size_t element_size, void* temp_record_ptr,
                      int (*cmp_fn) (const void *, const void *))
{
    int partition_index;
    int   res;


    if (last_index > first_index)
    {
        partition_index = partition(array, first_index, last_index,
                                    element_size, temp_record_ptr, cmp_fn) ;

        if (partition_index < 0) return partition_index;

        res = quick_sort(array, first_index, partition_index-1,
                         element_size, temp_record_ptr, cmp_fn);

        if (res != NO_ERROR) return res;

        res = quick_sort(array, partition_index+1, last_index,
                        element_size, temp_record_ptr, cmp_fn);

        return res;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      partition_index
 * -----------------------------------------------------------------------------
*/

static int partition(char* array, int left, int right, size_t element_size,
                     void* temp_record_ptr,
                     int (*cmp_fn) (const void *, const void *))
{
    int pivot_index;
    int cmp_res;


    pivot_index = find_pivot(array, left, right, element_size, cmp_fn);

    if (pivot_index < 0) return pivot_index;

    kjb_memcpy(temp_record_ptr, (void*)(array + pivot_index*element_size),
               element_size);

    /* 
     * Changed after Andrew noticed that the call to memcpy sometimes had
     * overlapping source and destination ranges. 
     *
     * I am not sure if it is a problem that pivot_index could be the same as
     * left comming in, but now we at least do not try to overwrite a block with
     * itself. 
    */
    if (pivot_index != left) 
    {
        kjb_memcpy((void*)(array + pivot_index*element_size),
                   (void*)(array + left*element_size),
                   element_size);
    }

    while (left < right)
    {

        cmp_res = (*cmp_fn)(temp_record_ptr,
                            (void*)(array + right*element_size));
        fs_sort_rec_comparison_count++;

        while ((left < right ) && (cmp_res < 0))
        {
            --right;

            cmp_res = (*cmp_fn)(temp_record_ptr,
                                (void*)(array + right*element_size));
            fs_sort_rec_comparison_count++;
        }

        if (left < right )
        {
            kjb_memcpy((void*)(array + left*element_size),
                       (void*)(array + right*element_size),
                       element_size);

            ++left;
        }

        cmp_res = (*cmp_fn)(temp_record_ptr,
                            (void*)(array + left*element_size));
        fs_sort_rec_comparison_count++;

        while (( left < right ) && (cmp_res > 0))
        {
            ++left;

            cmp_res = (*cmp_fn)(temp_record_ptr,
                                (void*)(array + left*element_size));

            fs_sort_rec_comparison_count++;
        }

        if (left < right)
        {
             kjb_memcpy((void*)(array + right*element_size),
                        (void*)(array + left*element_size),
                        element_size);
            --right;
        }
    }

    kjb_memcpy((void*)(array + left*element_size), temp_record_ptr,
               element_size);

    return left;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      find_pivot
 * -----------------------------------------------------------------------------
*/

static int find_pivot(char* array, int left, int right, size_t element_size,
                      int (*cmp_fn) (const void *, const void *))
{
    IMPORT volatile Bool sort_atn_flag;
    int                 middle;
    int                 cmp_res;


    if (sort_atn_flag)
    {
        return INTERRUPTED;
    }

    middle = (left + right) / 2;

    cmp_res = (*cmp_fn)((void*)(array + left*element_size),
                        (void*)(array + middle*element_size));

    fs_sort_rec_comparison_count++;

    if (cmp_res > 0)
    {
        cmp_res = (*cmp_fn)((void*)(array + right*element_size),
                            (void*)(array + middle*element_size));
        fs_sort_rec_comparison_count++;

        if (cmp_res > 0)
        {
            cmp_res = (*cmp_fn)((void*)(array + right*element_size),
                                (void*)(array + left*element_size));
            fs_sort_rec_comparison_count++;

            if (cmp_res > 0)
            {
                return left;
            }
            else
            {
                return right;
            }
        }
        else
        {
            return middle;
        }
    }
    else  /*   left gives key less than middle  */
    {
        cmp_res = (*cmp_fn)((void*)(array + right*element_size),
                            (void*)(array + middle*element_size));
        fs_sort_rec_comparison_count++;

        if (cmp_res > 0)
        {
            return middle;
        }
        else
        {
            cmp_res = (*cmp_fn)((void*)(array + right*element_size),
                                (void*)(array + left*element_size));
            fs_sort_rec_comparison_count++;

            if (cmp_res > 0)
            {
                return right;
            }
            else
            {
                return left;
            }
        }
    }

    /*NOTREACHED*/
#ifdef __GNUC__
    /* Some compilers think we can get here! Keep them happy. */
    return ERROR;
#endif
}

#endif   /* Case that we do not have qsort(). */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                check_sort
 *
 * Tests whether an array is sorted
 *
 * This funtion tests whether an array is sorted. The main use is to test
 * sorting code. However, one can imagine other uses. Hences we expose this
 * function.
 *
 * Returns:
 *    TRUE (1) if the array is sorted, FALSE (0) otherwise. 
 *
 * Index: sorting/searching
 *
 * -----------------------------------------------------------------------------
 */

int check_sort(void* array, int num_elements, size_t element_size,
             int (*cmp_fn) (const void *, const void *))
{
    int i;
    int cmp_res;

    for (i=1; i<num_elements; i++)
    {
         cmp_res = cmp_fn((char*)array + (i - 1) * element_size,
                          (char*)array +  i      * element_size);

         if (cmp_res > 0)
         {
             return FALSE;
         }
    }

    return TRUE;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                int_sort
 *
 * Sorts an array of arbitrary elements by integer key.
 *
 * This routine is much like kjb_sort except the call backs are avoided on the
 * assumption that the key is an integer at offset key_pos (see offsetof()).
 * This routine is barely legal ANSI-C and might fail on some bizzare system
 * (but not anything common). The operation in question is whether:
 * |
 * |       (int*)((void*)((char*)array + element_size*n + key_pos))
 * |
 * evaluates to a valid pointer to the n'th integer key, where key_pos is
 * offsetof() the integer key, and element_size is sizeof(<element-type>). (I
 * don't know of any systems where this is not the case). This routine is only
 * meant to provide a faster sort in this specialized case, so if you don't
 * want to take a chance with the above then kjb_sort or qsort can be used.
 *
 * The interupt handling is the same as kjb_sort (see kjb_sort(3)).
 *
 * Returns:
 *    This function returns ERROR or NO_ERROR depending upon success. 
 *    In addition, if USE_SORT_ATN_HANDLING is used, and the sort was
 *    interrupted, INTERRUPTED is returned.
 *
 *    This function previously returned the number calls to the
 *    comparison required but this is probably too confusing now that we have
 *    changed kjb_sort() to not do this. If needed, the number of comparisons
 *    is available using the function get_last_sort_comparison_count(). 
 *
 * Index: sorting/searching
 *
 * -----------------------------------------------------------------------------
 */

int int_sort(void* array, int num_elements, size_t element_size,
             size_t key_pos, int interrupt_action)
{
    IMPORT volatile Bool sort_atn_flag;
    void*               temp_record_ptr;
    int                 res;


    if (    (interrupt_action != USE_SORT_ATN_HANDLING)
         && (interrupt_action != DISABLE_ATN_HANDLING)
         && (interrupt_action != USE_CURRENT_ATN_HANDLING)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (num_elements < 2)
    {
        return NO_ERROR;
    }

    if ((element_size <= 0) || (num_elements < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    NRE(temp_record_ptr = (void*)KJB_MALLOC(element_size));

    if (interrupt_action == USE_SORT_ATN_HANDLING)
    {
        ERE(set_atn_trap(sort_atn_fn, RESTART_AFTER_SIGNAL));
    }
    else if (interrupt_action == DISABLE_ATN_HANDLING)
    {
        ERE(set_atn_trap(SIG_IGN, RESTART_AFTER_SIGNAL));
    }

    sort_atn_flag = FALSE;
    fs_sort_rec_comparison_count = 0;

    res = int_quick_sort((char*)array, (int)0, (int)num_elements - 1,
                          element_size, key_pos, temp_record_ptr);

    kjb_free( temp_record_ptr );

    if (interrupt_action != USE_CURRENT_ATN_HANDLING)
    {
        ERE(unset_atn_trap());
    }

    return res;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      int_quick_sort
 * -----------------------------------------------------------------------------
*/

static int int_quick_sort(char* array, int first_index, int last_index,
                          size_t element_size, size_t key_pos,
                          void* temp_record_ptr)
{
    int partition_index;
    int res;


    if (last_index > first_index)
    {
        partition_index = int_partition(array, first_index, last_index,
                                        element_size, key_pos, temp_record_ptr);

        if (partition_index < 0) return partition_index;

        res = int_quick_sort(array, first_index, partition_index-1,
                             element_size, key_pos, temp_record_ptr);

        if (res != NO_ERROR) return res;

        res = int_quick_sort(array, partition_index+1, last_index,
                             element_size, key_pos, temp_record_ptr);

        return res;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      int_partition
 * -----------------------------------------------------------------------------
*/

static int int_partition(char* array, int left, int right, size_t element_size,
                         size_t key_pos, void* temp_record_ptr)
{
    IMPORT volatile Bool sort_atn_flag;
    int                 pivot_index;
    int                 pivot_val;
    int                 test_val;
    char*               char_ptr;

    pivot_index = int_find_pivot(array, left, right, element_size, key_pos);

    if (pivot_index < 0) return pivot_index;

    char_ptr = (char*)array + pivot_index*element_size + key_pos;
    pivot_val = *((int*)((void*)char_ptr));

    kjb_memcpy(temp_record_ptr, (void*)(array + pivot_index*element_size),
               element_size);

    /* 
     * Changed after Andrew noticed that memcpy could be overwritten. I am not
     * sure if it is a problem that pivot_index could be the same as left
     * comming in. 
    */
    if (pivot_index != left) 
    {
        kjb_memcpy((void*)(array + pivot_index*element_size),
                   (void*)(array + left*element_size),
                   element_size);
    }

    if (sort_atn_flag) return INTERRUPTED;

    while ( left < right )
    {
        char_ptr = (char*)array + right*element_size + key_pos;
        test_val = *((int*)((void*)char_ptr));
        fs_sort_rec_comparison_count++;

        while ((left < right ) && (pivot_val < test_val))
        {
             --right;

            char_ptr = (char*)array + right*element_size + key_pos;
            test_val = *((int*)((void*)char_ptr));
            fs_sort_rec_comparison_count++;
        }

        if (sort_atn_flag) return INTERRUPTED;

        if (left < right )
        {
            fs_sort_rec_comparison_count++;

            kjb_memcpy((void*)(array + left*element_size),
                       (void*)((char*)array + right*element_size),
                       element_size);
            ++left;
        }

        if (sort_atn_flag) return INTERRUPTED;

        char_ptr = (char*)array + left*element_size + key_pos;
        test_val = *((int*)((void*)char_ptr));
        fs_sort_rec_comparison_count++;

        while ((left < right ) && (pivot_val > test_val))
        {
            ++left;
            fs_sort_rec_comparison_count += 2;

            char_ptr = (char*)array + left*element_size + key_pos;
            test_val = *((int*)((void*)char_ptr));
            fs_sort_rec_comparison_count++;
        }

        if (sort_atn_flag) return INTERRUPTED;

        if (left < right)
        {
            kjb_memcpy((void*)(array + right*element_size),
                       (void*)(array + left*element_size),
                       element_size);
           --right;
        }

        if (sort_atn_flag) return INTERRUPTED;
    }

    kjb_memcpy((void*)(array + left*element_size), temp_record_ptr,
               element_size);

    return left;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      int_find_pivot
 * -----------------------------------------------------------------------------
*/

static int int_find_pivot(char* array, int left, int right, size_t element_size,
                          size_t key_pos)
{
    IMPORT volatile Bool sort_atn_flag;
    int                 middle;
    int                 right_int;
    int                 middle_int;
    int                 left_int;


    if (sort_atn_flag) return INTERRUPTED;

    middle = (left + right) / 2;

    fs_sort_rec_comparison_count++;

    left_int  = *((int*)((void*)((char*)array + left*element_size + key_pos)));
    right_int = *((int*)((void*)((char*)array + right*element_size + key_pos)));
    middle_int= *((int*)((void*)((char*)array + middle*element_size +key_pos)));

    if (left_int > middle_int)
    {
         fs_sort_rec_comparison_count++;

         if (right_int > middle_int)
         {
              fs_sort_rec_comparison_count++;

              if (right_int > left_int)
              {
                  return left;
              }
              else return right;
          }
          else return middle;
      }
      else  /*   left gives key less than middle  */
      {
          fs_sort_rec_comparison_count++;

          if (right_int > middle_int)
          {
              return middle;
          }
          else
          {
              fs_sort_rec_comparison_count++;

              if (right_int > left_int)
              {
                   return right;
               }
               else return left;
           }
      }

    /* Some compilers think we can get here! Keep them happy. */

#ifdef NeXT
#ifdef __GNUC__
    return ERROR;
#endif
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                long_sort
 *
 * Sorts an array of arbitrary elements by long integer key.
 *
 * This routine is much like kjb_sort except the call backs are avoided on the
 * assumption that the key is a long integer at offset key_pos (see offsetof()).
 *
 * The interupt handling is the same as kjb_sort (see kjb_sort(3)).
 *
 * Returns:
 *    On success int_sort returns the number of key comparisons required. If
 *    there was an error, then ERROR is returned. On failure ERROR is returned
 *    and an error message is set. In addition, if USE_SORT_ATN_HANDLING is
 *    used, and the sort was interrupted, INTERRUPTED is returned.
 *
 * Index: sorting/searching
 *
 * -----------------------------------------------------------------------------
 */

long long_sort(void* array, long num_elements, size_t element_size,
               size_t key_pos, int interrupt_action)
{
    IMPORT volatile Bool sort_atn_flag;
    void*               temp_record_ptr;
    long                res;


    if (    (interrupt_action != USE_SORT_ATN_HANDLING)
         && (interrupt_action != DISABLE_ATN_HANDLING)
         && (interrupt_action != USE_CURRENT_ATN_HANDLING)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (num_elements < 2)  
    {
        return NO_ERROR;
    }

    if ((element_size <= 0) || (num_elements < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    NRE(temp_record_ptr = (void*)KJB_MALLOC(element_size));

    if (interrupt_action == USE_SORT_ATN_HANDLING)
    {
        ERE(set_atn_trap(sort_atn_fn, RESTART_AFTER_SIGNAL));
    }
    else if (interrupt_action == DISABLE_ATN_HANDLING)
    {
        ERE(set_atn_trap(SIG_IGN, RESTART_AFTER_SIGNAL));
    }

    sort_atn_flag = FALSE;
    fs_sort_rec_comparison_count = 0;

    res = long_quick_sort((char*)array, (long)0, (long)num_elements - 1,
                          element_size, key_pos, temp_record_ptr);

    kjb_free( temp_record_ptr );

    if (interrupt_action != USE_CURRENT_ATN_HANDLING)
    {
        ERE(unset_atn_trap());
    }

    if (res == NO_ERROR)
    {
        return fs_sort_rec_comparison_count;
    }
    else
    {
        return res;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      long_quick_sort
 * -----------------------------------------------------------------------------
*/

static long long_quick_sort(char* array, long first_index, long last_index,
                            size_t element_size, size_t key_pos,
                            void* temp_record_ptr)
{
    long partition_index;
    long res;


    if (last_index > first_index)
    {
        partition_index = long_partition(array, first_index, last_index,
                                        element_size, key_pos, temp_record_ptr);

        if (partition_index < 0) return partition_index;

        res = long_quick_sort(array, first_index, partition_index-1,
                             element_size, key_pos, temp_record_ptr);

        if (res != NO_ERROR) return res;

        res = long_quick_sort(array, partition_index+1, last_index,
                             element_size, key_pos, temp_record_ptr);

        return res;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      long_partition
 * -----------------------------------------------------------------------------
*/

static long long_partition(char* array, long left, long right, size_t element_size,
                           size_t key_pos, void* temp_record_ptr)
{
    IMPORT volatile Bool sort_atn_flag;
    long                pivot_index;
    long                pivot_val;
    long                test_val;
    char*               char_ptr;


    pivot_index = long_find_pivot(array, left, right, element_size, key_pos);

    if (pivot_index < 0) return pivot_index;

    char_ptr = (char*)array + pivot_index*element_size + key_pos;
    pivot_val = *((long*)((void*)char_ptr));

    kjb_memcpy(temp_record_ptr, (void*)(array + pivot_index*element_size),
               element_size);

    if(pivot_index != left)
    {
        kjb_memcpy((void*)(array + pivot_index*element_size),
                   (void*)(array + left*element_size),
                   element_size);
    }


    if (sort_atn_flag) return INTERRUPTED;

    while ( left < right )
    {
        char_ptr = (char*)array + right*element_size + key_pos;
        test_val = *((long*)((void*)char_ptr));
        fs_sort_rec_comparison_count++;

        while ((left < right ) && (pivot_val < test_val))
        {
             --right;

            char_ptr = (char*)array + right*element_size + key_pos;
            test_val = *((long*)((void*)char_ptr));
            fs_sort_rec_comparison_count++;
        }

        if (sort_atn_flag) return INTERRUPTED;

        if (left < right )
        {
            fs_sort_rec_comparison_count++;

            kjb_memcpy((void*)(array + left*element_size),
                       (void*)((char*)array + right*element_size),
                       element_size);
            ++left;
        }

        if (sort_atn_flag) return INTERRUPTED;

        char_ptr = (char*)array + left*element_size + key_pos;
        test_val = *((long*)((void*)char_ptr));
        fs_sort_rec_comparison_count++;

        while ((left < right ) && (pivot_val > test_val))
        {
            ++left;
            fs_sort_rec_comparison_count += 2;

            char_ptr = (char*)array + left*element_size + key_pos;
            test_val = *((long*)((void*)char_ptr));
            fs_sort_rec_comparison_count++;
        }

        if (sort_atn_flag) return INTERRUPTED;

        if (left < right)
        {
            kjb_memcpy((void*)(array + right*element_size),
                       (void*)(array + left*element_size),
                       element_size);
           --right;
        }

        if (sort_atn_flag) return INTERRUPTED;
    }

    kjb_memcpy((void*)(array + left*element_size), temp_record_ptr,
               element_size);

    return left;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      long_find_pivot
 * -----------------------------------------------------------------------------
*/

static long long_find_pivot(char* array, long left, long right, size_t element_size,
                          size_t key_pos)
{
    IMPORT volatile Bool sort_atn_flag;
    long                middle;
    long                right_long;
    long                middle_long;
    long                left_long;


    if (sort_atn_flag) return INTERRUPTED;

    middle = (left + right) / 2;

    fs_sort_rec_comparison_count++;

    left_long  = *((long*)((void*)((char*)array + left*element_size + key_pos)));
    right_long = *((long*)((void*)((char*)array + right*element_size + key_pos)));
    middle_long= *((long*)((void*)((char*)array + middle*element_size +key_pos)));

    if (left_long > middle_long)
    {
         fs_sort_rec_comparison_count++;

         if (right_long > middle_long)
         {
              fs_sort_rec_comparison_count++;

              if (right_long > left_long)
              {
                  return left;
              }
              else return right;
          }
          else return middle;
      }
      else  /*   left gives key less than middle  */
      {
          fs_sort_rec_comparison_count++;

          if (right_long > middle_long)
          {
              return middle;
          }
          else
          {
              fs_sort_rec_comparison_count++;

              if (right_long > left_long)
              {
                   return right;
               }
               else return left;
           }
      }

    /* Some compilers think we can get here! Keep them happy. */

#ifdef NeXT
#ifdef __GNUC__
    return ERROR;
#endif
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  binary_search
 *
 * Finds an element in a sorted array
 *
 * This routine finds an element (pointed to by search_key_ptr) in the sorted
 * array "array" by binary search. The search is done with the compare routine
 * "cmp_fn".
 *
 * For searching on integer keys, the routine int_binary_search should be
 * faster, since call backs are not required for comparison (the trade of is
 * slightly less standardized clean and possible portable code).
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * Index: sorting/searching
 *
 * -----------------------------------------------------------------------------
 */

int binary_search(const void* array, int num_elements, size_t element_size,
                  int (*cmp_fn) (const void *, const void *),
                  const void* search_key_ptr)
{
    int   first_elem;
    int   last_elem;
    int   cur_elem;
    int   compare_result;
    const char* cur_elem_ptr;
    int   loop_count;


    /*
     * Changes to this routine likely should be considered for
     * int_binary_search().
    */

    if (num_elements == 0) return NOT_FOUND;

    loop_count = 0;
    first_elem = 0;
    last_elem = num_elements - 1;

    while ( loop_count < MAX_BIN_SEARCH_LOOP_COUNT )
    {
        cur_elem = (first_elem + last_elem)/2;
        cur_elem_ptr =  (const char*)array + cur_elem*element_size;

        compare_result = (*cmp_fn)(search_key_ptr, (const void*)cur_elem_ptr);

        if (compare_result == 0)
        {
            return cur_elem;
        }
        else if (compare_result < 0)
        {
            last_elem = cur_elem - 1;

            if (first_elem > last_elem)
            {
                return NOT_FOUND;
            }
        }
        else
        {
            first_elem = cur_elem + 1;

            if (first_elem > last_elem)
            {
                return NOT_FOUND;
            }
        }
        loop_count++;
    }
    return NOT_FOUND;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  int_binary_search
 *
 * Finds an element in a sorted array
 *
 * This routine finds an element with integer key search_key in the sorted array
 * "array" by binary search in the special case where the key is an integer.
 *
 * This routine avoids the call backs on the assumption that the key is an
 * integer at offset key_pos (see offsetof()).  This routine is barely legal
 * ANSI-C and might fail on some bizzare system (but not anything common). The
 * operation in question is whether:
 * |
 * |       (int*)((void*)((char*)array + element_size*n + key_pos))
 * |
 * evaluates to a valid pointer to the n'th integer key, where key_pos is
 * offsetof() the integer key, and element_size is sizeof(<element-type>). (I
 * don't know of any systems where this is not the case). This routine is only
 * meant to provide a faster search in this specialized case, so if you don't
 * want to take a chance with the above then binary_search can be used.
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * Index: sorting/searching
 *
 * -----------------------------------------------------------------------------
 */

int int_binary_search(const void* array, int num_elements, size_t element_size,
                      size_t element_offset, int search_key)
{
    int   first_elem;
    int   last_elem;
    int   cur_elem;
    int   cur_elem_key;
    int   loop_count;


    /*
     * Changes to this routine likely should be considered for
     * binary_search_int_array().
    */

    if (num_elements == 0) return NOT_FOUND;

    loop_count = 0;
    first_elem = 0;
    last_elem = num_elements - 1;

    while ( loop_count < MAX_BIN_SEARCH_LOOP_COUNT )
    {
        cur_elem = (first_elem + last_elem)/2;
        cur_elem_key = *((const int*)((const void*)((const char*)array + cur_elem*element_size + element_offset)));

        if (cur_elem_key == search_key)
        {
            return cur_elem;
        }
        else if (cur_elem_key > search_key)
        {
            last_elem = cur_elem - 1;

            if (first_elem > last_elem)
            {
                return NOT_FOUND;
            }
        }
        else
        {
            first_elem = cur_elem + 1;

            if (first_elem > last_elem)
            {
                return NOT_FOUND;
            }
        }
        loop_count++;
    }
    return NOT_FOUND;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  long_binary_search
 *
 * Finds an element in a sorted array
 *
 * This routine finds an element with long integer key search_key in the sorted
 * array "array" by binary search in the special case where the key is an
 * integer.
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * Index: sorting/searching
 *
 * -----------------------------------------------------------------------------
 */

long long_binary_search(const void* array, long num_elements, size_t element_size,
                        size_t element_offset, long search_key)
{
    long   first_elem;
    long   last_elem;
    long   cur_elem;
    long   cur_elem_key;
    long   loop_count;


    /*
     * Changes to this routine likely should be considered for
     * binary_search_long_array().
    */

    if (num_elements == 0) return NOT_FOUND;

    loop_count = 0;
    first_elem = 0;
    last_elem = num_elements - 1;

    while ( loop_count < MAX_BIN_SEARCH_LOOP_COUNT )
    {
        cur_elem = (first_elem + last_elem)/2;
        cur_elem_key = *((const long*)((const void*)((const char*)array + cur_elem*element_size + element_offset)));

        if (cur_elem_key == search_key)
        {
            return cur_elem;
        }
        else if (cur_elem_key > search_key)
        {
            last_elem = cur_elem - 1;

            if (first_elem > last_elem)
            {
                return NOT_FOUND;
            }
        }
        else
        {
            first_elem = cur_elem + 1;

            if (first_elem > last_elem)
            {
                return NOT_FOUND;
            }
        }
        loop_count++;
    }
    return NOT_FOUND;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                             binary_search_int_array
 *
 * Finds an element in a sorted array of integers
 *
 * This routine finds an element with integer key search_key in the sorted array
 * "array" of integers by binary search.
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * Index: sorting/searching
 *
 * -----------------------------------------------------------------------------
 */

int binary_search_int_array(const int* array, int num_elements, int search_key)
{
    int   first_elem;
    int   last_elem;
    int   cur_elem;
    int   cur_elem_key;
    int   loop_count;

    /*
     * Almost exaclty the same code as int_binary_search(), but copied so that
     * it can be a bit faster and more compact.
    */

    if (num_elements == 0) return NOT_FOUND;

    loop_count = 0;
    first_elem = 0;
    last_elem = num_elements - 1;

    while ( loop_count < MAX_BIN_SEARCH_LOOP_COUNT )
    {
        cur_elem = (first_elem + last_elem)/2;
        cur_elem_key = *(array + cur_elem);

        if (cur_elem_key == search_key)
        {
            return cur_elem;
        }
        else if (cur_elem_key > search_key)
        {
            last_elem = cur_elem - 1;

            if (first_elem > last_elem)
            {
                return NOT_FOUND;
            }
        }
        else
        {
            first_elem = cur_elem + 1;

            if (first_elem > last_elem)
            {
                return NOT_FOUND;
            }
        }
        loop_count++;
    }
    return NOT_FOUND;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                       binary_search_long_array
 *
 * Finds an element in a sorted array of long integers
 *
 * This routine finds an element with long integer key search_key in the sorted
 * array "array" of integers by binary search.
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * Index: sorting/searching
 *
 * -----------------------------------------------------------------------------
 */

long binary_search_long_array(const long* array, long num_elements, long search_key)
{
    long   first_elem;
    long   last_elem;
    long   cur_elem;
    long   cur_elem_key;
    long   loop_count;

    /*
     * Almost exaclty the same code as long_binary_search(), but copied so that
     * it can be a bit faster and more compact.
    */

    UNTESTED_CODE(); 

    if (num_elements == 0) return NOT_FOUND;

    loop_count = 0;
    first_elem = 0;
    last_elem = num_elements - 1;

    while ( loop_count < MAX_BIN_SEARCH_LOOP_COUNT )
    {
        cur_elem = (first_elem + last_elem)/2;
        cur_elem_key = *(array + cur_elem);

        if (cur_elem_key == search_key)
        {
            return cur_elem;
        }
        else if (cur_elem_key > search_key)
        {
            last_elem = cur_elem - 1;

            if (first_elem > last_elem)
            {
                return NOT_FOUND;
            }
        }
        else
        {
            first_elem = cur_elem + 1;

            if (first_elem > last_elem)
            {
                return NOT_FOUND;
            }
        }
        loop_count++;
    }
    return NOT_FOUND;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  linear_search
 *
 * Finds an element in an array
 *
 * This routine finds an element (pointed to by search_key_ptr) in the array
 * "array" with a linear search. The array need not be sorted. The search is
 * done with the compare routine "cmp_fn".
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * -----------------------------------------------------------------------------
 */

int linear_search(const void* array, int num_elements, size_t element_size,
                  int (*cmp_fn) (const void *, const void *),
                  const void* search_key_ptr)
{
    int   compare_result;
    const char* cur_elem_ptr;
    int   i;


    cur_elem_ptr = (const char*)array;

    for (i=0; i<num_elements; i++)
    {
        compare_result=(*cmp_fn)(search_key_ptr, cur_elem_ptr);

        if (compare_result == 0)
        {
            return i;
        }

        cur_elem_ptr += element_size;
    }

    return NOT_FOUND;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  int_linear_search
 *
 * Finds an element in an array with integer key
 *
 * This routine finds an element in the array "array" with a linear search
 * assuming that the key is an integer. The array need not be sorted.
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * -----------------------------------------------------------------------------
 */

int int_linear_search(const void* array, int num_elements, size_t element_size,
                      size_t element_offset, int search_key)
{
    const char* cur_elem_ptr;
    int   cur_elem_key;
    int   i;


    /*
     * Untested, since moving the previous int_linear_search to
     * linear_search_int_array(), and creating this function in parallel with
     * int_binary_search().
    */

    cur_elem_ptr = (const char*)array;

    for (i=0; i<num_elements; i++)
    {
        cur_elem_key = *((const int*)((const void*)(cur_elem_ptr + element_offset)));

        if (cur_elem_key == search_key)
        {
            return i;
        }

        cur_elem_ptr += element_size;
    }

    return NOT_FOUND;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                                  long_linear_search
 *
 * Finds an element in an array with long integer key
 *
 * This routine finds an element in the array "array" with a linear search
 * assuming that the key is a longinteger. The array need not be sorted.
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * -----------------------------------------------------------------------------
 */

long long_linear_search(const void* array, long num_elements, size_t element_size,
                      size_t element_offset, long search_key)
{
    const char* cur_elem_ptr;
    long   cur_elem_key;
    long   i;


    UNTESTED_CODE(); 

    cur_elem_ptr = (const char*)array;

    for (i=0; i<num_elements; i++)
    {
        cur_elem_key = *((const long*)((const void*)(cur_elem_ptr + element_offset)));

        if (cur_elem_key == search_key)
        {
            return i;
        }

        cur_elem_ptr += element_size;
    }

    return NOT_FOUND;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                               linear_search_int_array
 *
 * Finds an integer in an array of integers
 *
 * This routine finds an integer in the array "array" with a linear search. The
 * array need not be sorted.
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * -----------------------------------------------------------------------------
 */

int linear_search_int_array(const int* array, int num_elements, int search_elem)
{
    int i;


    for (i=0; i<num_elements; i++)
    {
        if (*array == search_elem)
        {
            return i;
        }

        array++;
    }

    return NOT_FOUND;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * =============================================================================
 *                               linear_search_long_array
 *
 * Finds an integer in an array of long integers
 *
 * This routine finds a long integer in the array "array" with a linear search.
 * The array need not be sorted.
 *
 * Returns:
 *    On success, this routine returns an index into the array. If the element
 *    is not found, then it returns NOT_FOUND.
 *
 * -----------------------------------------------------------------------------
 */

long linear_search_long_array(const long* array, long num_elements, long search_elem)
{
    long i;


    UNTESTED_CODE(); 

    for (i=0; i<num_elements; i++)
    {
        if (*array == search_elem)
        {
            return i;
        }

        array++;
    }

    return NOT_FOUND;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int insert_into_sorted_array(void* array,
                             int* num_elements_ptr,
                             size_t element_size,
                             int (*cmp_fn) (const void *, const void *),
                             void* new_element_ptr)
{
    int   num_elements;
    int   i;
    char* target_pos;
    char* source_pos;
    int   count;


    num_elements = *num_elements_ptr;

    ERE(i = find_insert_position(array, num_elements, element_size,
                                 cmp_fn, new_element_ptr));

    ASSERT(i >= 0);

    count = (num_elements - i) * element_size;
    source_pos = (char*)array + num_elements*element_size - 1;
    target_pos = (char*)array + num_elements*element_size+ element_size - 1;

    while (count > 0)
    {
        *target_pos-- = *source_pos--;
        count--;
    }

    kjb_memcpy((void*)((char*)array + i*element_size), new_element_ptr,
                element_size);

    (*num_elements_ptr)++;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                      find_insert_position
 * -----------------------------------------------------------------------------
*/

static int find_insert_position(void* array, int num_elements,
                                size_t element_size,
                                int (*cmp_fn) (const void *, const void *),
                                void* search_key_ptr)
{
    int   first_elem;
    int   last_elem;
    int   cur_elem;
    int   compare_result;
    char* cur_elem_ptr;
    int   loop_count;


    if (num_elements == 0) return 0;

    loop_count = 0;
    first_elem = 0;
    last_elem = num_elements - 1;


    while (loop_count < MAX_BIN_SEARCH_LOOP_COUNT )
    {
        cur_elem = (first_elem + last_elem)/2;

        cur_elem_ptr = (char*)array + cur_elem*element_size;

        compare_result = (*cmp_fn)(search_key_ptr, (void*)cur_elem_ptr);

        if (compare_result == 0)
        {
            return cur_elem;
        }
        else if (compare_result < 0)
        {
            /* Test element is less than cur_elem. */
            last_elem = cur_elem - 1;

            if (first_elem > last_elem)
            {
                return first_elem;
            }
        }
        else
        {
            /* Test element is greater than cur_elem. */
            first_elem = cur_elem + 1;

            if (first_elem > last_elem)
            {
                return first_elem;
            }
        }
        loop_count++;
    }
 
    set_bug("Max count (%d) exceeded in find_insert_position.",
            MAX_BIN_SEARCH_LOOP_COUNT);

    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_last_sort_comparison_count(void)
{
#ifdef KJB_HAVE_QSORT
    set_error("Sort comparison count not available when using qsort for sorting.");
    return ERROR; 
#else
    return fs_sort_rec_comparison_count;
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

