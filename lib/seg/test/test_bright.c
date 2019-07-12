/**
 * @file
 * @brief unit test for function find_bright_spots_in_image().
 * @author Andrew Predoehl
 */
/*
 * $Id: test_bright.c 21491 2017-07-20 13:19:02Z kobus $
 */

#include "l/l_sys_std.h"
#include "l/l_sort.h"
#include "l/l_init.h"
#include "l/l_int_mat_io.h"
#include "m/m_matrix.h"
#include "m/m_vec_io.h"
#include "i/i_float.h"
#include "i/i_float_io.h"
#include "seg/seg_spots.h"

#define SIZE 100 /* [pix] edge length of test image */
#define SPOT 6   /* [pix] edge length of square test spot */

static int VERBOSE = 0;

static int crunch(int a, int b)
{
    return a << 8  |  b;
}

static int int_greater(const void* v1, const void* v2)
{
    /* Kobus: 17/07/11. How this used to be (commented out below) is wrong! */
    return * (const int*)v1 - * (const int*)v2;
    /* return * (const int*)v1 > * (const int*)v2;  */
}

static int check_answers(
    const Int_vector *c1ref,
    const Int_vector *c2ref,
    const Int_matrix_vector *roster,
    const Vector_vector *centroids,
    int try_again
)
{
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    int match = 1;
    int return_val = ERROR;
    Int_vector *c1test = NULL, *c2test = NULL;
    int i;

    ASSERT(c1ref);
    ASSERT(c2ref);
    NRE(roster);
    NRE(centroids);

    ERE(get_target_int_vector(&c1test, c1ref -> length));
    EGC(get_target_int_vector(&c2test, c2ref -> length));

    if (roster -> length != 2)
    {
        pso("Number of components should be 2, was %d\n", roster -> length);
        goto cleanup;
    }
    if (centroids -> length != 2)
    {
        pso("Wrong number of centroids, should be 2, was %d\n",
                centroids -> length);
        goto cleanup;
    }
    if (roster -> elements[0] -> num_cols != 2)
    {
        pso("Malformed component 1 roster");
        goto cleanup;
    }
    if (roster -> elements[1] -> num_cols != 2)
    {
        pso("Malformed component 2 roster");
        goto cleanup;
    }
    if (roster -> elements[0] -> num_rows != c1ref -> length)
    {
        pso("Wrong number of component pixels, should be %u, was %u\n",
                roster -> elements[0] -> num_rows, c1ref -> length);
        goto cleanup;
    }
    if (roster -> elements[1] -> num_rows != c2ref -> length)
    {
        pso("Wrong number of component pixels, should be %u, was %u\n",
                roster -> elements[1] -> num_rows, c2ref -> length);
        goto cleanup;
    }
    for (i = 0; i < c1ref -> length; ++i)
    {
        /*       MatVec     Matrix 0        RowVec i   */
        int *r = roster -> elements[0] -> elements[i];
        x1 += r[0]; /* column 0 */
        y1 += r[1]; /* column 1 */
        c1test -> elements[i] = crunch(r[1], r[0]); /* x,y order thus col,row*/
    }
    for (i = 0; i < c2ref -> length; ++i)
    {
        int *r = roster -> elements[1] -> elements[i];
        x2 += r[0];
        y2 += r[1];
        c2test -> elements[i] = crunch(r[1], r[0]); /* x,y order thus col,row*/
    }

    /* sort c1test, c2test */

    EGC(kjb_sort(c1test -> elements, c1ref -> length, sizeof(int), int_greater,
                 USE_CURRENT_ATN_HANDLING));
    
    EGC(kjb_sort(c1ref -> elements, c1ref -> length, sizeof(int), int_greater,
                 USE_CURRENT_ATN_HANDLING));
    EGC(kjb_sort(c2test -> elements, c2ref -> length, sizeof(int), int_greater,
                 USE_CURRENT_ATN_HANDLING));
    EGC(kjb_sort(c2ref -> elements, c2ref -> length, sizeof(int), int_greater,
                 USE_CURRENT_ATN_HANDLING));

    /* compare c1test with c1ref, c2test with c2ref */
    for (i = 0; match && i < c1ref -> length; ++i)
    {
        if (c1ref -> elements[i] != c1test -> elements[i])
        {
            TEST_PSE(("bad component 1 match\n"));
            match = 0;
        }
    }
    for (i = 0; match && i < c2ref -> length; ++i)
    {
        if (c2ref -> elements[i] != c2test -> elements[i])
        {
            TEST_PSE(("bad component 2 match\n"));
            match = 0;
        }
    }
    /*
    TEST_PSE(("centroid 1 x,y = %f,%f; Nx,Ny = %f,%f.\n"
                "Test sum Sx,Sy = %f,%f\n",
                centroids -> elements[0] -> elements[0],
                centroids -> elements[0] -> elements[1],
                centroids -> elements[0] -> elements[0] * c1ref -> length,
                centroids -> elements[0] -> elements[1] * c1ref -> length,
                x1, y1));
    */
    if (centroids-> elements[0]-> elements[0] * c1ref-> length != x1) match=0;
    if (centroids-> elements[0]-> elements[1] * c1ref-> length != y1) match=0;
    if (centroids-> elements[1]-> elements[0] * c2ref-> length != x2) match=0;
    if (centroids-> elements[1]-> elements[1] * c2ref-> length != y2) match=0;

    if (match)
    {
        return_val = NO_ERROR;
    }
    else if (try_again)
    {
        /* If the ref and test sets mismatched, we might swap and try again. */
        free_int_vector(c2test);
        free_int_vector(c1test);
        return check_answers(c2ref, c1ref, roster, centroids, 0);
    }

cleanup:
    free_int_vector(c2test);
    free_int_vector(c1test);
    return return_val;
}


int main (void)
{
    int r, c;

    Pixel w = { 100, 100, 100, { {0,0,0, VALID_PIXEL } } },
          g = { 50, 50, 50, { {0,0,0, VALID_PIXEL} } };

    KJB_image *i = NULL;
    Matrix *b = NULL, *t = NULL;
    Int_matrix_vector *roster = NULL;
    Vector_vector *ctrs = NULL;
    Int_vector *c1ref = NULL, *c2ref = NULL;

    EPETE(kjb_init());
    EPETE(get_initialized_image_2(&i, SIZE,SIZE, 0,0,0));

    EPETE(get_target_int_vector(&c1ref, SPOT*SPOT));
    EPETE(get_target_int_vector(&c2ref, SPOT*SPOT));

    /* Build test image, with square lighter zones. */
    for (r = 0; r < 4; ++r)
    {
        for (c = 0; c < 4; ++c)
        {
            i -> pixels[20 + r][50 + c] = w; /* too small */
        }
    }

    /* Two of these spots are just right. */
    for (r = 0; r < SPOT; ++r)
    {
        for (c = 0; c < SPOT; ++c)
        {
            int c1r = 40 + r, c1c = 20 + c, c2c = 80 + c;

            i -> pixels[c1r][c1c] = w;
            c1ref -> elements[r*SPOT+c] = crunch(c1r, c1c);

            i -> pixels[c1r][c2c] = w;
            c2ref -> elements[r*SPOT+c] = crunch(c1r, c2c);

            i -> pixels[c1r][50 + c] = g; /* too faint */
        }
    }

    for (r = 0; r < 8; ++r)
    {
        for (c = 0; c < 8; ++c)
        {
            i -> pixels[60 + r][50 + c] = w; /* too big */
        }
    }

    if (VERBOSE) { EPETE(kjb_write_image(i, "debug_spy_1.tif")); }

    /* Prepare background, threshold matrix and call function to test. */
    EPETE(get_zero_matrix(&b, SIZE, SIZE));
    EPETE(get_initialized_matrix(&t, SIZE, SIZE, 10));
    EPETE(find_bright_spots_in_image(i, b, t, 75, 30, 50, 0.9,
                                     &roster, &ctrs));

    /* You can print the hideous details if you like. */
    if (VERBOSE)
    {
        TEST_PSE(("roster size = %d\n"
                  "centroid size = %d\n"
                  "Centroids:\n", 
                  roster -> length, ctrs -> length));
        EPETE(write_vector_vector(ctrs, NULL));
        TEST_PSE(("Component pixel coordinates:\n"));
        for (r = 0; r < roster -> length; ++r)
        {
            TEST_PSE(("\tComponent %d:\n", r));
            EPETE(write_int_matrix(roster -> elements[r], NULL));
        }
    }

    /* Automatically verify the hideous details. We exit with EXIT_BUG if
     * check_answers returns ERROR.
    */
    EPETB(check_answers(c1ref, c2ref, roster, ctrs, 1));

    free_vector_vector(ctrs);
    free_int_matrix_vector(roster);
    free_matrix(b);
    free_matrix(t);
    free_int_vector(c2ref);
    free_int_vector(c1ref);
    kjb_free_image(i);
    kjb_cleanup();

    return EXIT_SUCCESS;
}

