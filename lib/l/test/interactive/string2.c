
/* $Id: string2.c 21602 2017-07-31 20:36:24Z kobus $ */


#include "l/l_incl.h" 


/*ARGSUSED*/
int main(int argc, char **argv)
{
    char  line_copy[ 10000 ];
    char  line2[ 10000 ];
    char  line[ 10000 ];
    char* line_pos;
    int   res;


    while ((res=BUFF_STDIN_GET_LINE("str>", line_copy)) != EOF)
    {
        EPETE(res);

        BUFF_CPY(line, line_copy);
        line_pos = line;

        pso("The trim_len of that is %d.\n", trim_len(line_pos));
        put_line("");

        BUFF_CPY(line, line_copy);
        line_pos = line;
        put_line("Lets trim the begining with trim_beg");
        res = trim_beg(&line_pos);
        pso("Trim_beg returned %d.\n", res);
        pso("Line is ->%s<-\n", line_pos);
        put_line("");


        BUFF_CPY(line, line_copy);
        put_line("Lets trim the end with trim_end");
        res = trim_end(line);
        pso("Trim_end returned %d.\n", res);
        pso("Line is ->%s<-\n", line);
        put_line("");

        BUFF_CPY(line, line_copy);
        put_line("Lets translate to lc with extended_uc_lc");
        extended_uc_lc(line);
        put_line(line);
        put_line("");

        BUFF_CPY(line, line_copy);
        put_line("Lets translate to uc with extended_lc_uc");
        extended_lc_uc(line);
        pso("Line is ->%s<-\n", line);
        put_line("");

        BUFF_CPY(line, line_copy);
        put_line("Lets translate to lc with extended_n_uc_lc with n=5");
        extended_n_uc_lc(line, 5);
        pso("Line is ->%s<-\n", line);
        put_line("");

        BUFF_CPY(line, line_copy);
        put_line("Lets translate to uc with extended_n_lc_uc with n=5");
        extended_n_lc_uc(line, 5);
        pso("Line is ->%s<-\n", line);
        put_line("");


        BUFF_CPY(line, line_copy);
        put_line("Lets add some blanks to make it 50 chars wide");
        rpad(line, strlen(line), 50);
        pso("Line is ->%s<-\n", line);
        put_line("");

        BUFF_CPY(line, line_copy);
        put_line("Lets make a 50 char wide copy");
        rpad_cpy(line2, line, 50);
        pso("Line is ->%s<-\n", line2);
        put_line("");

        BUFF_CPY(line, line_copy);
        put_line("Lets add 12345");
        BUFF_CAT(line, "12345");
        pso("Line is ->%s<-\n", line);
        put_line("");

        BUFF_CPY(line, line_copy);
        put_line("Lets add 12345 but suppose the buffer size is 8");
        kjb_strncat(line, "12345", 8);
        pso("Line is ->%s<-\n", line);
        put_line("");
    }

    return EXIT_SUCCESS; 
}

