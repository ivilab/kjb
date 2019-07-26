
/* $Id: slatec_machine_constants.c 21491 2017-07-20 13:19:02Z kobus $ */


#include "wrap_slatec/wrap_slatec.h"

#define RELATIVE_TOLERENCE  (2.0 * DBL_EPSILON)   

/*ARGSUSED*/
int main(int argc, char *argv[])
{
    int i;
    double dbl_machine_constant;
    int int_machine_constant; 
    int failed = FALSE;

    kjb_init(); 

    kjb_set_debug_level(1);

    if (is_interactive())
    {
        kjb_set_verbose_level(1);
    }
    else
    {
        kjb_set_verbose_level(0);
    }

    i = 1;
    EPETE(slatec_d1mach(&dbl_machine_constant, i));
    verbose_pso(1, "Test: %d\n", i); 
    verbose_pso(1, "This should be smallest positive magnitude (%.10e): %.10e.\n",
                dbl_machine_constant, DBL_MIN); 

    /*
     * It does not necessarily need to be this way, but if it is not, we should
     * check it and adjust this test if it is OK.  
    */
    if ( ! IS_NEARLY_EQUAL_DBL(dbl_machine_constant, DBL_MIN, RELATIVE_TOLERENCE))
    {
        p_stderr("Test %d  FAILED   !!!! \n", i); 
        dbe(dbl_machine_constant);
        dbe(DBL_MIN);
        dbe(dbl_machine_constant - DBL_MIN); 
        dbe(ABS_OF(dbl_machine_constant - DBL_MIN) / ABS_OF(DBL_MIN)); 
        dbe(DBL_EPSILON);
        dbe((ABS_OF(dbl_machine_constant - DBL_MIN) / DBL_MIN) / DBL_EPSILON); 
        dbe(ADD_RELATIVE_DBL(dbl_machine_constant, DBL_EPSILON) - DBL_MIN);
        dbe(ADD_RELATIVE_DBL(DBL_MIN, DBL_EPSILON) - dbl_machine_constant);
        
        failed = TRUE;
    }

    if (kjb_get_verbose_level() > 0)
    {
        verbose_pso(1, "In hex: "); 
        hex_print(stdout, &dbl_machine_constant, 8); 
        verbose_pso(1, "\n--------------------------\n\n"); 
    }


    i = 2;
    EPETE(slatec_d1mach(&dbl_machine_constant, i));
    verbose_pso(1, "Test: %d\n", i); 
    verbose_pso(1, "This should be bigggest magnitude (%.10e): %.10e.\n",
                DBL_MOST_POSITIVE, dbl_machine_constant); 

    /* It does not necessarily need to be this way, but if it is not, we should
     * check it and adjust this test if it is OK.  
    */
    if ( ! IS_NEARLY_EQUAL_DBL(dbl_machine_constant, DBL_MOST_POSITIVE, RELATIVE_TOLERENCE))
    {
        p_stderr("Test %d  FAILED   !!!! \n", i); 
        dbe(dbl_machine_constant - DBL_MOST_POSITIVE); 
        dbe(ABS_OF(dbl_machine_constant - DBL_MOST_POSITIVE) / ABS_OF(DBL_MOST_POSITIVE)); 
        dbe(DBL_EPSILON);
        dbe((ABS_OF(dbl_machine_constant - DBL_MOST_POSITIVE) / DBL_MOST_POSITIVE) / DBL_EPSILON); 
        dbe(DBL_EPSILON);
        
        failed = TRUE;
    }

    if (kjb_get_verbose_level() > 0)
    {
        verbose_pso(1, "In hex: "); 
        hex_print(stdout, &dbl_machine_constant, 8); 
        verbose_pso(1, "\n--------------------------\n\n"); 
    }

    i = 3;
    verbose_pso(1, "Test: %d\n", i); 
    EPETE(slatec_d1mach(&dbl_machine_constant, i));
    verbose_pso(1, "Smallest relative spacing is: %.10e.\n",
                dbl_machine_constant); 
    verbose_pso(1, "Assuming that the floating point base is 2, this is 1/2 DBL_EPSILON (%.10e).\n",
                DBL_EPSILON / 2.0); 

    /* It does not necessarily need to be this way, but if it is not, we should
     * check it and adjust this test if it is OK.  
    */
    if ( ! IS_EQUAL_DBL(dbl_machine_constant, DBL_EPSILON / 2.0))
    {
        dbp("!!!! FAILED !!!!"); 
        dbw();
        dbi(i);
        dbe(dbl_machine_constant - DBL_EPSILON / 2.0); 
        dbe(ADD_RELATIVE_DBL(dbl_machine_constant, DBL_EPSILON) - DBL_EPSILON / 2.0);
        dbe(ADD_RELATIVE_DBL(DBL_EPSILON / 2.0, DBL_EPSILON) - dbl_machine_constant);
        
        failed = TRUE;
    }

    if (kjb_get_verbose_level() > 0)
    {
        verbose_pso(1, "In hex: "); 
        hex_print(stdout, &dbl_machine_constant, 8); 
        verbose_pso(1, "\n--------------------------\n\n"); 
    }

    i = 4;
    verbose_pso(1, "Test: %d\n", i); 
    EPETE(slatec_d1mach(&dbl_machine_constant, i));
    verbose_pso(1, "Largest relative spacing is %.10e. Should be DBL_EPSILON (%.10e).\n",
                dbl_machine_constant, DBL_EPSILON); 

    /* It does not necessarily need to be this way, but if it is not, we should
     * check it and adjust this test if it is OK.  
    */
    if ( ! IS_EQUAL_DBL(dbl_machine_constant, DBL_EPSILON))
    {
        dbp("!!!! FAILED !!!!"); 
        dbw();
        dbi(i);
        dbe(dbl_machine_constant - DBL_EPSILON); 
        dbe(ADD_RELATIVE_DBL(dbl_machine_constant, DBL_EPSILON) - DBL_EPSILON);
        dbe(ADD_RELATIVE_DBL(DBL_EPSILON, DBL_EPSILON) - dbl_machine_constant);
        
        failed = TRUE;
    }

    if (kjb_get_verbose_level() > 0)
    {
        verbose_pso(1, "In hex: "); 
        hex_print(stdout, &dbl_machine_constant, 8); 
        verbose_pso(1, "\n");
    }

    i = 5;
    verbose_pso(1, "Test: %d\n", i); 
    EPETE(slatec_d1mach(&dbl_machine_constant, i));
    verbose_pso(1, "Log10 of floating point base: %.10e.\n",
                dbl_machine_constant); 
    verbose_pso(1, "Should be log10(2.0)=%.10e.\n", log10(2.0)); 

    /* It does not necessarily need to be this way, but if it is not, we should
     * check it and adjust this test if it is OK.  
    */
    if ( ! IS_EQUAL_DBL(dbl_machine_constant, log10(2.0)))
    {
        dbp("!!!! FAILED !!!!"); 
        dbw();
        dbi(i);
        dbe(dbl_machine_constant - log10(2.0)); 
        dbe(ADD_RELATIVE_DBL(dbl_machine_constant, DBL_EPSILON) - log10(2.0));
        dbe(ADD_RELATIVE_DBL(log10(2.0), DBL_EPSILON) - dbl_machine_constant);

        failed = TRUE;
    }

    if (kjb_get_verbose_level() > 0)
    {
        verbose_pso(1, "In hex: "); 
        hex_print(stdout, &dbl_machine_constant, 8); 
        verbose_pso(1, "\n--------------------------\n\n"); 
    }

    /* ---------------------------------------------------------------- */


    i = 9;
    verbose_pso(1, "Test: %d\n", i); 
    EPETE(slatec_i1mach(&int_machine_constant, i));
    verbose_pso(1, "Should be largest integer (%d): %d.\n",
                INT_MAX, int_machine_constant); 

    /* It does not necessarily need to be this way, but if it is not, we should
     * check it and adjust this test if it is OK.  
    */
    if (int_machine_constant != INT_MAX)
    {
        dbp("!!!! FAILED !!!!"); 
        dbw(); 
        dbi(int_machine_constant);
        dbi(INT_MAX);
        dbi(int_machine_constant - INT_MAX); 
        
        failed = TRUE;
    }

    if (kjb_get_verbose_level() > 0)
    {
        verbose_pso(1, "--------------------------\n\n"); 
    }

    
    i = 11;
    verbose_pso(1, "Test: %d\n", i); 
    EPETE(slatec_i1mach(&int_machine_constant, i));
    verbose_pso(1, "Number of single precsion floating point digits: %d.\n", 
                int_machine_constant); 
    verbose_pso(1, "Usually 24. One for sign, and 23 for the digits (see ieee754.h).\n"); 

    /* It does not necessarily need to be this way, but if it is not, we should
     * check it and adjust this test if it is OK.  
    */
    if (int_machine_constant != 24)
    {
        dbp("!!!! FAILED !!!!"); 
        dbw(); 
        dbi(int_machine_constant);
        dbi(int_machine_constant - 24); 
        
        failed = TRUE;
    }


    verbose_pso (1, "--------------------------\n\n");

    i = 14;
    verbose_pso(1, "Test: %d\n", i); 
    EPETE(slatec_i1mach(&int_machine_constant, i));
    verbose_pso(1, "Number of floating point digits: %d.\n", int_machine_constant); 
    verbose_pso(1, "Usually 53. One for sign, and 52 for the digits (see ieee754.h).\n"); 

    /* It does not necessarily need to be this way, but if it is not, we should
     * check it and adjust this test if it is OK.  
    */
    if (int_machine_constant != 53)
    {
        dbp("!!!! FAILED !!!!"); 
        dbw(); 
        dbi(int_machine_constant);
        dbi(int_machine_constant - 53); 
        
        failed = TRUE;
    }

    verbose_pso (1, "--------------------------\n\n");
    
    i = 15;
    verbose_pso(1, "Test: %d\n", i); 
    EPETE(slatec_i1mach(&int_machine_constant, i));
    verbose_pso(1, "The min exponant: %d.\n", int_machine_constant); 
    verbose_pso(1, "Usually -1022 (see ieee754.h).\n"); 
    verbose_pso(1, "This is 1-1023, where 1 is the smallest unsigned greater than 0 and 1023 is the bias.\n"); 
    verbose_pso(1, "However, the counting used by i1mach seems to count the implicit leading one.\n"); 
    verbose_pso(1, "Under that assumption we expect -1021.\n"); 
    verbose_pso(1, "(I am not positive that I have the explanation right)\n"); 

    /* It does not necessarily need to be this way, but if it is not, we should
     * check it and adjust this test if it is OK.  
    */
    if (int_machine_constant != -1021)
    {
        dbp("!!!! FAILED !!!!"); 
        dbw(); 
        dbi(int_machine_constant);
        dbi(int_machine_constant + 1021); 
        
        failed = TRUE;
    }

    verbose_pso (1, "--------------------------\n\n");
    
    i = 16;
    verbose_pso(1, "Test: %d\n", i); 
    EPETE(slatec_i1mach(&int_machine_constant, i));
    verbose_pso(1, "The max exponant: %d.\n", int_machine_constant); 
    verbose_pso(1, "It seems to me that this should be 1023 (see ieee754.h).\n"); 
    verbose_pso(1, "This is 2046-1023, where 1 is the smallest unsigned greater than 0 and 1023 is the bias.\n"); 
    verbose_pso(1, "However, the counting used by i1mach seems to count the implicit leading one.\n"); 
    verbose_pso(1, "Under that assumption we expect 1024.\n"); 
    verbose_pso(1, "(I am not positive that I have the explanation right)\n"); 

    /* It does not necessarily need to be this way, but if it is not, we should
     * check it and adjust this test if it is OK.  
    */
    if (int_machine_constant != 1024)
    {
        dbp("!!!! FAILED !!!!"); 
        dbw(); 
        dbi(int_machine_constant);
        dbi(int_machine_constant - 1024); 
        
        failed = TRUE;
    }

    verbose_pso (1, "--------------------------\n");
    verbose_pso(1, "\n");

    if (kjb_get_verbose_level() > 0)
    {
        dbe(FLT_MIN); 
        dbe(kjb_log2(FLT_MIN)); 
        dbe(FLT_MAX); 
        dbe(kjb_log2(FLT_MAX)); 
        

        dbp("--------------------------"); 

        dbe(DBL_MIN); 
        dbe(kjb_log2(DBL_MIN)); 
        dbe(DBL_MAX); 
        dbe(kjb_log2(DBL_MAX)); 
        
        dbp("--------------------------"); 
    }

    if (failed)
    {
        dbp("\nAt least one test failed.\n"); 
        return EXIT_BUG; 
    }
    
    return EXIT_SUCCESS; 
}


