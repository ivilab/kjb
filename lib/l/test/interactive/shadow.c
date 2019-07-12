
/* $Id: shadow.c 4723 2009-11-16 18:57:09Z kobus $ */


/*
   Always regenerate pse.c, pso.c, kjb_fprintf_term.c and fprintf.c 
   if this file is changed.

   Lines begining with null comment have stdout->stderr in kjb_fprintf_term.c 
*/

#include "l/l_incl.h" 

int main(void)
{
    const char* s1 = "Hello world.\n";
    const char* s1_pos;
    

    start_stdout_shadow("stdout_shadow"); 
    start_stderr_shadow("stderr_shadow"); 


    pso(s1);
    p_stderr(s1); 
    verbose_pso(1, s1);
    TEST_PSO((s1));
    kjb_fputs(stdout, s1);
    kjb_fputs(stderr, s1);

    s1_pos = s1;

    while (*s1_pos)
    {
        kjb_putc(*s1_pos);
        kjb_fputc(stderr, *s1_pos);

        s1_pos++; 
    }

    return EXIT_SUCCESS; 
}

