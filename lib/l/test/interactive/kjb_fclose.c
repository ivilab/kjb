
/* $Id: kjb_fclose.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    FILE* fp; 


    NPETE(fp = kjb_fopen("test_file", "w")); 

    pso("Close of test_file"); 
    EPE(kjb_fclose(fp));

    pso("SECOND close of test_file"); 
    EPE(kjb_fclose(fp));

    pso("Close of stdin");
    EPE(kjb_fclose(stdin));

    pso("Close of stdout");
    EPE(kjb_fclose(stdout));

    pso("Close of stderr");
    EPE(kjb_fclose(stderr));

    return EXIT_SUCCESS; 
}

