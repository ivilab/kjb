
/* $Id: string.c 21602 2017-07-31 20:36:24Z kobus $ */


#include "l/l_incl.h" 

/*ARGSUSED*/
main(int argc, char **argv)
{
    char line[ 50 ]; 
    char buff[ 200 ]; 
    char *buff_pos; 


    kjb_l_set("debug_level", "2"); 

    buff_pos = buff; 

    while (BUFF_STDIN_GET_LINE("> ",line) != EOF) 
    {
        dbi(find_char(line,'a')); 
        dbi(n_find_char(line,5,'a')); 
        dbi(count_char(line,'a')); 
        dbs(line); 
        rpad(line,3,5);
        dbs(line); 
        char_for_char_translate(line,'a','*'); 
        dbs(line); 
        str_char_build(&buff_pos,'!',10); 
        dbs(buff); 
        dbs(line); 
        dbnc(line, 7); 
        dbc(*(line + 1)); 
        
        dbs(line);
        dbi(strlen(line));
        dbi(trim_len(line)); 
    }

    return EXIT_SUCCESS; 
}

