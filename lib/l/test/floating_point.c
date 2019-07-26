
/* $Id: floating_point.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "l/l_incl.h" 

#define CHECK_TRUE(X) \
\
    if ( ! (X) ) \
    { \
        p_stderr("FALIED: "#X" is true.\n");  \
        failure = TRUE; \
    } \
    else if (is_interactive()) \
    { \
        p_stderr("Passed: "#X" is true.\n");  \
    }



int main(int argc, char **argv)
{
#ifdef DEF_OUT
    int    i;
#endif 
    int    test_factor = 1; 
    int    failure = FALSE;


    kjb_init();

    if (argc > 1)
    {
        EPETE(ss1pi(argv[ 1 ], &test_factor)); 

        if (test_factor == 0) 
        {
            test_factor = 1;
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

#ifdef MACHINE_CONSTANT_NOT_AVAILABLE
    p_stderr("Some machine constant is not available on this system.\n");
    failure = TRUE;
#else
    p_stderr("\nPassed: MACHINE_CONSTANT_NOT_AVAILABLE is not defined.\n\n");
#endif 

    CHECK_TRUE(IS_EQUAL_FLT(FLT_ZERO, FLT_ZERO));
    CHECK_TRUE(IS_NEARLY_EQUAL_FLT(FLT_ZERO, FLT_ZERO, FLT_EPSILON));
    CHECK_TRUE(IS_NEARLY_EQUAL_FLT(FLT_ZERO, FLT_ZERO, FLT_ZERO));

    CHECK_TRUE(IS_EQUAL_DBL(0.0, 0.0));
    CHECK_TRUE(IS_ZERO_DBL(0.0));
    CHECK_TRUE(IS_ZERO_DBL(DBL_MIN));
    CHECK_TRUE(IS_ZERO_DBL(-DBL_MIN));
    CHECK_TRUE( ! IS_ZERO_DBL(1.0));
    CHECK_TRUE( ! IS_ZERO_DBL(DBL_EPSILON / 10.0));
    CHECK_TRUE( ! IS_ZERO_DBL(DBL_MIN * 10.0));
    CHECK_TRUE(IS_NEARLY_EQUAL_DBL(0.0, 0.0, DBL_EPSILON));
    CHECK_TRUE(IS_NEARLY_EQUAL_DBL(0.0, 0.0, 0.0));

    CHECK_TRUE(! IS_EQUAL_DBL(1.0, 2.0));
    CHECK_TRUE(! IS_EQUAL_DBL(1.0, 1.00001));
    CHECK_TRUE(! IS_EQUAL_DBL(1.0, 1.0 + 10.0 * DBL_EPSILON));

    CHECK_TRUE(1e-15 > DBL_EPSILON); 
    CHECK_TRUE(0.0 < DBL_EPSILON); 
    CHECK_TRUE(1e-300 > DBL_MIN); 
    CHECK_TRUE(1e+300 < DBL_MAX); 

    CHECK_TRUE(SUB_RELATIVE_DBL(1.0, 2*DBL_EPSILON) < 1.0);
    CHECK_TRUE(SUB_RELATIVE_DBL(1.0, 2*DBL_EPSILON) > 1.0 - 5*DBL_EPSILON);
    CHECK_TRUE(ADD_RELATIVE_DBL(1.0, 2*DBL_EPSILON) > 1.0);
    CHECK_TRUE(ADD_RELATIVE_DBL(1.0, 2*DBL_EPSILON) < 1.0 + 5*DBL_EPSILON);

#ifdef DEF_OUT
    for (i = -test_factor * 1000; i< test_factor * 1000; i++)
    {
    }
#endif 

    kjb_cleanup();
    
    if (failure)
    {
        return EXIT_BUG;
    }
    else 
    {
        return EXIT_SUCCESS;
    }
}

