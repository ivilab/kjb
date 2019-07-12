
/* $Id: l_string.h 21654 2017-08-05 14:10:14Z kobus $ */

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

#ifndef L_STRING_INCLUDED
#define L_STRING_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifndef __C2MAN__

#ifdef PROGRAMMER_IS_kobus
#ifdef TEST
#define EMULATE_MEMCPY 
#endif 
#endif 

#ifndef EMULATE_MEMCPY
#    define kjb_memcpy(x, y, z)  (void*)memcpy((void*)(x), \
                                              (const void*)(y), \
                                              (size_t)(z))
#endif 

#endif

#define WHITE_SPACE_STR        " \t"
#define IS_WHITE_SPACE(x)      (((x) == ' ') || ((x) == '\t' ))
#define NOT_WHITE_SPACE(x)     (((x) != ' ') && ((x) != '\t' ))
#define IS_DIGIT(x)            (((x) >= '0') && ((x) <= '9' ))


#define BUFF_CPY(x,y)              kjb_buff_cpy(x, y, (sizeof(x)))
#define BUFF_CAT(x,y)              kjb_buff_cat(x, y, (sizeof(x)))
#define BUFF_TRUNC_CPY(x,y)        str_trunc_cpy(x, y, sizeof(x))
#define BUFF_TRUNC_CAT(x,y)        str_trunc_cat(x, y, sizeof(x))
#define BUFF_TRUNC_QUOTE_CPY(x,y)  trunc_quote_cpy(x, y, sizeof(x))

#define EXTENDED_LC_BUFF_CPY(x,y)  extended_lc_strncpy(x, y, (sizeof(x)))
#define EXTENDED_UC_BUFF_CPY(x,y)  extended_uc_strncpy(x, y, (sizeof(x)))
#define FIELD_CPY(x,y)             (void*)kjb_memcpy((void*)(x),(const void*)&(y),(sizeof(y)))


#define BUFF_REVERSE(x, y)                kjb_reverse(x, y, sizeof(y))
#define BUFF_CAP_FIRST_LETTER_CPY(x, y)   cap_first_letter_cpy(x, y, sizeof(x))


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             BUFF_SGET_LINE
 *
 * Sets up call to sget_line (MACRO)
 *
 * The max_len parameter is set to sizeof(line).  Using sizeof to set the
 * buffer size is recomended where applicable, as the code will not be broken
 * if the buffer size changes. HOWEVER, neither this method, nor the macro, is
 * applicable if line is NOT a character array.  If line is declared by "char
 * *line", then the size of line is the number of bytes in a character pointer
 * (usually 4), which is NOT what is normally intended.  You have been WARNED!
 *
 * Note that sget_line() and const_sget_line() do the same thing. Neither change
 * the actual string being parsed, only the position pointer. There are two
 * versions to better communicate with various compilers.
 *
 * Related:
 *    sget_line, CONST_BUFF_SGET_LINE, const_sget_line, BUFF_DGET_LINE, dget_line,
 *    BUFF_FGET_LINE, fget_line
 *
 * Index: strings, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    int BUFF_SGET_LINE(char** input_line, char buff[ ]);
#endif

#ifndef __C2MAN__  /* Just doing "else" confuses ctags for the first such line. 
                      I have no idea what the problem is. */
#    define BUFF_SGET_LINE(x, y)        sget_line(x, y, sizeof(y))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             CONST_BUFF_SGET_LINE
 *
 * Sets up call to const_sget_line (MACRO)
 *
 * The max_len parameter is set to sizeof(line).  Using sizeof to set the
 * buffer size is recomended where applicable, as the code will not be broken
 * if the buffer size changes. HOWEVER, neither this method, nor the macro, is
 * applicable if line is NOT a character array.  If line is declared by "char
 * *line", then the size of line is the number of bytes in a character pointer
 * (usually 4), which is NOT what is normally intended.  You have been WARNED!
 *
 * Note that const_sget_line() and sget_line() do the same thing. Neither change
 * the actual string being parsed, only the position pointer. There are two
 * versions to better communicate with various compilers.
 *
 * Related:
 *    const_sget_line, BUFF_SGET_LINE, sget_line, BUFF_DGET_LINE, dget_line,
 *    BUFF_FGET_LINE, fget_line
 *
 * Index: strings, Macros
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__    /* Pretend we are a ".c" file to document MACROS. */
    int BUFF_CONST_SGET_LINE(const char** input_line, char buff[ ]);
#else
#    define BUFF_CONST_SGET_LINE(x, y)  const_sget_line(x, y, sizeof(y))
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define EQUAL_STRINGS            0
#define FIRST_STRING_GREATER     1
#define SECOND_STRING_GREATER    (-1)
#define EQUAL_HEADS              0
#define UNEQUAL_STRINGS          (-1)

#define NOT_A_SUBSTRING          0
#define CHARACTER_NOT_IN_STRING  0

#define HEAD_CMP_EQ(x, y)     (head_cmp((x),(y)) == EQUAL_HEADS)
#define MEMCMP_EQ(x, y, z)    (kjb_memcmp((x),(y),(size_t)(z)) == EQUAL_STRINGS)
#define STRCMP_EQ(x, y)       (kjb_strcmp((x),(y)) == EQUAL_STRINGS)
#define STRNCMP_EQ(x, y, z)   (kjb_strncmp((x),(y),(size_t)(z))==EQUAL_STRINGS)
#define IC_HEAD_CMP_EQ(x, y)  (ic_head_cmp((x),(y)) == EQUAL_HEADS)
#define IC_STRCMP_EQ(x, y)    (kjb_ic_strcmp((x),(y)) == EQUAL_STRINGS)
#define FIND_CHAR_YES(x, y)   (find_char((x),(y)) != CHARACTER_NOT_IN_STRING)
#define FIND_CHAR_NO(x, y)    (find_char((x),(y)) == CHARACTER_NOT_IN_STRING)
#define FIND_STRING_YES(x, y) (find_string((x),(y)) != NOT_A_SUBSTRING)
#define FIND_STRING_NO(x, y)  (find_string((x),(y)) == NOT_A_SUBSTRING)


#ifdef TRACK_MEMORY_ALLOCATION
#    define kjb_strdup(x)         debug_kjb_str_dup(x, __FILE__, __LINE__)
#endif


int    signed_strlen   (const char* str);
size_t const_trim_beg  (const char** string_arg_ptr);
size_t trim_beg        (char** string_arg_ptr);
size_t trim_end        (char* string_arg);
size_t trim_len        (const char* str);
size_t gen_trim_beg  
(
    char**      string_arg_ptr, /* Address of string pointer. */
    const char* terminators 
);

size_t const_gen_trim_beg
(
    const char** string_arg_ptr, /* Address of string pointer. */
    const char*  terminators 
);

size_t gen_trim_end  (char* string_arg, const char* terminators);
void   extended_uc_lc  (char* string_arg);
void   extended_lc_uc  (char* string_arg);
void   extended_n_uc_lc(char* in_string, size_t len);
void   extended_n_lc_uc(char* in_string, size_t len);
int    extended_tolower(int c);
int    extended_toupper(int c);
int    kjb_strcmp      (const char* s1, const char* s2);
int    void_strcmp     (const void* s1, const void* s2);
int    kjb_memcmp      (const char* s1, const char* s2, size_t len);
int    kjb_strncmp     (const char* s1, const char* s2, size_t max_len);
int    kjb_ic_strcmp   (const char* s1, const char* s2);
int    kjb_ic_strncmp  (const char* s1, const char* s2, size_t max_len);
int    head_cmp        (const char* input, const char* test_head);
int    ic_head_cmp     (const char* input, const char* test_head);

#ifdef METHOD_ONE
    int ptr_strcmp(const char** str_ptr1, const char** str_ptr2);

    int ptr_strncmp
    (
        const char** str_ptr1,
        const char** str_ptr2,
        size_t       len
    );

    int ptr_ic_strcmp
    (
        const char** str_ptr1,
        const char** str_ptr2
    );

    int ptr_head_cmp(const char** s1_ptr, const char** s2_ptr);
    int ptr_ic_head_cmp(const char** s1_ptr, const char** s2_ptr);
#else
    int ptr_strcmp(const void* str_ptr1, const void* str_ptr2);

    int ptr_strncmp
    (
        const void* str_ptr1,
        const void* str_ptr2,
        size_t      len
    );

    int ptr_ic_strcmp
    (
        const void* str_ptr1,
        const void* str_ptr2
    );

    int ptr_head_cmp(const void* s1_ptr, const void* s2_ptr);
    int ptr_ic_head_cmp(const void* s1_ptr, const void* s2_ptr);
#endif

void rpad(char* string, size_t cur_len, size_t total_len);

void rpad_cpy
(
    char*       target_string,
    const char* source_string,
    size_t      pad_len
);


#ifdef TRACK_MEMORY_ALLOCATION
    char* debug_kjb_str_dup
    (
        const char* input_string,
        const char* file_name,
        int         line_number
    );
#else
    char* kjb_strdup(const char* input_string);
#endif


void str_trunc_cpy
(
    char*       output_string,
    const char* input_string,
    size_t      max_len
);

void str_trunc_cat
(
    char*       output_string,
    const char* input_string,
    size_t      max_len 
);

void trunc_quote_cpy
(
    char*       output_string,
    const char* input_string,
    size_t      max_len
);

void kjb_buff_cat(char* s1, const char* s2, size_t max_len);
void kjb_strncat (char* s1, const char* s2, size_t max_len);
int  kjb_buff_cpy(char* s1, const char* s2, size_t max_len);
void kjb_strncpy (char* s1, const char* s2, size_t max_len);

#ifndef kjb_memcpy
void *kjb_memcpy(void* dest, const void* src, size_t max_len);
#endif

void   extended_lc_strncpy (char* s1, const char* s2, size_t max_len);
void   extended_uc_strncpy (char* s1, const char* s2, size_t max_len);
void   cap_first_letter_cpy(char* s1, const char* s2, size_t max_len);
void   str_build           (char** s1_ptr, const char* s2);

void   str_n_build
(
    char**      s1_ptr,
    const char* s2,
    size_t      max_len
);

void   str_char_build      (char** s1_ptr, int char_arg, size_t num);

void   byte_build
(
    char**         s1_ptr,
    unsigned char* s2,
    size_t         max_len
);

void   increment_byte_copy (char** s1_ptr, char** s2_ptr, size_t count);
void   fill_with_blanks    (char** str_ptr, size_t count);
size_t find_string         (const char* str1, const char* str2);
size_t find_char           (const char* str, int test_char_arg);
size_t n_find_char         (const char* str, size_t len, int test_char);
size_t find_char_pair      (const char* str, int c1, int c2);
size_t count_char          (const char* str, int test_char);

int word_in_phrase(const char* phrase, const char* word);

void char_for_char_translate(char* str, int c1, int c2);
void remove_duplicate_chars(char* str, int c);
void str_delete(char* str, size_t num);

int str_insert
(
    char*       target_str,
    const char* str_to_insert,
    size_t      max_len
);

int const_sget_line(const char** str_ptr, char* line, size_t max_len);
int sget_line(char** str_ptr, char* line, size_t max_len);

int sget_line_2
(
    char**  str_ptr,
    char*   line,
    size_t  max_len,
    size_t* num_chars_processed_ptr
);

int const_sget_line_2
(
    const char**  str_ptr,
    char*   line,
    size_t  max_len,
    size_t* num_chars_processed_ptr
);

size_t get_str_indent(const char* str);
char last_char(const char* str);

int kjb_reverse(const char* input, char* output, size_t output_buff_len);

int string_is_in_string_array(char* test_str, char** str_array);
int is_pattern(const char* pattern);

int match_pattern(const char* pattern, const char* str);

int output_str
(
    const char* output_dir,
    const char* file_name,
    const char* value
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

