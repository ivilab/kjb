
/* $Id: welch_t.c 21491 2017-07-20 13:19:02Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003, by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               |
|        Kobus Barnard                                                         |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */


#include "m/m_incl.h"
#include "stat/stat_incl.h"


/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


int main(void)
{
    double mean_1      = 10.0;
    double mean_2      = 20.0;
    double var_1       = 5.0;
    double var_2       = 3.0;
    int    count_1     = 10;
    int    count_2     = 20;
    double t_prime;
    double effect_size;
    double p_val;
    int    df;


    kjb_init();

    EPETE(welch_t_test_one_sided(mean_1, mean_2, var_1, var_2, count_1, count_2, 
                                 &t_prime, &df, &effect_size, &p_val));

    pso("Welch t statistic is %.3e, df is %d, effect size is: %.3f, p-value is %.2e\n\n", 
        t_prime, df, effect_size, p_val);

    kjb_exit(EXIT_SUCCESS); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

