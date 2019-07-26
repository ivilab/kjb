
/* $Id: kjb_fprintf-2.c 20919 2016-10-31 22:09:08Z kobus $ */



#include "l/l_incl.h" 

int main(void)
{
    const char *longish  = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
    char buf[ 1000 ]; 


    pso("%%ABC\n");

    return 0;

    pso("%q\n", longish); 
    dbw(); 
    pso("%*q\n", 8, longish); 
    dbw(); 
    pso("%*q\n", 16, longish); 
    pso("%*q\n", 32, longish); 
    pso("%*q\n", 64, longish); 
    pso("%8q\n", longish); 
    pso("%16q\n", longish); 
    pso("%32q\n", longish); 
    pso("%64q\n", longish); 
    pso("\n"); 
    pso("\n"); 


    kjb_sprintf(buf, sizeof(buf), "%q\n", longish);
    pso(buf); 
    dbw(); 
    kjb_sprintf(buf, sizeof(buf), "%*q\n", 8, longish);
    dbw(); 
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%*q\n", 16, longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%*q\n", 32, longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%*q\n", 64, longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%8q\n", longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%16q\n", longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%32q\n", longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%64q\n", longish);
    pso(buf); 
    pso("\n"); 
    pso("\n"); 


    pso("%t\n", longish); 
    dbw(); 
    pso("%*t\n", 8, longish); 
    dbw(); 
    pso("%*t\n", 16, longish); 
    pso("%*t\n", 32, longish); 
    pso("%*t\n", 64, longish); 
    pso("%8t\n", longish); 
    pso("%16t\n", longish); 
    pso("%32t\n", longish); 
    pso("%64t\n", longish); 
    pso("\n"); 
    pso("\n"); 


    kjb_sprintf(buf, sizeof(buf), "%t\n", longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%*t\n", 8, longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%*t\n", 16, longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%*t\n", 32, longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%*t\n", 64, longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%8t\n", longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%16t\n", longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%32t\n", longish);
    pso(buf); 
    kjb_sprintf(buf, sizeof(buf), "%64t\n", longish);
    pso(buf); 
    pso("\n"); 
    pso("\n"); 


    return EXIT_SUCCESS; 
}

