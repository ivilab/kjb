
/* $Id: test_jv_lap.c 21491 2017-07-20 13:19:02Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 1994-1998, by Kobus Barnard (author).                         |
|                                                                              |
|  For use outside the SFU vision lab please contact the author(s).            |
|                                                                              |
* =========================================================================== */

#include "l/l_incl.h" 
#include "m/m_incl.h" 
#include "graph/jv_lap.h"
#include "graph/hungarian.h"


/* -------------------------------------------------------------------------- */

  
/* -------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    Matrix* mp = NULL;
    Int_vector* row_vp = NULL; 
    double cost; 


    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }

    EPETE(read_matrix(&mp, NULL)); 

    EPETE(jv_lap(mp, &row_vp, &cost));
    
    pso("cost: %.3e\n\n");
    write_col_int_vector(row_vp, NULL);
    
    free_matrix(mp);
    free_int_vector(row_vp); 

    return EXIT_SUCCESS; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

