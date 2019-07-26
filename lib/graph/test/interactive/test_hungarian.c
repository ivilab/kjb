
/* $Id: test_hungarian.c 21491 2017-07-20 13:19:02Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-1998, by Kobus Barnard (author).                         |
|                                                                              |
|  For use outside the SFU vision lab please contact the author(s).            |
|                                                                              |
* =========================================================================== */

#include "l/l_incl.h" 
#include "m/m_incl.h" 
#include "graph/hungarian.h"


/* -------------------------------------------------------------------------- */

  
/* -------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    Matrix* mp = NULL;
    Int_vector* row_vp = NULL; 
    double cost; 
    int result = EXIT_SUCCESS;


    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }

    EPETE(read_matrix(&mp, NULL)); 

    if (hungarian(mp, &row_vp, &cost) == ERROR)
    {
        kjb_print_error();
        result = EXIT_BUG;
    }
    else 
    {
        pso("cost: %.3e\n\n");
        write_col_int_vector(row_vp, NULL);
    }
    
    free_matrix(mp);

    free_int_vector(row_vp); 

    return result; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

