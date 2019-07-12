
/* $Id: trim.c 21590 2017-07-30 00:18:11Z kobus $ */


#include "l/l_incl.h" 


int main(int argc, char* argv[])
{
    const char* base_str;
    char  line[ 3000 ];
    int   res;
    char buff_1[ 1 ];
    char buff_2[ 2 ];
    char buff_3[ 3 ];
    char buff_4[ 4 ];
    char buff_5[ 5 ];
    char buff_6[ 6 ];
    char buff_7[ 7 ];
    char buff_8[ 8 ];
    char buff_9[ 9 ];
    char buff_16[ 16 ];
    char buff_32[ 32 ];
    char buff_64[ 32 ];

    /* The truncate routines are used by the bug reporting routines. */
    kjb_set_debug_level(1);

    check_num_args(argc, 1, 1, NULL);

    base_str = argv[ 1];

    while ((res=BUFF_STDIN_GET_LINE("str> ",line)) != EOF) 
    {
        BUFF_TRUNC_CPY(buff_1, base_str);
        dbb(buff_1);
        BUFF_TRUNC_CAT(buff_1, line);
        dbb(buff_1);
        BUFF_TRUNC_QUOTE_CPY(buff_1, base_str);
        dbb(buff_1);

        BUFF_TRUNC_CPY(buff_2, base_str);
        dbb(buff_2);
        BUFF_TRUNC_CAT(buff_2, line);
        dbb(buff_2);
        BUFF_TRUNC_QUOTE_CPY(buff_2, base_str);
        dbb(buff_2);

        BUFF_TRUNC_CPY(buff_3, base_str);
        dbb(buff_3);
        BUFF_TRUNC_CAT(buff_3, line);
        dbb(buff_3);
        BUFF_TRUNC_QUOTE_CPY(buff_3, base_str);
        dbb(buff_3);

        BUFF_TRUNC_CPY(buff_4, base_str);
        dbb(buff_4);
        BUFF_TRUNC_CAT(buff_4, line);
        dbb(buff_4);
        BUFF_TRUNC_QUOTE_CPY(buff_4, base_str);
        dbb(buff_4);

        BUFF_TRUNC_CPY(buff_5, base_str);
        dbb(buff_5);
        BUFF_TRUNC_CAT(buff_5, line);
        dbb(buff_5);
        BUFF_TRUNC_QUOTE_CPY(buff_5, base_str);
        dbb(buff_5);

        BUFF_TRUNC_CPY(buff_6, base_str);
        dbb(buff_6);
        BUFF_TRUNC_CAT(buff_6, line);
        dbb(buff_6);
        BUFF_TRUNC_QUOTE_CPY(buff_6, base_str);
        dbb(buff_6);

        BUFF_TRUNC_CPY(buff_7, base_str);
        dbb(buff_7);
        BUFF_TRUNC_CAT(buff_7, line);
        dbb(buff_7);
        BUFF_TRUNC_QUOTE_CPY(buff_7, line);
        dbb(buff_7);

        BUFF_TRUNC_CPY(buff_8, base_str);
        dbb(buff_8);
        BUFF_TRUNC_CAT(buff_8, line);
        dbb(buff_8);
        BUFF_TRUNC_QUOTE_CPY(buff_8, base_str);
        dbb(buff_8);

        BUFF_TRUNC_CPY(buff_9, base_str);
        dbb(buff_9);
        BUFF_TRUNC_CAT(buff_9, line);
        dbb(buff_9);
        BUFF_TRUNC_QUOTE_CPY(buff_9, base_str);
        dbb(buff_9);

        BUFF_TRUNC_CPY(buff_16, base_str);
        dbb(buff_16);
        BUFF_TRUNC_CAT(buff_16, line);
        dbb(buff_16);
        BUFF_TRUNC_QUOTE_CPY(buff_16, base_str);
        dbb(buff_16);

        BUFF_TRUNC_CPY(buff_32, base_str);
        dbb(buff_32);
        BUFF_TRUNC_CAT(buff_32, line);
        dbb(buff_32);
        BUFF_TRUNC_QUOTE_CPY(buff_32, base_str);
        dbb(buff_32);

        BUFF_TRUNC_CPY(buff_64, base_str);
        dbb(buff_64);
        BUFF_TRUNC_CAT(buff_64, line);
        dbb(buff_64);
        BUFF_TRUNC_QUOTE_CPY(buff_64, base_str);
        dbb(buff_64);

    }


    kjb_exit(EXIT_SUCCESS); 
}

