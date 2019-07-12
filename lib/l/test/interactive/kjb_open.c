
/* $Id: kjb_open.c 21602 2017-07-31 20:36:24Z kobus $ */


#include "l/l_incl.h" 


/*ARGSUSED*/
int main(int argc, char **argv)
{
    char  line[ 3000 ];
    char  path[ 1000 ];
    char  name[ 1000 ];
    char  mode[ 10 ];
    int   res;
    int   do_it;
    FILE* fp;


    kjb_l_set("debug_level", "1"); 

    BUFF_GET_FD_NAME(fileno(stdin), name);
    p_stderr("Name of stdin is %s (kjb_isatty==>%d).\n", name,
             kjb_isatty(fileno(stdin))); 

    BUFF_GET_FD_NAME(fileno(stdout), name);
    p_stderr("Name of stdout is %s (kjb_isatty==>%d).\n", name,
             kjb_isatty(fileno(stdout))); 

    BUFF_GET_FD_NAME(fileno(stderr), name);
    p_stderr("Name of stderr is %s (kjb_isatty==>%d).\n", name,
             kjb_isatty(fileno(stderr))); 

    p_stderr("\n");

    term_set_raw_mode_with_no_echo();

    while ((res = BUFF_STDIN_GET_LINE("file> ",line)) != EOF) 
    {
        EPETE(res);

        res = BUFF_STDIN_GET_LINE("mode> ",mode);

        EPETE(res);

        do_it = TRUE;

        res = BUFF_REAL_PATH(line, path);
        if (res != ERROR)
        {
            p_stderr("PATH: %s\n", path); 
        }

        if (*mode == 'w') 
        {
            if ((res != ERROR) && (! HEAD_CMP_EQ("/dev", path))) 
            {
                if (! confirm_risky_action("Overwrite? ")) 
                {
                    do_it = FALSE;
                }
            }
        }

        if (do_it) 
        {
            NPE(fp = kjb_fopen(line, mode));
        }
        else 
        {
            fp = NULL;
        }

        if (fp != NULL) 
        {
            EPE(BUFF_GET_FD_NAME(fileno(fp), name));
            p_stderr("REAL Name is: %s (kjb_isatty==>%d)\n", name,
                     kjb_isatty(fileno(fp))); 

            EPE(BUFF_GET_USER_FD_NAME(fileno(fp), name));
            p_stderr("USER Name is: %s (kjb_isatty==>%d)\n", name, 
                     kjb_isatty(fileno(fp))); 
        }

        p_stderr("\n");
    }

    term_reset();  /* Not really needed because kjb_cleanup below. */

    kjb_cleanup();

    return EXIT_SUCCESS; 
}

