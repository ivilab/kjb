
/* $Id: debug_macros.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "l/l_incl.h" 

int main(int argc, char **argv)
{
    short j=-5; 
    int i=-5;
    long k=-5;
    char *s;
    float f = -5.0;
    double d = -5.0;



    dbm("Hello world");

    s = "Hello";
    p_stderr("string s is ->%s<-\n", s);
    dbs(s);
    dbc(*(s+4));
    dbnc(s, 3);

    j = -5;
    p_stderr("short j is %hd\n", j);
    dbi(j);
    dbo(j);
    dbx(j);

    j = 9;
    p_stderr("short j is %hd\n", j);
    dbi(j);
    dbo(j);
    dbx(j);

    i = -5;
    p_stderr("int i is %d\n", i);
    dbi(i);
    dbo(i);
    dbx(i);

    i = 9;
    p_stderr("int i is %d\n", i);
    dbi(i);
    dbo(i);
    dbx(i);

    k = -5;
    p_stderr("long k is %ld\n", k);
    dbi(k);
    dbo(k);
    dbx(k);

    k = 9;
    p_stderr("long k is %ld\n", k);
    dbi(k);
    dbo(k);
    dbx(k);

    p_stderr("Float f is %f and double d is %f\n", f, d);

    dbf(f); 
    dbf(d);
    dbe(d);

    ASSERT(k==9);

    return EXIT_SUCCESS; 
}

