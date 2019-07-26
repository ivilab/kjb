
/* $Id: background.c 4723 2009-11-16 18:57:09Z kobus $ */



#include "l/l_incl.h" 


/*ARGSUSED*/
int main(int argc, char **argv)
{
    int   in_background;
    FILE* fp;



    fp = fopen("xxx", "w");
    fprintf(fp, "Before\n");
    fflush(fp);

    in_background = is_in_background();
    fprintf(fp, "After\n");
    fflush(fp);

    fprintf(stdout, "in_background is %d\n", in_background);

    if (in_background)
    {
        fprintf(stderr, "I'm in the background\n");
    }
    else 
    {
        fprintf(stderr, "I'm in the foreground\n");
    }

    return EXIT_SUCCESS; 
}

