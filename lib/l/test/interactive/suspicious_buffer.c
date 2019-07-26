
/* $Id: suspicious_buffer.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 

int main(int argc, char **argv)
{
    char b4[ 4 ];
    char b8[ 8 ];

    b4[0]='\0';
    b4[0]='\0';

    EPE(kjb_sprintf(b4, sizeof(b4), "HELLO")); 
    EPE(kjb_sprintf(b8, sizeof(b8), "HELLO")); 

    dbb(b4);
    dbb(b8); 

    pso("\n"); 

    BUFF_CPY(b4, "HELLO"); 
    BUFF_CPY(b8, "HELLO"); 

    dbb(b4);
    dbb(b8); 

    pso("\n"); 

    BUFF_CAT(b4, "-WORLD"); 
    BUFF_CAT(b8, "-WORLD"); 

    dbb(b4);
    dbb(b8); 

    kjb_strncat(b4, "-HELLO", sizeof(b4)); 
    kjb_strncat(b8, "-HELLO", sizeof(b8)); 

    dbb(b4);
    dbb(b8); 

    pso("\n"); 

    return EXIT_SUCCESS; 
}

