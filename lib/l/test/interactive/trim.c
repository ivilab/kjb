
/* $Id: trim.c 21654 2017-08-05 14:10:14Z kobus $ */


#include "l/l_incl.h" 


int main(void)
{
    char  line[ 3000 ];
    int   res;
#ifdef DISABLE
    char   *str1       = "   Hello World";
    char   *str2       = "Hello World   ";
    char   *str1_pos; 
    char   *str2_pos; 
    SIZE_T result;    

    str1_pos = str1;

    dbi(trim_beg(&str1_pos)); 
    dbi(put_line(str1_pos));

    str2_pos = str2;

    dbi(trim_end(str2_pos)); 
    dbi(put_line(str2_pos)); 
#endif

#ifdef TRY_MANIPULATING_CONSTANT_STRINGS_ONE
    char *str = "   Hello World";
    char *str_pos = str;
    trim_beg(&str_pos); 
    put_line(str_pos); 
    dbi(strlen(str_pos));
#endif

#ifdef TRY_MANIPULATING_CONSTANT_STRINGS_TWO
    char *str = "Hello World   ";
    char *str_pos = str;
    trim_end(str_pos);   
    put_line(str_pos); 
    dbi(strlen(str_pos));
#endif 

    /* The trim routines are used by the bug reporting routines. */
    kjb_set_debug_level(1);

    kjb_init();

    while ((res=BUFF_STDIN_GET_LINE("str> ",line)) != EOF) 
    {
        char* line_ptr = line;
        size_t line_trim_len;
        int    trim_count;

        dbi(res);

        dbp("");
        dbs(line_ptr);
        dbi(strlen(line_ptr));
        line_trim_len = trim_len(line_ptr);
        dbi(line_trim_len);

        dbp("");
        dbp("gen_trim_beg");
        trim_count = gen_trim_beg(&line_ptr, " ");
        dbs(line_ptr);
        line_trim_len = trim_len(line_ptr);
        dbi(strlen(line_ptr));
        dbi(line_trim_len);
        dbi(trim_count);


        dbp("");
        dbp("gen_trim_end");
        trim_count = gen_trim_end(line_ptr, " ");
        dbs(line_ptr);
        line_trim_len = trim_len(line_ptr);
        dbi(strlen(line_ptr));
        dbi(line_trim_len);
        dbi(trim_count);
    }

    kjb_exit(EXIT_SUCCESS); 
}

