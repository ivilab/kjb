
#include "l/l_incl.h"

int main(int argc, char** argv)
{
    char line[ 10000 ];
    const char* line_pos;
    char token[ 1000 ];
    char def_str[ 1000 ];
    int  skip = FALSE;
    int  skip_next;
    int  num_endif_needed = 0;
    int  in_else = FALSE;
    int  negated = FALSE;
    int  skip_count = 0;
    int  in_block = FALSE;


    check_num_args(argc, 1, 1, (char*)NULL);

    BUFF_CPY(def_str, argv[ 1 ]);

    while (BUFF_FGET_LINE(stdin, line) != EOF)
    {
        line_pos = line;

        const_trim_beg(&line_pos);

        skip_next = skip;

        if (*line_pos == '#')
        {
            line_pos++;
            const_trim_beg(&line_pos);

            if (BUFF_CONST_GET_TOKEN_OK(&line_pos, token))
            {
                if (    (STRCMP_EQ(token, "ifdef"))
                     || (STRCMP_EQ(token, "if"))
                   )
                {
                    if (BUFF_CONST_GET_TOKEN_OK(&line_pos, token))
                    {
                        if ((! in_block ) && (STRCMP_EQ(token, def_str)))
                        {
                            skip = TRUE;
                            skip_next = TRUE;
                            in_block = TRUE;
                        }
                        if (in_block)
                        {
                            num_endif_needed++;
                        }
                    }
                }
                else if (STRCMP_EQ(token, "ifndef"))
                {
                    if (BUFF_CONST_GET_TOKEN_OK(&line_pos, token))
                    {
                        if (( ! in_block ) && (STRCMP_EQ(token, def_str)))
                        {
                            skip = TRUE;
                            skip_next = FALSE;
                            negated = TRUE;
                            in_block = TRUE;
                        }
                        if (in_block)
                        {
                            num_endif_needed++;
                        }
                    }
                }
                else if (     (in_block)
                          && (num_endif_needed == 1)
                          && (STRCMP_EQ(token, "else"))
                        )
                {
                    skip = TRUE;
                    in_else = TRUE;

                    if (negated)
                    {
                        skip_next = TRUE;
                    }
                    else
                    {
                        skip_next = FALSE;
                    }
                }
                else if ((in_block) && (STRCMP_EQ(token, "endif")))
                {
                    num_endif_needed--;

                    if (num_endif_needed == 0)
                    {
                        skip = TRUE;
                        skip_next = FALSE;
                        in_else = FALSE;
                        negated = FALSE;
                        in_block = FALSE;
                    }
                }
            }
        }

        if (in_else)
        {
            line_pos = line;
            const_trim_beg(&line_pos);

            if (HEAD_CMP_EQ(line_pos, "/*  ==>"))
            {
                skip = TRUE;
            }
        }

        if (skip)
        {
            skip_count++;
        }
        else
        {
            kjb_puts(line);
            kjb_puts("\n");
        }

        skip = skip_next;
    }


    /*
    return skip_count;
    */

    return EXIT_SUCCESS; 
}

