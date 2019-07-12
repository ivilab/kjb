
/* $Id: justify2.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 


#define justify left_justify


int main(int argc, char **argv)
{
    IMPORT int kjb_debug_level;
    char in_buff[ 10000 ];
    char out_buff[ 10000 ] = "XXXXX";
    char *buff_pos = in_buff;
    int c;
    int i, width, no_format_margin; 
    int justify_res; 
    int keep_indent;
    int extra_indent;

    for (keep_indent=0; keep_indent<2; keep_indent++)
    {
        for (extra_indent=0; extra_indent<15; extra_indent+=5)
        {
            pso("\n\nkeep indent is now %d and extra inden is %d\n\n",
                keep_indent, extra_indent); 


            for (i=0; i<sizeof(out_buff); i++)
            {
                out_buff[ i ] = '\0';
            }

            while ((c = getchar()) != EOF)
            {
                *buff_pos++ = c;
            }

            *buff_pos = '\0';

            kjb_debug_level = 0; 

            p_stderr("---------------------\n");
            p_stderr("TESTING BUFFER OVERFLOW\n");


            for (i=1; i<100; i++)
            {
                width = 1;
                no_format_margin = 1;

                dbi(i);
                dbi(width);
                dbi(no_format_margin); 

                justify_res = justify(in_buff, width, no_format_margin, (char*)NULL,
                                      keep_indent, extra_indent, out_buff, i);
                dbr(justify_res); 
                dbi(strlen(out_buff));

                if ((i > 0) && (strlen(out_buff) > i - 1))
                {
                    p_stderr("Buffer size %d caused overflow with (%d, %d).\n", 
                             i, width, no_format_margin); 
                }

                if (kjb_debug_level > 1)
                {
                    kjb_fputs(stderr, out_buff); 
                    p_stderr("---------------------\n"); 
                }
            }

            for (i=1; i<100; i++)
            {
                width = 5;
                no_format_margin = 0;

                dbi(i);
                dbi(width);
                dbi(no_format_margin); 

                justify_res = justify(in_buff, width, no_format_margin, (char*)NULL,
                                      keep_indent, extra_indent, out_buff, i);
                dbr(justify_res); 
                dbi(strlen(out_buff));

                if (    ((i == 0) && (justify_res != ERROR))
                        || ((i > 0) && (strlen(out_buff) > i - 1))
                   )
                {
                    p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, 
                             width, no_format_margin); 
                }

                if (kjb_debug_level > 1)
                {
                    kjb_fputs(stderr, out_buff); 
                    p_stderr("---------------------\n"); 
                }
            }

            for (i=1; i<100; i++)
            {

                width = 30; 
                no_format_margin = 5;

                dbi(i);
                dbi(width);
                dbi(no_format_margin); 

                justify_res = justify(in_buff, width, no_format_margin, (char*)NULL,
                                      keep_indent, extra_indent, out_buff, i);
                dbr(justify_res); 
                dbi(strlen(out_buff));

                if (    ((i == 0) && (justify_res != ERROR))
                        || ((i > 0) && (strlen(out_buff) > i - 1))
                   )
                {
                    p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i,
                             width, no_format_margin); 
                }

                if (kjb_debug_level > 1)
                {
                    kjb_fputs(stderr, out_buff); 
                    p_stderr("---------------------\n"); 
                }
            }

            for (i=1; i<100; i++)
            {
                width = 90;
                no_format_margin = 10;

                dbi(i);
                dbi(width);
                dbi(no_format_margin); 

                justify_res = justify(in_buff, width, no_format_margin, (char*)NULL,
                                      keep_indent, extra_indent, out_buff, i);
                dbr(justify_res); 
                dbi(strlen(out_buff));

                if (    ((i == 0) && (justify_res != ERROR))
                        || ((i > 0) && (strlen(out_buff) > i - 1))
                   )
                {
                    p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, 
                             width, no_format_margin); 
                }

                if (kjb_debug_level > 1)
                {
                    kjb_fputs(stderr, out_buff); 
                    p_stderr("---------------------\n"); 
                }
            }

            for (i=1; i<100; i++)
            {
                width = 90;
                no_format_margin = 0;

                dbi(i);
                dbi(width);
                dbi(no_format_margin); 

                justify_res = justify(in_buff, width, no_format_margin, (char*)NULL,
                                      keep_indent, extra_indent, out_buff, i);
                dbr(justify_res); 
                dbi(strlen(out_buff));

                if (    ((i == 0) && (justify_res != ERROR))
                        || ((i > 0) && (strlen(out_buff) > i - 1))
                   )
                {
                    p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, 
                             width, no_format_margin); 
                }

                if (kjb_debug_level > 1)
                {
                    kjb_fputs(stderr, out_buff); 
                    p_stderr("---------------------\n"); 
                }
            }

            for (i=1; i<100; i++)
            {
                width = 3;
                no_format_margin = 1;

                dbi(i);
                dbi(width);
                dbi(no_format_margin); 

                justify_res = justify(in_buff, width, no_format_margin, (char*)NULL,
                                      keep_indent, extra_indent, out_buff, i);
                dbr(justify_res); 
                dbi(strlen(out_buff));

                if (    ((i == 0) && (justify_res != ERROR))
                        || ((i > 0) && (strlen(out_buff) > i - 1))
                   )
                {
                    p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, 
                             width, no_format_margin); 
                }

                if (kjb_debug_level > 1)
                {
                    kjb_fputs(stderr, out_buff); 
                    p_stderr("---------------------\n"); 
                }
            }

            p_stderr("---------------------\n"); 

            kjb_debug_level = 2; 

            i = sizeof(out_buff);
            width = 3;
            no_format_margin = 1;

            dbi(i);
            dbi(width);
            dbi(no_format_margin); 

            justify_res = justify(in_buff, width, no_format_margin, (char*)NULL, keep_indent, extra_indent, out_buff, i);

            if ((i > 0) && (strlen(out_buff) > i - 1))
            {
                p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, width,
                         no_format_margin); 
            }

            dbr(justify_res); 
            kjb_fputs(stderr, out_buff); 

            p_stderr("---------------------\n"); 

            kjb_debug_level = 2; 

            dbw();

            i = sizeof(out_buff);
            width = 1;
            no_format_margin = 0;

            dbi(i);
            dbi(width);
            dbi(no_format_margin); 

            justify_res = justify(in_buff, width, no_format_margin, 
                                  (char*)NULL, keep_indent, extra_indent, out_buff, i);

            if ((i > 0) && (strlen(out_buff) > i - 1))
            {
                p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, width,
                         no_format_margin); 
            }

            dbr(justify_res); 
            kjb_fputs(stderr, out_buff); 

            p_stderr("---------------------\n"); 
            i = sizeof(out_buff);
            width = 2;
            no_format_margin = 0;

            dbi(i);
            dbi(width);
            dbi(no_format_margin); 

            justify_res = justify(in_buff, width, no_format_margin, (char*)NULL, 
                                  keep_indent, extra_indent, out_buff, i);

            if ((i > 0) && (strlen(out_buff) > i - 1))
            {
                p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, width,
                         no_format_margin); 
            }

            dbr(justify_res); 
            kjb_fputs(stderr, out_buff); 

            kjb_fputs(stderr, out_buff); 
            p_stderr("---------------------\n"); 

            i = sizeof(out_buff);
            width = 90;
            no_format_margin = 5;

            dbi(i);
            dbi(width);
            dbi(no_format_margin); 

            justify_res = justify(in_buff, width, no_format_margin, (char*)NULL, 
                                  keep_indent, extra_indent, out_buff, i);

            if ((i > 0) && (strlen(out_buff) > i - 1))
            {
                p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, width,
                         no_format_margin); 
            }

            dbr(justify_res); 
            kjb_fputs(stderr, out_buff); 
            p_stderr("---------------------\n"); 

            i = sizeof(out_buff);
            width = 30;
            no_format_margin = 0;

            dbi(i);
            dbi(width);
            dbi(no_format_margin); 

            justify_res = justify(in_buff, width, no_format_margin, (char*)NULL, 
                                  keep_indent, extra_indent, out_buff, i);

            if ((i > 0) && (strlen(out_buff) > i - 1))
            {
                p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, width,
                         no_format_margin); 
            }

            dbr(justify_res); 
            kjb_fputs(stderr, out_buff); 
            p_stderr("---------------------\n"); 

            i = sizeof(out_buff);
            width = 10;
            no_format_margin = 10;

            dbi(i);
            dbi(width);
            dbi(no_format_margin); 

            justify_res = justify(in_buff, width, no_format_margin, (char*)NULL, 
                                  keep_indent, extra_indent, out_buff, i);

            if ((i > 0) && (strlen(out_buff) > i - 1))
            {
                p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, width,
                         no_format_margin); 
            }

            dbr(justify_res); 
            kjb_fputs(stderr, out_buff); 
            p_stderr("---------------------\n"); 

            i = sizeof(out_buff);
            width = 30;
            no_format_margin = 10;

            dbi(i);
            dbi(width);
            dbi(no_format_margin); 

            justify_res = justify(in_buff, width, no_format_margin, (char*)NULL,
                                  keep_indent, extra_indent, out_buff, i);

            if ((i > 0) && (strlen(out_buff) > i - 1))
            {
                p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, width,
                         no_format_margin); 
            }

            dbr(justify_res); 
            kjb_fputs(stderr, out_buff); 
            p_stderr("---------------------\n"); 

            i = sizeof(out_buff);
            width = 60;
            no_format_margin = 10;

            dbi(i);
            dbi(width);
            dbi(no_format_margin); 

            justify_res = justify(in_buff, width, no_format_margin, (char*)NULL,
                                  keep_indent, extra_indent, out_buff, i);
            dbr(justify_res); 

            if ((i > 0) && (strlen(out_buff) > i - 1))
            {
                p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, width,
                         no_format_margin); 
            }

            kjb_fputs(stderr, out_buff); 
            p_stderr("---------------------\n"); 

            i = sizeof(out_buff);
            width = 120;
            no_format_margin = 5;

            dbi(i);
            dbi(width);
            dbi(no_format_margin); 

            justify_res = justify(in_buff, width, no_format_margin, (char*)NULL,
                                  keep_indent, extra_indent, out_buff, i);

            if ((i > 0) && (strlen(out_buff) > i - 1))
            {
                p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, width,
                         no_format_margin); 
            }

            dbr(justify_res); 
            kjb_fputs(stderr, out_buff); 
            p_stderr("---------------------\n"); 

            p_stderr("Using no_justify_chars == \"*@\"\n");

            i = sizeof(out_buff);
            width = 20;
            no_format_margin = 0;

            dbi(i);
            dbi(width);
            dbi(no_format_margin); 

            justify_res = justify(in_buff, width, no_format_margin, "*@", keep_indent, extra_indent, out_buff, i);

            if ((i > 0) && (strlen(out_buff) > i - 1))
            {
                p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, width,
                         no_format_margin); 
            }

            dbr(justify_res); 
            kjb_fputs(stderr, out_buff); 
            p_stderr("---------------------\n"); 

            p_stderr("Using no_justify_chars == \"*@\"\n");

            i = sizeof(out_buff);
            width = 20;
            no_format_margin = 1;

            dbi(i);
            dbi(width);
            dbi(no_format_margin); 

            justify_res = justify(in_buff, width, no_format_margin, "*@", keep_indent, extra_indent, out_buff, i);

            if ((i > 0) && (strlen(out_buff) > i - 1))
            {
                p_stderr("Buffer size %d caused overflow with (%d, %d).\n", i, width,
                         no_format_margin); 
            }

            dbr(justify_res); 
            kjb_fputs(stderr, out_buff); 
            p_stderr("---------------------\n"); 
        }
    }

    return EXIT_SUCCESS; 
}

