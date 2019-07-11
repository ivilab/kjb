
#include "l/l_incl.h"

/*
 * This program removes empty #ifdef's. If they are nested, then only the inner
 * empty one is removed. In other words, an #ifdef that has only an empty #ifdef
 * inside it, is not empty. It could be removed by running the program a second
 * time on the output of the first run.
 *
 * This program ASSUMES that the code COMPILES. 
*/

int main(void)
{
    char line[ 10000 ];
    const char* line_pos;
    char token[ 1000 ];
    char save_if_line[ 10000 ]; 
    char if_prefix[ 10000 ]; 
    char if_suffix[ 10000 ]; 
    int  in_empty_if = FALSE; 
    int  in_empty_else = FALSE; 
    int  negated = FALSE; 
    int  blank_line_count = 0; 
    int  modified = 0; 


    while (BUFF_FGET_LINE(stdin, line) != EOF)
    {
        int if_noted = FALSE;
        int else_noted = FALSE;
        int endif_noted = FALSE;
        int count = 1; /* Room for NULL. */

        line_pos = line;

        count += const_trim_beg(&line_pos);

        if (*line_pos == '#')
        {
            line_pos++;
            count++;
            count += const_trim_beg(&line_pos);

            if (BUFF_CONST_GET_TOKEN_OK(&line_pos, token))
            {
                if (STRCMP_EQ(token, "ifdef"))
                {
                    if_noted = TRUE;
                    negated  = FALSE;
                }
                else if (STRCMP_EQ(token, "ifndef"))
                {
                    if_noted = TRUE;
                    negated  = TRUE;
                }
                else if (STRCMP_EQ(token, "else"))
                {
                    else_noted = TRUE;
                }
                else if (STRCMP_EQ(token, "endif"))
                { 
                    endif_noted = TRUE;
                }
            }
        }

        if ((in_empty_if) || (in_empty_else))
        {
            if (else_noted)
            {
                BUFF_CPY(save_if_line, if_prefix);

                if (negated)
                {
                    BUFF_CAT(save_if_line, "ifdef "); 
                }
                else
                {
                    BUFF_CAT(save_if_line, "ifndef "); 
                }

                BUFF_CAT(save_if_line, if_suffix); 

                modified = 1;

                blank_line_count = 0;

                continue;
            }
            else if (endif_noted)
            {
                if (in_empty_else)
                {
                    kjb_puts(line);
                    kjb_puts("\n");
                }

                in_empty_if = FALSE; 
                in_empty_else = FALSE; 
                blank_line_count = 0;
                modified = 1;

                continue;
            }
            else if (*line_pos == '\0')
            {
                blank_line_count++; 
                continue;
            }
            else 
            {
                int i;

                kjb_puts(save_if_line);
                kjb_puts("\n");

                for (i = 0; i < blank_line_count; i++)
                {
                    kjb_puts("\n");
                }

                in_empty_if = FALSE; 
                in_empty_else = FALSE; 
                blank_line_count = 0;
            }
        }

        ASSERT(! in_empty_if);
        ASSERT(! in_empty_else);
        ASSERT(blank_line_count == 0); 

        if (if_noted)
        {
            in_empty_if = TRUE;
            BUFF_CPY(save_if_line, line); 
            kjb_strncpy(if_prefix, line, count);   
            BUFF_CPY(if_suffix, line_pos); 
        }
        else if (else_noted)
        {
            /*
             * We can only be here if the #ifdef part was not empty. So we are
             * simply setting up for erasing the #else part if it is empty.
            */
            in_empty_else = TRUE;
            BUFF_CPY(save_if_line, line); 
        }
        else
        {
            kjb_puts(line);
            kjb_puts("\n");
        }

    }

    return modified; 
}

