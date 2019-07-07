

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

    EPETE(pso("<h1><center>\n%s\n</center></h1>\n", argv[ 2 ]));

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

    EPETE(pso("<h2>Routines by category</h2>\n<p><dl>\n"));

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

            BUFF_CPY(out_buff, "<a href="); ;
            BUFF_CAT(out_buff, index_ptr[ i ].name);
            BUFF_CAT(out_buff, ".html>"); ;
            BUFF_CAT(out_buff, index_ptr[ i ].name);
            BUFF_CAT(out_buff, "</a>"); ;
        }
        else
        {
            BUFF_CAT(out_buff, ", &nbsp &nbsp");
            BUFF_CAT(out_buff, "<a href="); ;
            BUFF_CAT(out_buff, index_ptr[ i ].name);
            BUFF_CAT(out_buff, ".html>"); ;
            BUFF_CAT(out_buff, index_ptr[ i ].name);
            BUFF_CAT(out_buff, "</a>"); ;
        }
        prev_category = cur_category;
    }

    EPETE(output_category(out_buff, prev_category));

    pso("\n</p></dl>\n");

    EPETE(pso("<h2>Routines in alphabetical order</h2>\n"));

    ERE(put_line("<pre>\n")); 

    EPETE(kjb_sort((void*)index_ptr, num_index_lines, sizeof(Index),
                   compare_name_fn, USE_SORT_ATN_HANDLING));

    prev_name = "";

    for (i=0; i<num_index_lines; i++)
    {
        cur_name = index_ptr[ i ].name;

        if ( ! STRCMP_EQ(prev_name, cur_name) )
        {
            pso("     <a href=%s.html>%s</a>\n", cur_name, cur_name);
        }
        prev_name = cur_name;
    }

    ERE(put_line("</pre>")); 

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

static int filter_category_to_anchor_id(
    char* buf,            /* output:  anchor label derived from category */
    size_t size,          /* size of buf */
    const char* category  /* null-terminated string of unknown length */
)
{
    size_t j;
    size_t i = 0;         /* cursor through output buffer */

    /* Scan category, keep only the letters and numbers.
     * Change spaces to underscores.  Filter out the rest.
     * Example:  category="3D model I/O" buf="3D_model_IO"
     */
    for (j = 0; category[j] != '\0' && i < size-1; ++j)
    {
        if (isalnum(category[j]))
        {
            buf[i++] = category[j];
        }
        else if (category[j] == ' ')
        {
            buf[i++] = '_';
        }
    }
    buf[i] = '\0';

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int output_category(const char* buff, const char* category)
{

    char anchor_id[ 4096 ];

    ERE(filter_category_to_anchor_id(anchor_id, sizeof(anchor_id), category));
    ERE(pso("<p id=\"%s\">", anchor_id));
    ERE(pso("<dt><strong>%s</strong>\n<dd>", category));
    ERE(kjb_puts(buff));
    ERE(kjb_puts("</dd></dt></p>\n"));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int output_intro(char* buff)
{
    char        formatted_buff[ 100000 ];
    const char* formatted_buff_pos;
    char        out_line[ 1000 ];



    ERE(put_line("<pre>\n")); 

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

    ERE(put_line("</pre>\n")); 

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


