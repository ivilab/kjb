
/* $Id: round.c 21602 2017-07-31 20:36:24Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-1998, by Kobus Barnard (author).                         |
|                                                                              |
|  For use outside the SFU vision lab please contact the author(s).            |
|                                                                              |
* =========================================================================== */

#include "l/l_incl.h" 

/* -------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    char line[ 1000 ];
    double d;


    while (BUFF_STDIN_GET_LINE("double> ", line) != EOF)
    {
        if (ss1d(line, &d) == ERROR)
        {
            kjb_print_error();
        }
        else
        {
            pso("%d == %d?\n", kjb_rint(d), (int)rint(d));
        }
    }
    
    return EXIT_SUCCESS; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

