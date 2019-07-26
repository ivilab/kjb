
/* $Id: sort_vector.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h" 

#define BASE_NUM_TRIES   1000
#define BASE_LENGTH      10000

/*ARGSUSED*/
int main(int argc, char **argv)
{
    int status = EXIT_SUCCESS;
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    Vector* vp = NULL;
    int try_count, i; 


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor == 0)
    {
        num_tries = 1;
    }
    else
    {
        num_tries *= test_factor;
    }

    for (try_count = 0; try_count < num_tries; try_count++)
    {
        int length = 1 + kjb_rint(1.0 + BASE_LENGTH * kjb_rand());

        EPETE(get_random_vector(&vp, length));
        EPETE(ascend_sort_vector(vp));

        for (i = 0; i < length - 1; i++)
        {
            if (vp->elements[ i ] > vp->elements[ i + 1 ])
            {
                p_stderr("Out of order elements.\n");
                status = EXIT_BUG;
            }
        }


        /*
         *  No descend_sort_vector() yet. 
         *

        EPETE(get_random_vector(&vp, length));
        EPETE(descend_sort_vector(vp));

        for (i = 0; i < length - 1; i++)
        {
            if (vp->elements[ i ] < vp->elements[ i + 1 ])
            {
                p_stderr("Out of order elements.\n");
                status = EXIT_BUG;
            }
        }

        */
    }

    free_vector(vp);

    return status; 
}

