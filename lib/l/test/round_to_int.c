
/* $Id: round_to_int.c 21491 2017-07-20 13:19:02Z kobus $ */


#define BASE_NUM_TRIES  3000

#include "l/l_incl.h" 

int main(int argc, char **argv)
{
    double dx;
    float  fx;
    int    i, j;
    int    ri, kri, dri, dkri; 
    int    test_factor = 1; 
    IMPORT int kjb_debug_level;
    int result = EXIT_SUCCESS;
    int num_tries = BASE_NUM_TRIES; 


    kjb_init();

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 

        if (test_factor == 0) 
        {
            num_tries = 100;
        }
        else
        {
            num_tries *= test_factor;
        }
    }

    if (is_interactive())
    {
        kjb_debug_level = 2;
    }
    else 
    {
        kjb_debug_level = 0; 
    }

    for (i = -num_tries; i< num_tries; i++)
    {
        dbi(i); 

        dx = (double)i + 0.5;
        ri = rint(dx);
        kri = kjb_rint(dx);

        if (kri != ri)
        {
            set_error("Test 1: Problem rounding double %.3e (%d != %d).", 
                      dx, kri, ri);
            kjb_print_error();
            result = EXIT_BUG;
        }

        dx = (double)i - 0.5;
        ri = rint(dx);
        kri = kjb_rint(dx);

        if (kri != ri)
        {
            set_error("Test 2: Problem rounding double %.3e (%d != %d).", 
                      dx, kri, ri);
            kjb_print_error();
            result = EXIT_BUG;
        }

        for (j = 0; j < num_tries; j++)
        {
            dx = (double)i + kjb_rand();
            ri = rint(dx);
            kri = kjb_rint(dx);

            if (kri != ri)
            {
                set_error("Test 3: Problem rounding double %.3e (%d != %d).", 
                          dx, kri, ri);
                kjb_print_error();
                result = EXIT_BUG;
            }
        }

#ifdef __GNUC__

        /*
         * Not all systems have rintf(). GNU systems generally have it. Test
         * kjb_rintf() on those systems for now. 
        */

        fx = (float)i + 0.5f;
        ri = rintf(fx);
        kri = kjb_rintf(fx);
        dri = rint((double)fx);
        dkri = kjb_rint((double)fx); 

        if (kri != ri)
        {
            set_error("Test 4: Problem rounding float %.3e (%d != %d)    [ Double results are %d and %d ].", 
                      (double)fx, kri, ri, dkri, dri);
            kjb_print_error();
            result = EXIT_BUG;
        }

        fx = (float)i - 0.5f;
        ri = rintf(fx);
        kri = kjb_rintf(fx);
        dri = rint((double)fx);
        dkri = kjb_rint((double)fx); 

        if (kri != ri)
        {
            set_error("Test 5: Problem rounding float %.3e (%d != %d)    [ Double results are %d and %d ].", 
                      (double)fx, kri, ri, dkri, dri);
            kjb_print_error();
            result = EXIT_BUG;
        }

        for (j = 0; j < num_tries; j++)
        {
            fx = (float)i + (float)kjb_rand();
            ri = rintf(fx);
            kri = kjb_rintf(fx);
            dri = rint((double)fx);
            dkri = kjb_rint((double)fx); 

            if (kri != ri)
            {
                set_error("Test 6: Problem rounding float %.3e (%d != %d)    [ Double results are %d and %d ].", 
                      (double)fx, kri, ri, dkri, dri);
                kjb_print_error();
                result = EXIT_BUG;
            }
        }
#endif 
    }

    kjb_cleanup();
    
    return result;
}

