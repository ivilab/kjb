
/* $Id: int_vector.c 21491 2017-07-20 13:19:02Z kobus $ */


#define BASE_NUM_TRIES  100
#define MAX_LEN         1000000

#include "l/l_incl.h" 

int main(int argc, char **argv)
{
    int    try_count, i;
    int    test_factor = 1; 
    int num_tries = BASE_NUM_TRIES; 
    Int_vector* vp = NULL; 
    int prev_len;


    kjb_init();

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 

        if (test_factor == 0) 
        {
            num_tries = 10;
        }
        else
        {
            num_tries *= test_factor;
        }
    }

    if (is_interactive())
    {
        kjb_set_debug_level(2); 
    }
    else 
    {
        kjb_set_debug_level(0); 
    }

    prev_len = 100; 
    EPETE(ra_get_target_int_vector(&vp, prev_len));

    for (i = 0; i < prev_len; i++)
    {
        vp->elements[ i ] = i;
    }

    for (try_count = 0; try_count< num_tries; try_count++)
    {
        int len = MIN_OF(MAX_LEN - 1, 2.0 + (kjb_rand() * (double)MAX_LEN));

        if (try_count % 5 == 0)
        {
            len += MIN_OF(try_count, 100) * MAX_LEN;
        }

        dbx(vp->elements); 

        EPETE(ra_get_target_int_vector(&vp, len));

        for (i = 0; i < MIN_OF(len, prev_len); i++)
        {
            if (vp->elements[ i ] != try_count + i)
            {
                p_stderr("Preservation condition failed.\n");
                return EXIT_BUG;
            }
        }

        for (i = 0; i < len; i++)
        {
            vp->elements[ i ] = 1 + try_count + i;
        }

        prev_len = len;
    }

    free_int_vector(vp);

    kjb_cleanup();
    
    return EXIT_SUCCESS;
}

