
/* $Id: justify.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 


#define justify left_justify


/*ARGSUSED*/
int main(int argc, char **argv)
{
    char  big_buff[ 100000 ]     = { '\0' };
    char  out_buff[ 100000 ];
    char  line[ 1000 ];
    char  no_format_chars[ 100 ] = { '|', '\0' };
    int   width                  = 60;
    int   keep_indent            = 1;
    int   extra_indent           = 5;
    int   no_format_indent       = 0;
    int   res;

    if (argc >= 2)
    {
        EPETE(ss1pi(argv[ 1 ], &width));
    }

    if (argc >= 3)
    {
        EPETE(ss1pi(argv[ 2 ], &no_format_indent));
    }

    if (argc >= 4)
    {
        BUFF_CPY(no_format_chars, argv[ 3 ]);
    }

    if (argc >= 5)
    {
        EPETE(ss1pi(argv[ 4 ], &keep_indent));
    }

    if (argc >= 6)
    {
        EPETE(ss1pi(argv[ 5 ], &extra_indent));
    }

    while ((res = BUFF_FGET_LINE(stdin, line)) != EOF)
    {
        ASSERT(res >= 0);

        BUFF_CAT(big_buff, line);
        BUFF_CAT(big_buff, "\n");
    }

    EPETE(justify(big_buff, width, no_format_indent, no_format_chars,
                  keep_indent, extra_indent, out_buff, sizeof(out_buff)));

    pso(out_buff); 

    return EXIT_SUCCESS; 
}

