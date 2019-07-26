/*
 * Are the random number generators actually repeatable?  
 * 
 * $Id: basic_rand.c 22407 2019-04-14 20:21:31Z kobus $
 */

#include <l/l_sys_lib.h>
#include <l/l_sys_io.h>
#include <l/l_sys_rand.h>

#define SIZE 1000

double rand1early[SIZE], rand2early[SIZE], rand1late[SIZE], rand2late[SIZE];

static int check_for_errors(void);

int check_for_errors(void)
{
    int i;
    int first_error_1 = INT_MAX, first_error_2 = INT_MAX;
    int err_count_1 = 0, err_count_2 = 0;
    int result = EXIT_SUCCESS;

    for (i = 0; i < SIZE; ++i)
    {
        if (rand1early[i] != rand1late[i])
        {
            result = EXIT_BUG;
            first_error_1 = MIN_OF(first_error_1, i);
            err_count_1 += 1;
        }

        if (rand2early[i] != rand2late[i])
        {
            result = EXIT_BUG;
            first_error_2 = MIN_OF(first_error_2, i);
            err_count_2 += 1;
        }
    }

    if (is_interactive())
    {
        if (err_count_1)
        {
            pso("Stream 1:  %d of %d values differ, "
                    "first difference at index %d.\n",
                    err_count_1, SIZE, first_error_1);
        }
        if (err_count_2)
        {
            pso("Stream 2:  %d of %d values differ, "
                    "first difference at index %d.\n",
                    err_count_2, SIZE, first_error_2);
        }
        if (0 == err_count_1 && 0 == err_count_2)
        {
            kjb_puts("Success!\n");
        }
    }

    return result;
}

int main(void)
{
    int i;

    for (i = 0; i < SIZE; ++i)
    {
        rand1early[i] = kjb_rand();
        rand2early[i] = kjb_rand_2();
    }

    kjb_seed_rand(0, 0);
    kjb_seed_rand_2(0l);

    for (i = 0; i < SIZE; ++i)
    {
        rand1late[i] = kjb_rand();
        rand2late[i] = kjb_rand_2();
    }

    return check_for_errors();
}
