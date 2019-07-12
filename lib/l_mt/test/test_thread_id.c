/*
 * How well does a thread's internal "sense" of when it was born align with
 * the order in which calls to pthread_create are initiated?
 * Sometimes I have seen them go in lockstep over and over again.
 * Other times, there is a discrepancy I can measure but I cannot explain.
 * This program exists to try to smoke out any discrepancy.
 *
 * $Id: test_thread_id.c 21491 2017-07-20 13:19:02Z kobus $
 */

#include "l/l_sys_io.h"
#include "l/l_sys_debug.h"
#include "l/l_debug.h"
#include "l/l_global.h"
#include "l/l_error.h"
#include "l/l_int_vector.h"
#include "l_mt/l_mt_pthread.h"

#define WASTE_TIME 1

#if WASTE_TIME
#include <m/m_convolve.h>
#endif


#define TEST_REPS 100 /* you might want more */
#define THREAD_CT 30

struct Thread_pack
{
    int creation_order;
    int* thread_number;
#if WASTE_TIME
    const Matrix *kern, *mat_in;
    Matrix* mat_out;
#endif
};

static void* workfun(void* v)
{
    int n;
    struct Thread_pack* tp = (struct Thread_pack*) v;
    NRN(tp);

#if WASTE_TIME
    ERN(convolve_matrix(& tp -> mat_out, tp -> mat_in, tp -> kern));
#endif

    * tp -> thread_number = n = get_kjb_pthread_number();
    return n == tp -> creation_order ? v : NULL;
}

static int test_id(Int_vector* sum)
{
    int i, j;
    kjb_pthread_t tids[THREAD_CT];
    int result = ERROR;
    struct Thread_pack tp[THREAD_CT];
#if WASTE_TIME
    Matrix *busywork[2*THREAD_CT], *kernel = NULL;
#endif
    Int_vector* pn = NULL;

    NRE(sum);
    ASSERT(sum -> length == THREAD_CT);
    ERE(get_target_int_vector(&pn, THREAD_CT));

#if WASTE_TIME
    /* do some useless convolution */
    EPETE(get_random_matrix(&kernel, 15, 15));
    for (j = 0; j < 2 * THREAD_CT; ++j)
    {
        busywork[j] = NULL;
    }
    for (i = j = 0; j < THREAD_CT; ++j)
    {
        EGC(get_random_matrix(busywork + j, 100, 100));
        EGC(get_target_matrix(busywork + j + THREAD_CT, 100, 100));
    }
#endif

    /* each thread queries its serial number, and returns it. */
    for (i = 0; i < THREAD_CT; ++i)
    {
        tp[i].creation_order = 1+i;
        tp[i].thread_number = pn -> elements + i;
#if WASTE_TIME
        tp[i].mat_in = busywork[i];
        tp[i].mat_out = busywork[i + THREAD_CT];
        tp[i].kern = kernel;
#endif
        EGC(kjb_pthread_create(tids + i, NULL, &workfun, tp + i));
    }

    /* does the returned serial number match the calling order? */
    ASSERT(THREAD_CT == i);
    while (i > 0)
    {
        void *v;
        EGC(kjb_pthread_join(tids[i-1], &v));
        --i;
        NGC(v);
        if (1 + i != pn->elements[i])
        {
            TEST_PSE(("thread %d said it was # %d.\n", i, pn->elements[i]));
            sum -> elements[i] += 1;
        }
    }
    result = NO_ERROR;

cleanup:
    while (i > 0) EPE(kjb_pthread_cancel(tids[--i]));
#if WASTE_TIME
    for (j = 0; j < 2 * THREAD_CT; ++j)
    {
        free_matrix(busywork[j]);
    }
    free_matrix(kernel);
#endif
    free_int_vector(pn);
    return result;
}


int main(void)
{
    int i, sumsum;
    int result = EXIT_SUCCESS;
    Int_vector* sumv = NULL;

    EPETE(get_initialized_int_vector(&sumv, THREAD_CT, 0));

    for (i = 0; i < TEST_REPS; ++i, reset_kjb_pthread_counter())
    {
        EPETE(test_id(sumv));
    }

    if ((sumsum = sum_int_vector_elements(sumv)))
    {
        EPETE(write_row_int_vector(sumv, NULL));
        kjb_printf("Total failures: %d\n", sumsum);
        result = EXIT_BUG;
    }

    free_int_vector(sumv);

    if (is_interactive())
    {
        kjb_puts( EXIT_SUCCESS==result ? "Success\n" : "Failure\n" );
    }
    return result;
}

