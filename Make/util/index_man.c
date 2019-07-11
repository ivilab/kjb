

#include "l/l_incl.h"

/* ------------------------------------------------------------------------- */

#ifndef OUTPUT_WIDTH
#    define OUTPUT_WIDTH 80
#endif

/* ------------------------------------------------------------------------- */

typedef struct Index
{
    char* name;
    char* category;
}
Index;

/* ------------------------------------------------------------------------- */

static int compare_name_fn
(
    const void* elem_one_ptr,
    const void* elem_two_ptr
);

static int compare_category_fn
(
    const void* elem_one_ptr,
    const void* elem_two_ptr
);

static int output_category(const char* buff, const char* category);
static int output_intro(char* buff);

/* ------------------------------------------------------------------------- */

int main(int argc, char** argv)
{
    FILE*  index_fp = NULL;
    Index* index_ptr;
    int    num_index_lines;
    char   line[ 1000 ];
    char*  line_pos;
    char   name_buff[ 1000 ];
    int         i;
    const char* prev_name;
    char*       cur_name;
    const char* prev_category;
    char*       cur_category;
    char        out_buff[ 2000000 ] = { '\0' };
    FILE*       intro_fp = NULL;


    if ((argc != 3) && (argc != 4))
    {
        p_stderr("Usage: index_man <index_file> <title> { <intro_file> } \n");
        kjb_exit(EXIT_FAILURE);
    }

    NPETE(index_fp = kjb_fopen(argv[ 1 ], "r"));

    EPETE(num_index_lines = count_real_lines(index_fp));

    if (num_index_lines == 0)
    {
        p_stderr("Warning : No lines in index file.\n");
        kjb_exit(EXIT_SUCCESS);
    }


    NPETE(index_ptr = N_TYPE_MALLOC(Index, num_index_lines));

    for (i=0; i<num_index_lines; i++)
    {
        BUFF_GET_REAL_LINE(index_fp, line);

        line_pos = line;

        BUFF_GET_TOKEN(&line_pos, name_buff);
        NPETE(index_ptr[ i ].name = kjb_strdup(name_buff));

        trim_beg(&line_pos);
        trim_end(line_pos);

        NPETE(index_ptr[ i ].category = kjb_strdup(line_pos));
    }

    EPETE(man_print_title(stdout, argv[ 2 ], "3"));
    EPETE(pso("\n\n"));

    if ((argc == 4) && ((intro_fp = kjb_fopen(argv[ 3 ], "r")) != NULL))
    {
        char intro_buff[ 200000 ];


        intro_buff[ 0 ] = '\0';

        /* Can't use get_real_line, because we want blanks. */
        while (BUFF_FGET_LINE(intro_fp, line) != EOF)
        {
            IMPORT int kjb_comment_char;

            if (line[ 0 ] != kjb_comment_char)
            {
                /* Be slow and inefficient */
                BUFF_CAT(intro_buff, line);
                BUFF_CAT(intro_buff, "\n");
            }
        }

        output_intro(intro_buff);
    }

    man_print_heading(stdout, "Routines by category");
    pso("\n");

    EPETE(kjb_sort((void*)index_ptr, num_index_lines, sizeof(Index),
                   compare_category_fn, USE_SORT_ATN_HANDLING));

    prev_category = "";

    for (i=0; i<num_index_lines; i++)
    {
        cur_category = index_ptr[ i ].category;

        if ( ! STRCMP_EQ(prev_category, cur_category) )
        {
            if (i != 0)
            {
                EPETE(output_category(out_buff, prev_category));
            }

            BUFF_CPY(out_buff, index_ptr[ i ].name);
        }
        else
        {
            BUFF_CAT(out_buff, ", ");
            BUFF_CAT(out_buff, index_ptr[ i ].name);
        }
        prev_category = cur_category;
    }

    EPETE(output_category(out_buff, prev_category));

    pso("\n\n");

    man_print_heading(stdout, "Routines in alphabetical order");
    pso("\n");

    EPETE(kjb_sort((void*)index_ptr, num_index_lines, sizeof(Index),
                   compare_name_fn, USE_SORT_ATN_HANDLING));

    prev_name = "";

    for (i=0; i<num_index_lines; i++)
    {
        cur_name = index_ptr[ i ].name;

        if ( ! STRCMP_EQ(prev_name, cur_name) )
        {
            pso("     ");
            pso(cur_name);
            pso("\n");
        }
        prev_name = cur_name;
    }

    for (i=0; i<num_index_lines; i++)
    {
        kjb_free(index_ptr[ i ].name);
        kjb_free(index_ptr[ i ].category);
    }
    kjb_free(index_ptr);

    kjb_fclose(index_fp);
    kjb_fclose(intro_fp);

    return EXIT_SUCCESS;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int output_category(const char* buff, const char* category)
{
    char        formatted_buff[ 100000 ];
    const char* formatted_buff_pos;
    char        out_line[ 1000 ];



    pso("    ");
    print_underlined(stdout, category);
    pso("\n");

    ERE(left_justify(buff, OUTPUT_WIDTH, 0, "", 0, 10, formatted_buff,
                     sizeof(formatted_buff)));

    formatted_buff_pos = formatted_buff;

    while (BUFF_CONST_SGET_LINE(&formatted_buff_pos, out_line) != EOF)
    {
        put_line(out_line);
    }

    pso("\n");

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int output_intro(char* buff)
{
    char        formatted_buff[ 100000 ];
    const char* formatted_buff_pos;
    char        out_line[ 1000 ];




    ERE(left_justify(buff, OUTPUT_WIDTH, 0, "|", 1, 0, formatted_buff,
                     sizeof(formatted_buff)));

    formatted_buff_pos = formatted_buff;

    while (BUFF_CONST_SGET_LINE(&formatted_buff_pos, out_line) != EOF)
    {
        if (*out_line == '|')
        {
            *out_line = ' ';
        }
        put_line(out_line);
    }

    pso("\n");

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int compare_name_fn
(
    const void* elem_one_ptr,
    const void* elem_two_ptr
)
{
    return kjb_strcmp(((const Index*)elem_one_ptr)->name,
                      ((const Index*)elem_two_ptr)->name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int compare_category_fn
(
    const void* elem_one_ptr,
    const void* elem_two_ptr
)
{
    int result;

    result = kjb_strcmp(((const Index*)elem_one_ptr)->category,
                       ((const Index*)elem_two_ptr)->category);

    if (result != EQUAL_STRINGS) return result;

    return kjb_strcmp(((const Index*)elem_one_ptr)->name,
                      ((const Index*)elem_two_ptr)->name);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


