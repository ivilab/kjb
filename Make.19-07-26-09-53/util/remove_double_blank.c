
#include "l/l_incl.h"

/*ARGSUSED*/
int main(int argc, char** __attribute__((unused)) argv)
{
    char line[ 10000 ];
    const char* line_pos;

    check_num_args(argc, 0, 0, (char*)NULL);

    while (BUFF_FGET_LINE(stdin, line) != EOF)
    {
        kjb_puts(line);
        kjb_puts("\n");

        line_pos = line;

        const_trim_beg(&line_pos);

        if (*line_pos == '\0')
        {
            while (TRUE)
            {
                if (BUFF_FGET_LINE(stdin, line) == EOF)
                {
                    return EXIT_SUCCESS;
                }

                line_pos = line;

                const_trim_beg(&line_pos);

                if (*line_pos != '\0')
                {
                    kjb_puts(line);
                    kjb_puts("\n");
                    break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

