
/* $Id: kjb_fprintf.c 20919 2016-10-31 22:09:08Z kobus $ */


/*
   Always regenerate pse.c, pso.c, kjb_fprintf_term.c and fprintf.c 
   if this file is changed.

   Lines begining with null comment have stdout->stderr in kjb_fprintf_term.c 
*/

#include "l/l_incl.h" 

int main(void)
{
    int         i32;
    short       i16;
    float       fl;
    long_double ldb;
    char*       s1;
    int         res;
    int         i;
    int         j;
    int         count;
    char*       zap     = "zap";
    char*       longish = "longish";
    int         twelve  = 12;
    int         five    = 5;


    pso("%%ABC\n");

    s1 = "Hi there everybody !!!";

    i32 = 666; 
    i16 = 666;

    kjb_fprintf(stdout, "Some %%'s followed by 666: %% %% %% %d.\n",i32); 

    kjb_fprintf(stdout,"6 666's:   %d %hd %ld %i %hi %li.\n",
                i32, i16, i32, i32, i16, i32); 

    kjb_fprintf(stdout,"--%d--%10d--%+10d--%-10d--%010d--% d--\n",
                i32, i32, i32, i32, i32, i32);

    kjb_fprintf(stdout,"--%*.*d--\n", 8, 4, i32); 
    /**/ kjb_fprintf(stdout,"--%*.*d--\n", 15, 6, i32); 
    kjb_fprintf(stdout,"--%*.*d--\n", 15, 1, i32); 

    fl = 100.0; 

    for (i=0; i<10; ++i) 
    {
        fl=i*1.4+.3245;
        /**/     kjb_fprintf(stdout, "i is %d and fl is %7.*f\n",i,2,fl); 
        kjb_fprintf(stdout, "i is %ld and fl is %*.2f\n",i,7,fl); 
        /**/     kjb_fprintf(stdout, "i is %hd and fl is %*.*f\n",(short)i,7,i,fl); 
        kjb_fprintf(stdout, "i is %lx and fl is %*.*f\n",i,7,i,fl); 
        /**/     kjb_fprintf(stdout, "i is %-7d and fl is %*.*f\n",i,7,i,fl); 
        kjb_fprintf(stdout, "i is % 3d and fl is %e\n",i, fl); 
    }

    kjb_fprintf(stdout,"a %% b %% c %% d %s\n","hello"); 

    kjb_fprintf(stdout,"--%40s--.\n",s1); 
    kjb_fprintf(stdout,"--%15s--.\n",s1); 
    /**/ kjb_fprintf(stdout,"--%-40s--.\n",s1); 
    kjb_fprintf(stdout,"--%-15s--.\n",s1); 
    kjb_fprintf(stdout,"--%40.10s--.\n",s1); 
    /**/ kjb_fprintf(stdout,"--%15.10s--.\n",s1); 
    kjb_fprintf(stdout,"--%-40.10s--.\n",s1); 
    kjb_fprintf(stdout,"--%-15.10s--.\n",s1); 

    kjb_fprintf(stdout,"--%*.*s--.\n",40,15,s1);
    kjb_fprintf(stdout,"--%*.*s--.\n",40,5,s1);
    /**/ kjb_fprintf(stdout,"--%*.*s--.\n",10,8,s1);
    kjb_fprintf(stdout,"--%*.s--.\n",40,s1);
    /**/ kjb_fprintf(stdout,"--%*.s--.\n",20,s1);
    kjb_fprintf(stdout,"--%.*s--.\n",5,s1);
    /**/ kjb_fprintf(stdout,"--%30.*s--.\n",8,s1);

    kjb_fprintf(stdout,"--%.s--.\n",s1);
    kjb_fprintf(stdout,"--%.s--.\n",s1);

    /**/ kjb_fprintf(stdout, "fl is %*.*f\n",-7,-2,fl); 
    kjb_fprintf(stdout, "fl is %*.*f\n",-7,-1,fl); 

    for (i=0; i<10; ++i) 
    {
        ldb=i*1.4+.3245;
        kjb_fprintf(stdout, "i is %d and fl is %7.2f\n",i, (double)ldb); 
        kjb_fprintf(stdout, "i is %d and fl is %7.2Lf\n",i, ldb); 
        kjb_fprintf(stdout, "i is %d and fl is %7.*Lf\n",i, 2,ldb); 
        /**/     kjb_fprintf(stdout, "i is %ld and fl is %*.2Lf\n",i,7,ldb); 
        kjb_fprintf(stdout, "i is %hd and fl is %*.*Lf\n",(short)i,7,i,ldb); 
        /**/     kjb_fprintf(stdout, "i is %lx and fl is %*.*Lf\n",i,7,i,ldb); 
        kjb_fprintf(stdout, "i is %-7d and fl is %*.*Lf\n",i,7,i,ldb); 
        kjb_fprintf(stdout, "i is % 3d and fl is %Le\n",i, ldb); 
    }

    count = 1;
    i=45;
    j=-45;

    kjb_fprintf(stdout, 
                "\nCompare with  Harbison/Steele page 373, table 15-8\n\n");

    /**/ kjb_fprintf(stdout, "%-3d: ", count++);
    /**/ res = kjb_fprintf(stdout, "%12d | %12d", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    /**/ kjb_fprintf(stdout, "%-3d: ", count++);
    /**/ res = kjb_fprintf(stdout, "%012d | %012d", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    /**/ res = kjb_fprintf(stdout, "% 012d | % 012d", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    /**/ kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%+12d | %+12d", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    /**/ kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%+012d | %+012d", i, j); 
    /**/ kjb_fprintf(stdout, "    ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%-12d | %-12d", i, j); 
    /**/ kjb_fprintf(stdout, "    ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    /**/ res = kjb_fprintf(stdout, "%- 12d | %- 12d", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%-+12d | %-+12d", i, j); 
    /**/ kjb_fprintf(stdout, "    ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%12.4d | %12.4d", i, j); 
    /**/ kjb_fprintf(stdout, "    ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    /**/ res = kjb_fprintf(stdout, "%-12.4d | %-12.4d", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    count = 1;

    /**/ kjb_fprintf(stdout, 
    "\nCompare with  Harbison/Steele page 373, table 15-9\n\n");

    /**/ kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%14u | %14u", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    /**/ kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%014u | %014u", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%#14u | %#14u", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    /**/ kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%#014u | %#014u", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    /**/ res = kjb_fprintf(stdout, "%-14u | %-14u", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%-#14u | %-#14u", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    /**/ res = kjb_fprintf(stdout, "%14.4u | %14.4u", i, j); 
    kjb_fprintf(stdout, "    ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%-14.4u | %-14.4u", i, j); 
    /**/ kjb_fprintf(stdout, "    ---> %d\n", res);

    count = 1;

    kjb_fprintf(stdout, 
                "\nCompare with Harbison/Steele page 376, table 15-15\n\n");

    kjb_fprintf(stdout, "%-3d: ", count++);
    /**/ res = kjb_fprintf(stdout, "%12s  |  %12s  |  ", zap, longish); 
    kjb_fprintf(stdout, "  ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%12.5s  |  %12.5s  |  ", zap, longish); 
    kjb_fprintf(stdout, "  ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%012s  |  %012s  |  ", zap, longish); 
    kjb_fprintf(stdout, "  ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%-12s  |  %-12s  |  ", zap, longish); 
    kjb_fprintf(stdout, "  ---> %d\n", res);

    count = 1;

    kjb_fprintf(stdout, 
                "\nCompare with Harbison/Steele page 376, table 15-15\n\n");

    count = 1;

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%*s  |  %*s  |  ", twelve, zap, twelve, longish); 
    /**/ kjb_fprintf(stdout, "  ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%*.*s  |  %*.*s  |  ", twelve, five, zap, twelve,
                      five, longish); 
    /**/ kjb_fprintf(stdout, "  ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%0*s  |  %0*s  |  ", twelve, zap, twelve, 
                      longish); 
    /**/ kjb_fprintf(stdout, "  ---> %d\n", res);

    kjb_fprintf(stdout, "%-3d: ", count++);
    res = kjb_fprintf(stdout, "%-*s  |  %-*s  |  ", twelve, zap, twelve,
                      longish); 
    kjb_fprintf(stdout, "  ---> %d\n", res);


    return EXIT_SUCCESS; 
}

