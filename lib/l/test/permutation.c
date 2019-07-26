/*
 * $Id: permutation.c 21491 2017-07-20 13:19:02Z kobus $
 */

#include <l/l_sys_io.h>
#include <l/l_sys_lib.h>
#include <l/l_init.h>
#include <l/l_error.h>
#include <l/l_debug.h>
#include <l/l_global.h>
#include <l/l_int_vector.h>

int main(void)
{
    int r;
    Int_vector* p = NULL;
    char b[100];

    EPETE(kjb_init());
    EPETE(get_target_int_vector(&p, 10));

    p -> elements[0] = 2;
    p -> elements[1] = 6;
    p -> elements[2] = 7;
    p -> elements[3] = 5;
    p -> elements[4] = 1;
    p -> elements[5] = 9;
    p -> elements[6] = 4;
    p -> elements[7] = 0;
    p -> elements[8] = 8;
    p -> elements[9] = 3;

    /* p is a permutation now. */
    EPETE(int_vector_is_permutation(p, 0, &r));
    if (r != TRUE)
    {
        kjb_puts("Failure 1a\n");
        return EXIT_BUG;
    }

    EPETE(get_string_why_int_vector_is_not_permutation(p, 0, b, sizeof(b)));
    if (strlen(b) != 0)
    {
        kjb_puts("Failure 1b\n");
        return EXIT_BUG;
    }

    p -> elements[7] = 10;
    /* p is a NOT permutation now. */
    EPETE(int_vector_is_permutation(p, 0, &r));
    if (r != FALSE)
    {
        kjb_puts("Failure 2\n");
        return EXIT_BUG;
    }

    EPETE(get_string_why_int_vector_is_not_permutation(p, 0, b, sizeof(b)));
    if (0 == strlen(b))
    {
        kjb_puts("Failure 2b\n");
        return EXIT_BUG;
    }
    else if (is_interactive())
    {
        kjb_printf("Test 2 result:  something about out-of-range value 10.\n"
                   "Actual result: \"%s\"\n", b);
    }

    /* but, p is a permutation relative to a start_value of one. */
    EPETE(int_vector_is_permutation(p, 1, &r));
    if (r != TRUE)
    {
        kjb_puts("Failure 3a\n");
        return EXIT_BUG;
    }

    EPETE(get_string_why_int_vector_is_not_permutation(p, 1, b, sizeof(b)));
    if (strlen(b) != 0)
    {
        kjb_puts("Failure 3b\n");
        return EXIT_BUG;
    }

    p -> elements[9] = 9;
    /* p is a NOT permutation now. */
    EPETE(int_vector_is_permutation(p, 1, &r));
    if (r != FALSE)
    {
        kjb_puts("Failure 4a\n");
        return EXIT_BUG;
    }

    EPETE(get_string_why_int_vector_is_not_permutation(p, 1, b, sizeof(b)));
    if (0 == strlen(b))
    {
        kjb_puts("Failure 4b\n");
        return EXIT_BUG;
    }
    else if (is_interactive())
    {
        kjb_printf("Test 4 result:  something about missing value 3.\n"
                   "Actual result: \"%s\"\n", b);
    }

    p -> elements[5] = 3;
    /* p is again a permutation relative to a start_value of one. */
    EPETE(int_vector_is_permutation(p, 1, &r));
    if (r != TRUE)
    {
        kjb_puts("Failure 5a\n");
        return EXIT_BUG;
    }

    EPETE(get_string_why_int_vector_is_not_permutation(p, 1, b, sizeof(b)));
    if (strlen(b) != 0)
    {
        kjb_puts("Failure 5b\n");
        return EXIT_BUG;
    }

    /* bad inputs should provoke ERROR -- let's check. */
    if (int_vector_is_permutation(p, 0, NULL) != ERROR)
    {
        kjb_puts("Failure 6\n");
        return EXIT_BUG;
    }
    if (get_string_why_int_vector_is_not_permutation(p, 0, NULL, 17) != ERROR)
    {
        kjb_puts("Failure 7\n");
        return EXIT_BUG;
    }

    /* empty input is accepted as a trivial permutation */
    if (int_vector_is_permutation(NULL, 0, &r) != NO_ERROR || r != TRUE) 
    {
        kjb_puts("Failure 8\n");
        return EXIT_BUG;
    }
    if (get_string_why_int_vector_is_not_permutation(NULL, 0, b, 2) != NO_ERROR
        || strlen(b) != 0
       )
    {
        kjb_puts("Failure 9\n");
        return EXIT_BUG;
    }

    free_int_vector(p);
    kjb_cleanup();
    return EXIT_SUCCESS;
}
