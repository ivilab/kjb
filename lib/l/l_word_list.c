
/* $Id: l_word_list.c 21282 2017-03-05 07:58:08Z kobus $ */

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
#include "l/l_parse.h"
#include "l/l_int_vector.h"
#include "l/l_io.h"
#include "l/l_string.h"
#include "l/l_verbose.h"
#include "l/l_sort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              get_target_word_list
 *
 * Allocates a Word_list of the specified size
 *
 * The routine allocates a Word_list of the specified size, with the word
 * (string) pointers initialized to NULL. The standard KJB library allocation
 * semantics are followed. However, we do not do any clever storage recycling.
 * If *target_word_list_ptr_ptr is not null, we just free it and start again. 
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int get_target_word_list(Word_list** target_word_list_ptr_ptr, int num_words)
{
    Word_list* target_word_list_ptr = *target_word_list_ptr_ptr;
    char**     target_words;
    int        i;


    if (num_words < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    free_word_list(*target_word_list_ptr_ptr);

    NRE(*target_word_list_ptr_ptr = TYPE_MALLOC(Word_list));
    target_word_list_ptr = *target_word_list_ptr_ptr;

    /* Start at zero for safe free in case of failure. */
    target_word_list_ptr->num_words = 0;
    target_word_list_ptr->words = NULL;

    if (num_words > 0)
    {
        NRE(target_word_list_ptr->words = N_TYPE_MALLOC(char*, num_words));
        target_word_list_ptr->num_words = num_words;
    }

    target_words = target_word_list_ptr->words;

    for (i = 0; i < num_words; i++)
    {
        target_words[ i ] = NULL;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              ra_get_target_word_list
 *
 * Reallocates a Word_list of the specified size, like realloc().
 *
 * The routine is similar to get_target_word_list() except that if the first
 * parameter ponts to an existing word list, that part of the existing storage
 * that fits into the new size is preserved.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int ra_get_target_word_list
(
    Word_list** target_word_list_ptr_ptr,
    int         num_words
)
{
    Word_list* target_word_list_ptr = *target_word_list_ptr_ptr;


    if (num_words < 0)
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }
    else if (    (target_word_list_ptr == NULL)
              || (target_word_list_ptr->num_words == 0)
              || (num_words == 0)
            )
    {
        /* UNTESTED_CODE(); */
        return get_target_word_list(target_word_list_ptr_ptr, num_words);
    }
    else if (target_word_list_ptr->num_words == num_words)
    {
        UNTESTED_CODE();
        return NO_ERROR;
    }
    else
    {
        int cur_num_words = target_word_list_ptr->num_words;
        char** target_words = target_word_list_ptr->words;
        int i;

        for (i = num_words; i < cur_num_words; ++i)
        {
            kjb_free(target_words[ i ]);
        }

        NRE(target_words = N_TYPE_REALLOC(target_words, char*, num_words));

        for (i = cur_num_words; i < num_words; ++i)
        {
            target_words[ i ] = NULL;
        }

        target_word_list_ptr->words = target_words;
        target_word_list_ptr->num_words = num_words;

        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              free_word_list
 *
 * Frees a word list
 *
 * The routine frees the storage attached to a word list. As in all KJB library
 * free routines, it is safe to pass a NULL pointer.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/
void free_word_list(Word_list* word_list_ptr)
{
    int num_words, i;
    char** words;

    if (word_list_ptr == NULL) return;

    num_words = word_list_ptr->num_words;
    words = word_list_ptr->words;

    for (i = 0; i < num_words; i++)
    {
        kjb_free(words[ i ]);
    }

    kjb_free(words);
    kjb_free(word_list_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              append_word_list
 *
 * Adds a word to the end of a word list
 *
 * The routine adds a word to the end of a word list, as defined by the first
 * NULL pointer encountered. If the word list pointed to by the first argument
 * has no NULLs, then its size is increased without changing its contents, at
 * which point the append will succeed. The word list pointed to by the first
 * argument can be NULL.  The word pointer must not equal NULL.
 *
 * Note that if num_words increases, it might increase by more than one; the
 * list can have an unspecified number of empty entries added to the end.
 * This will make future calls to append_word_list() faster (compared to the
 * time required to realloc() and to copy the entries).
 * At exit, the num_words field stores the total number of entries (empty and
 * nonempty).  If you do not want empty entries at the tail end of the list,
 * consider also calling trim_word_list_empty_entries_at_tail().
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int append_word_list(Word_list** word_list_ptr_ptr, const char* word)
{
    const Word_list* word_list_ptr;
    int i, num_words, new_numwords;
    char** words;

    NRE( word_list_ptr_ptr );
    NRE( word );

    word_list_ptr = *word_list_ptr_ptr;
    num_words = (word_list_ptr == NULL) ? 0 : word_list_ptr->num_words;
    new_numwords = num_words + 10 + num_words / 5;
    words = (word_list_ptr == NULL) ? NULL : word_list_ptr->words;

    for (i = 0; i < num_words; i++)
    {
        if (words[ i ] == NULL)
        {
            return copy_word(&(words[ i ]), word);
        }
    }

    /*
     * Grow by at least 10 and up to 20%.
    */
    ERE(ra_get_target_word_list(word_list_ptr_ptr, new_numwords));

    return copy_word(&((*word_list_ptr_ptr)->words[ num_words]), word);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              select_from_word_list
 *
 * Copies selected words (strings) from a word list
 *
 * The routine copies selected words (strings) from a source word list to a
 * target word list which is created as needed to be the size needed for the
 * selected words (strings). The selection is specified using the integer vector
 * enable_vp. If enable_vp is NULL, this routine is the same as copy_word_list.
 * If enable_vp is not NULL, it must be the same length as the source word list,
 * and it specifies needed words (strings) by non zero elements.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int select_from_word_list
(
    Word_list**       target_word_list_ptr_ptr,
    const Word_list*  source_word_list_ptr,
    const Int_vector* enable_vp
)
{

    UNTESTED_CODE();

    if (enable_vp == NULL)
    {
        return copy_word_list(target_word_list_ptr_ptr, source_word_list_ptr);
    }

    if (source_word_list_ptr == NULL)
    {
        free_word_list(*target_word_list_ptr_ptr);
        *target_word_list_ptr_ptr = NULL;
        return NO_ERROR;
    }
    else
    {
        char** source_words     = source_word_list_ptr->words;
        int    num_source_words = source_word_list_ptr->num_words;
        int    i;
        char** target_words;
        int    num_target_words = 0;

        if (enable_vp->length != num_source_words)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }

        for (i = 0; i < num_source_words; i++)
        {
            if (enable_vp->elements[ i ])
            {
                num_target_words++;
            }
        }

        ERE(get_target_word_list(target_word_list_ptr_ptr, num_target_words));
        target_words = (*target_word_list_ptr_ptr)->words;

        num_target_words = 0;

        for (i = 0; i < num_source_words; i++)
        {
            if (enable_vp->elements[ i ])
            {
#if 0 /* was ifdef HOW_IT_WAS_07_11_10 */
                NRE(target_words[ num_target_words ] = kjb_strdup(source_words[ i ]));
#else
                UNTESTED_CODE();
                ERE(copy_word(&(target_words[ num_target_words ]), source_words[ i ]));
#endif
                num_target_words++;
            }
        }

        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              copy_word_list
 *
 * Copies a word list
 *
 * The routine copies a source word list to a target word list which is created
 * as needed to be the same length as the source word list.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int copy_word_list(Word_list**      target_word_list_ptr_ptr,
                   const Word_list* word_list_ptr)
{
    NRE( target_word_list_ptr_ptr );

    if (word_list_ptr == NULL)
    {
        free_word_list(*target_word_list_ptr_ptr);
        *target_word_list_ptr_ptr = NULL;
        return NO_ERROR;
    }
    else
    {
        char** words        = word_list_ptr->words;
        int    num_words    = word_list_ptr->num_words;
        int    i;
        char** target_words;

        ERE(get_target_word_list(target_word_list_ptr_ptr, num_words));
        target_words = (*target_word_list_ptr_ptr)->words;

        for (i = 0; i < num_words; i++)
        {
            /*
             * 'words' might contain NULL entries, which is perfectly normal,
             * but copy_word() regards that as an error.
             */
            if (words[ i ] != NULL)
            {
                ERE(copy_word(&(target_words[ i ]), words[ i ]));
            }
        }

        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef WAIT_UNTIL_WE_CONSIDER_CHANGING_WORD_TO_STRING
/* =============================================================================
 *                              copy_word
 *
 * Copies a word
 *
 * The routine copies a source word to a target word which is created as needed
 * to be the same length as the source word.  The pointer to the source word
 * must not equal NULL.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/
#endif
/* consider making this static */
int copy_word(char** target_word_ptr_ptr, const char* source_word_ptr)
{
    NRE( target_word_ptr_ptr );
    NRE( source_word_ptr );

    if (*target_word_ptr_ptr == NULL)
    {
        NRE(*target_word_ptr_ptr = kjb_strdup(source_word_ptr));
    }
    else if (strlen(*target_word_ptr_ptr) >= strlen(source_word_ptr))
    {
        strcpy(*target_word_ptr_ptr, source_word_ptr);
    }
    else
    {
        kjb_free(*target_word_ptr_ptr);
        NRE(*target_word_ptr_ptr = kjb_strdup(source_word_ptr));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              sort_word_list
 *
 * Sorts a word list
 *
 * The routine puts a sorted version of a source word list to a target word list
 * which is created as needed to be the same length as the source word list.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int sort_word_list(Word_list**      target_word_list_ptr_ptr,
                   const Word_list* word_list_ptr)
{

    ERE(copy_word_list(target_word_list_ptr_ptr, word_list_ptr));

    ERE(kjb_sort((*target_word_list_ptr_ptr)->words,
                 (*target_word_list_ptr_ptr)->num_words,
                 sizeof(char*),
                 ptr_strcmp,
                 USE_CURRENT_ATN_HANDLING));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              search_word_list
 *
 * Searches a word list for a given string
 *
 * The routine searches a word list for a given string. If it is not there, it
 * returns NOT_FOUND. Otherwise, it returns the index of where it was found.
 *
 * Returns:
 *    NOT_FOUND if the string is not in the word list (including the case where
 *    the word list in NULL, or the index where it is found.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int search_word_list(const Word_list* word_list_ptr, const char* search_str)
{

    if (word_list_ptr == NULL)
    {
        return NOT_FOUND;
    }
    else
    {
        char** words        = word_list_ptr->words;
        int    num_words    = word_list_ptr->num_words;
        int    i;

        for (i = 0; i < num_words; i++)
        {
            if (STRCMP_EQ(words[ i ], search_str))
            {
                return i;
            }
        }

        return NOT_FOUND;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              search_sorted_word_list
 *
 * Searches a sorted word list for a given string
 *
 * The routine searches a sorted word list for a given string. If it is not
 * there, it returns NOT_FOUND. Otherwise, it returns the index of where it was
 * found.
 *
 * Warning:
 *    If the list is NOT sorted, this routine will not work.
 *
 * Returns:
 *    NOT_FOUND if the string is not in the word list (including the case where
 *    the word list in NULL, or the index where it is found.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int search_sorted_word_list(const Word_list* word_list_ptr, const char* search_str)
{

    if (word_list_ptr == NULL)
    {
        return NOT_FOUND;
    }
    else
    {
        return binary_search(word_list_ptr->words,
                             word_list_ptr->num_words,
                             sizeof(char*),
                             ptr_strcmp,
                             (const void*)search_str);

    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              concat_word_lists
 *
 * Concatenates an array of word lists
 *
 * The routine concatenates an array of word lists, into a target word list
 * which is created or resized as needed to hold the result.
 *
 * Note that if the input lists contain empty entries, they are preserved in the
 * order they appear! Also note that function append_word_list() freely adds
 * empty entries at the end of lists.  If that is undesirable, use function
 * trim_word_list_empty_entries_at_tail() on the input lists, prior to calling
 * this function.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/
int concat_word_lists(Word_list** target_word_list_ptr_ptr, int num_word_lists,
                      const Word_list* const* word_list_ptr_array)
{
    int    word_list;
    int    num_total_words = 0;
    char** target_words;

    for (word_list = 0; word_list < num_word_lists; word_list++)
    {
        const Word_list* word_list_ptr = word_list_ptr_array[ word_list ];

        if (word_list_ptr != NULL)
        {
            num_total_words += (word_list_ptr_array[ word_list ])->num_words;
        }
    }

    if (num_total_words == 0)
    {
        free_word_list(*target_word_list_ptr_ptr);
        *target_word_list_ptr_ptr = NULL;
        return NO_ERROR;
    }

    ERE(get_target_word_list(target_word_list_ptr_ptr, num_total_words));

    num_total_words = 0;

    target_words = (*target_word_list_ptr_ptr)->words;

    for (word_list = 0; word_list < num_word_lists; word_list++)
    {
        const Word_list* word_list_ptr = word_list_ptr_array[ word_list ];

        if (word_list_ptr != NULL)
        {
            char** words        = word_list_ptr->words;
            int    num_words    = word_list_ptr->num_words;
            int    i;

            for (i = 0; i < num_words; i++)
            {
                if ( words[ i ] != NULL )
                {
                    ERE(copy_word(&(target_words[num_total_words]), words[i]));
                }
                num_total_words++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              read_word_list
 *
 * Reads a list of words from a file
 *
 * The routine reads a list of words (strings), assumed to be lines of the file
 * whose name is specified as the second argument.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int read_word_list(Word_list** word_list_ptr_ptr, const char* file_name)
{
    FILE*  fp;
    int    result = NO_ERROR;


    if ((file_name == NULL) || (*file_name == '\0'))
    {
        fp = stdin;
    }
    else
    {
        NRE(fp = kjb_fopen(file_name, "r"));
    }

    result = fp_read_word_list(word_list_ptr_ptr, fp);

    if ((file_name != NULL) && (*file_name != '\0'))
    {
        (void)kjb_fclose(fp);  /* Ignore return--only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              fp_read_word_list
 *
 * Reads a list of words from a file
 *
 * The routine reads a list of words (strings), assumed to be lines of the file
 * pointed to by the second argument.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int fp_read_word_list(Word_list** word_list_ptr_ptr, FILE* fp)
{
    int    count;
    int    result = NO_ERROR;
    char   buff[ 1000 ];
    Word_list* word_list_ptr = *word_list_ptr_ptr;
    int    cur_num_words = (word_list_ptr == NULL) ? 0 : word_list_ptr->num_words;
    char** words = (word_list_ptr == NULL) ? NULL : word_list_ptr->words;
    int    realloc_flag = FALSE;


    count = 0;

    /*CONSTCOND*/
    while (TRUE)
    {
        if (result == ERROR) break;


        if (cur_num_words <= count)
        {
            result = ra_get_target_word_list(word_list_ptr_ptr, 2 * (count + 1));
            if (result == ERROR) break;

            words = (*word_list_ptr_ptr)->words;
            cur_num_words = (*word_list_ptr_ptr)->num_words;

            realloc_flag = TRUE;
        }

        result = BUFF_GET_REAL_LINE(fp, buff);
        if (result == ERROR) break;

        if (result == EOF)
        {
            break;
        }

#if 0 /* was ifdef HOW_IT_WAS_07_11_10 */
        if ((words[ count ] = kjb_strdup(buff)) == NULL)
        {
            result = ERROR;
            break;
        }
#else
        UNTESTED_CODE();
        result = copy_word(&(words[ count ]), buff);
        if (result == ERROR) break;
#endif

        count++;
    }

    if (result == ERROR)
    {
        free_word_list(*word_list_ptr_ptr);
        *word_list_ptr_ptr = NULL;
    }
    else if (realloc_flag)
    {
        /*
         * If we have been responsible for bumping up the storage, then we trim
         * it to the exact size. Otherwise, we assume that the caller does not
         * want us to touch the size.
        */
        result = ra_get_target_word_list(word_list_ptr_ptr, count);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *                               sget_word_list
 *
 * Reads a list of words from a string.
 *
 * This routine reads a list of words from a string into an word list. The
 * argument delimiters is a string containing possible delimiters. If it is
 * NULL, then the list is assumed to be space or tab delimited. Currently
 * delimiters are ignored inside quoted entities. This means that quotes are not
 * valid delimters.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR if there are problems.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int sget_word_list(Word_list** word_list_ptr_ptr,
                   const char* line,
                   const char* delimiters)
{
    const char* line_pos = line;
    const char* save_line_pos = line_pos;
    char** words;
    int    count;
    int    num_words = 0;
    int    result = NO_ERROR;
    char   buff[ 1000 ];


    if (delimiters == NULL)
    {
        delimiters = " \t";
    }


    while (BUFF_CONST_GEN_MQ_GET_TOKEN(&line_pos, buff, delimiters))
    {
        num_words++;
    }

    line_pos = save_line_pos;

    ERE(get_target_word_list(word_list_ptr_ptr, num_words));
    words = (*word_list_ptr_ptr)->words;

    count = 0;

    while (BUFF_CONST_GEN_MQ_GET_TOKEN(&line_pos, buff, delimiters))
    {
#if 0 /* was ifdef HOW_IT_WAS_07_11_10 */
        if ((words[ count ] = kjb_strdup(buff)) == NULL)
        {
            result = ERROR;
            break;
        }
#else
        UNTESTED_CODE();
        result = copy_word(&(words[ count ]), buff);
        if (result == ERROR) break;
#endif
        count++;
    }

    if (result == ERROR)
    {
        free_word_list(*word_list_ptr_ptr);
        *word_list_ptr_ptr = NULL;
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              write_word_list
 *
 * Writes a list of words to a file
 *
 * The routine writes a list of words, one per row, to a file whose name is
 * specified as the second argument.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int write_word_list(const Word_list* word_list_ptr, const char* file_name)
{
    FILE*        fp;
    int          result;
    Error_action save_error_action = get_error_action();
    int          close_result;


    if (file_name != NULL)
    {
        if (skip_because_no_overwrite(file_name))
        {
            return NO_ERROR;
        }

        NRE(fp = kjb_fopen(file_name, "w"));
    }
    else
    {
        fp = stdout;
    }

    result = fp_write_word_list(word_list_ptr, fp);

    if (file_name != NULL)
    {
        if (result == ERROR)
        {
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        }
        close_result = kjb_fclose(fp);

        set_error_action(save_error_action);

        if (close_result == ERROR) result = ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              fp_write_word_list
 *
 * Writes a list of words to a file
 *
 * The routine writes a list of words, one per row, to a file whose file pointer
 * is specified as the second argument.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/

int fp_write_word_list(const Word_list* word_list_ptr, FILE* fp)
{
    int    num_words = (word_list_ptr == NULL) ? 0 : word_list_ptr->num_words;
    char** words     = (word_list_ptr == NULL) ? NULL : word_list_ptr->words;
    int    count;

    for (count = 0; count < num_words; count++)
    {
        if (words[ count ] != NULL)
        {
            ERE(fput_line(fp, words[ count ]));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              count_strings_in_word_list
 *
 * A Word_list can contain both strings and empty entries.  The number of
 * strings plus the number of empty entries is stored in the field
 * word_list_ptr->num_words.  This function counts only the number of strings,
 * i.e., the non-empty entries.  (Note also than an empty entry differs from an
 * empty string!  An empty entry corresponds to a char* equal to NULL somewhere
 * in the array, whereas an empty string corresponds to a char* containing the
 * address of a byte storing a '\0' character.  We count the latter but not the
 * former.)
 *
 * Returns:
 *    The number of strings in the list, or zero if the pointer equals NULL.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/
int count_strings_in_word_list( const Word_list* word_list_ptr )
{
    int i, count = 0;

    if ( NULL == word_list_ptr )
    {
        return 0;
    }

    for( i = 0; i < word_list_ptr -> num_words; ++i )
    {
        if ( word_list_ptr -> words[ i ] != NULL ) ++count;
    }
    return count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              trim_word_list_empty_entries_at_tail
 *
 * Removes empty entries from the tail end (only) of a word list.
 *
 * If you have called function append_word_list on Word_list L, then you may
 * have increased the number of list
 * entries in L to more than the number of strings stored in the list.
 * If you then use list concatenation on L and M, the empty entries at the end
 * of L will be copied before the M is copied, i.e., the resulting list will
 * have a hole in the middle.  That could be undesirable.
 * This function will remedy that problem by removing all empty entries from
 * the end of the list.  For example:
 * | Word_list* lp = 0;
 * | ERE(get_target_word_list(&lp, 0));             # zero empty entries
 * | ERE(append_word_list(&lp, "alpha"));           # unknown num empty entries
 * | # If you concatenate lp and another list now, it could have a hole in it.
 * | ERE(trim_word_list_empty_entries_at_tail(lp)); # zero empty entries
 * | # Now it is safe to concatenate using lp.
 *
 * This function does not deallocate anything; the object must still
 * be freed, as usual, after this function, even if num_words goes to zero.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure, with an appropriate error
 *    message being set.
 *
 * Index: strings, word lists
 *
 * -----------------------------------------------------------------------------
*/
int trim_word_list_empty_entries_at_tail( Word_list* word_list_ptr )
{
    char** last_entry;

    NRE( word_list_ptr );

    last_entry = word_list_ptr -> words + word_list_ptr -> num_words;

    while( 0 < word_list_ptr -> num_words && NULL == *--last_entry )
    {
        word_list_ptr -> num_words -= 1;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

