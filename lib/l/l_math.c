
/* $Id: l_math.c 21520 2017-07-22 15:09:04Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#include "l/l_gen.h"     /* Only safe as first include in a ".c" file. */
#include "l/l_sys_rand.h"
#include "l/l_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
// FIX
//
// Needs to return a negative value if overflow.
*/

int ipower(int a, int b)
{
    int result, count;

    result =1;

    for (count=0;count<b;++count)
    {
        result *= a;
    }
     return result;
 }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int int_plus(int a, int b)
{

    return a+b;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
/*
// int int_compare(a, b, dummy)
//   int a, b;
//   int dummy;
// {
//
//     if (a > b) return 1;
//     else if (a < b) return -1;
//     else return 0;
// }
*/
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*ARGSUSED*/
int ptr_int_compare
(
    int* a_ptr,
    int* b_ptr,
    int  __attribute__((unused)) dummy_arg
)
{

    if (*a_ptr > *b_ptr) return 1;
    else if (*a_ptr < *b_ptr) return -1;
    else return 0;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int min3(int a, int b, int c)
{

    if (a <= b)
    {
        if (a <= c) return a;
        else return c;
    }
    else
    {
        if (b <= c) return b;
        else return c;
    }

   /* Some compilers think we can get here! Keep them happy. */

#ifdef NeXT
#ifdef __GNUC__
    return 0;
#endif
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int max3(int a, int b, int c)
{

    if (a >= b)
    {
        if (a >= c) return a;
        else return c;
    }
    else
    {
        if (b >= c) return b;
        else return c;
    }

    /* Some compilers think we can get here! Keep them happy. */

#ifdef NeXT
#ifdef __GNUC__
    return 0;
#endif
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*VARARGS*/
int pos_min(int num_args, ...)
{
    va_list ap;
    int cur_arg;
    int min;
    int i;


    va_start(ap, num_args);

    min = 0;

    for (i=0; i<num_args; ++i)
    {
        cur_arg = (int)va_arg(ap, int);

        if (cur_arg > 0)
        {
            if (min == 0) min = cur_arg;
            else if (cur_arg < min) min = cur_arg;
        }
    }

    va_end(ap);

    return min;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_random_integer_list(int count, int min_value, int max_value,
                            int* output_array)
{
    int    i;
    int    j;
    int*   output_array_pos;
    int    range;
    double random_var;
    double temp;
    Bool   already_used;
    int    random_result;


    if (count > max_value-min_value+1)
    {
        set_error("Cannot choose %d different random values from %d.",
                  count, max_value - min_value + 1);
        return ERROR;
    }
    else if (count <= 0)
    {
        set_error("Number of values to choose must be one or greater.");
        return ERROR;
    }

    output_array_pos = output_array;

    for (i = 0; i < count; i++)
    {
        do
        {
            random_var =  kjb_rand();
            range = 1 + max_value - min_value;
            temp = range * random_var;
            temp += (double)min_value;

            /*
            // This breaks Sun's optimization. I'm not sure if it is my fault or
            // Sun's. On second thought, it seems that it may not be the best
            // approach anyway (we are just trying to ensure against some
            // random number generatory returning one). Instead, just make an
            // explicit check.
            //
            // temp = SUB_DBL_EPSILON(temp);
            */

            random_result = (int)temp;

            if (random_result > max_value)
            {
                /*
                // Pretend, OK?
                */
                already_used = TRUE;
            }
            else
            {
                already_used = FALSE;

                for (j=0; j<i; j++)
                {
                    if (output_array[ j] == random_result)
                    {
                        already_used = TRUE;
                        break;
                    }
                }
            }
        } while (already_used);

        *output_array_pos = random_result;
        output_array_pos++;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifndef M_LN2
#    define M_LN2     0.69314718055994530942  /* log_e 2 */
#endif

double kjb_log2(double x)
{
    return log(x)/M_LN2;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int kjb_floor(double x)
{
    double floor_x = floor(x); 
    int    int_x = (int)floor_x; 

    return int_x;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             kjb_rint
 *
 * Rounds a double to an integer
 *
 * This routine rounds a double to an integer. If the double is too big or too
 * small, then the min or max integer is returned, with a bug condition being
 * set. 
 *
 * This routine wraps rint() if it is available; otherwise it computes the
 * answer.
 *
 * Returns:
 *    The integer corresponding to the double by standard rounding rules. 
 *
 * Index: 
 *    Math
 *
 * -----------------------------------------------------------------------------
*/

int kjb_rint(double x)
{
    /*
     * Pulling this off with a macro is too complex. The hard part is to
     * implement rounding x.5 either up or down in the standard way (to avoid
     * statistical bias). 
    */
#ifdef TEST
    if (x < (double)(INT_MIN))
    {
        SET_UNDERFLOW_BUG();
        return INT_MIN;
    }
    else if (x > (double)(INT_MAX))
    {
        SET_OVERFLOW_BUG();
        return INT_MAX;
    }
#endif 
     
#ifdef HAVE_RINT
    return (int)rint(x); 
#else
    if (x > 0.0)
    {
        double y = x + 0.5;
        int    i = (int)y;

        if ((double)i == y)
        {
            if (IS_ODD(i))
            {
                i--;
            }
        }
        return i;
    }
    else if (x < 0.0)
    {
        double y = x - 0.5;
        int    i = (int)y;

        if ((double)i == y)
        {
            if (IS_ODD(i))
            {
                i++;
            }
        }
        return i;
    }
    else 
    {
        return 0;
    }

#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             kjb_rintf
 *
 * Rounds a float to an integer
 *
 * This routine rounds a double to an integer. If the float is too big or too
 * small, then the min or max integer is returned, with a bug condition being
 * set. 
 *
 * This routine wraps rintf() if it is available; otherwise it computes the
 * answer.
 *
 * Returns:
 *    The integer corresponding to the float by standard rounding rules. 
 *
 * Index: 
 *    Math
 *
 * -----------------------------------------------------------------------------
*/

int kjb_rintf(float x)
{
    /*
     * Pulling this off with a macro is too complex. The hard part is to
     * implement rounding x.5 either up or down in the standard way (to avoid
     * statisitcal bias). 
    */
#ifdef TEST
    if (x < (double)(INT_MIN))
    {
        SET_UNDERFLOW_BUG();
        return INT_MIN;
    }
    else if (x > (double)(INT_MAX))
    {
        SET_OVERFLOW_BUG();
        return INT_MAX;
    }
#endif 
     
#ifdef HAVE_RINTF
    return (int)rintf(x); 
#else
    if (x > 0.0f)
    {
        float y = x + 0.5f;
        int   i = (int)y;

        if ((float)i == y)
        {
            if (IS_ODD(i))
            {
                i--;
            }
        }
        return i;
    }
    else if (x < 0.0)
    {
        float y = x - 0.5;
        int   i = (int)y;

        if ((float)i == y)
        {
            if (IS_ODD(i))
            {
                i++;
            }
        }
        return i;
    }
    else 
    {
        return 0;
    }

#endif 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif



