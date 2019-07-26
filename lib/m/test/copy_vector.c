
/* $Id: copy_vector.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "m/m_incl.h" 

#define MAX_LENGTH  100000
#define BASE_NUM_TRIES 1000

/*ARGSUSED*/
int main(int argc, char **argv)
{
    Vector* vp = NULL;
    Vector* vp2 = NULL;
    int i;
    long memcpy_cpu = 0;
    long std_cpy_cpu   = 0; 
    double diff; 
    int  num_tries = BASE_NUM_TRIES;
    int  test_factor = 1;
    int status = EXIT_SUCCESS;


    kjb_init(); 

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 
    }

    if (test_factor <= 0)
    {
        num_tries = 1;
    }
    else
    {
        num_tries *= test_factor;
    } 


    kjb_l_set("debug", "2");
    kjb_l_set("page", "off");

    for (i = 0; i < num_tries; i++)
    {
        int p; 
        int length = kjb_rint(1 + MAX_LENGTH * kjb_rand());

        EPETE(get_random_vector(&vp, length));

        kjb_l_set("use-memcpy", "f");

        p = kjb_rint((double)length * kjb_rand());
        p = MIN_OF(length - 1, p);

        vp->elements[ p ] = i; 

        init_cpu_time();
        EPETE(copy_vector(&vp2, vp));
        std_cpy_cpu += get_cpu_time(); 

        diff = max_abs_vector_difference(vp, vp2);

        if (diff > DBL_MIN) 
        {
            set_error("Max vector diff (%.3e) is not zero.", diff);
            status = EXIT_BUG;
        }

        kjb_l_set("use-memcpy", "t");

        p = kjb_rint((double)length * kjb_rand());
        p = MIN_OF(length - 1, p);

        vp->elements[ p ] = i; 

        init_cpu_time();
        EPETE(copy_vector(&vp2, vp));
        memcpy_cpu += get_cpu_time(); 

        diff = max_abs_vector_difference(vp, vp2);

        if (diff > DBL_MIN) 
        {
            set_error("Max vector diff (%.3e) is not zero.", diff);
            status = EXIT_BUG;
        }

    }

    if (is_interactive())
    {
        dbi(std_cpy_cpu);
        dbi(memcpy_cpu);
    }

    free_vector(vp); 
    free_vector(vp2); 

    return status; 
}

