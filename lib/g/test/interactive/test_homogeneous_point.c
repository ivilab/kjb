#include <g/g_homogeneous_point.h>

#define TEST_DIR "homogenize_test/"
#define CASE "vec0"
#define EXPECTED "exp0"
#define ERR_CASE "err0"
#define STR_SZ 21
#define IDX 19
#define NUM_CASES 5
#define NUM_ERRS 2

int test_values(Vector*, Vector*);
void free_all();

Vector* to_test=0;
Vector* target=0;
Vector* original=0;
Vector* result=0;

char* read_str=0;
char* exp_str=0;
char* err_str=0;
int tests_failed=0;

void PASS_OR_FAIL(int pof)
{
    if (pof)
    {
        printf("Pass\n");
    } else {
        printf("FAIL!\n");
        tests_failed++;
    }
}

int main()
{
    int i,j;

    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    read_str = (char*)malloc(sizeof(char)*STR_SZ);
    exp_str = (char*)malloc(sizeof(char)*STR_SZ);
    err_str = (char*)malloc(sizeof(char)*STR_SZ);

    read_str = strcpy(read_str, TEST_DIR); read_str = strcat(read_str, CASE);
    exp_str = strcpy(exp_str, TEST_DIR); exp_str = strcat(exp_str, EXPECTED);
    err_str = strcpy(err_str, TEST_DIR); err_str = strcat(err_str, ERR_CASE);

    printf("This simple test runs against cases in directory \"%s\"\n\n",TEST_DIR);
    
    printf("-----\nNormal cases: %d\n-----\n", NUM_CASES);
    for (i=0; i<NUM_CASES; i++)
    {
        read_str[IDX] = '0'+i;
        exp_str[IDX] = '0'+i;

        read_vector(&to_test,read_str);
        read_vector(&original,read_str);
        read_vector(&result,exp_str);

        printf("Case %d:\n",i);
        printf("Original Vector:\t\t");
        for (j=0; j<to_test->length; j++)
        {
            printf("%f\t", to_test->elements[j]);
        }
        printf("\n");
        printf("Expected Vector:\t\t");
        for (j=0; j<result->length; j++)
        {
            printf("%f\t", result->elements[j]);
        }
        printf("\n");

        homogenize_vector(&target,to_test);

        printf("Call to homogenize_vector produced correct results: ");
        PASS_OR_FAIL((test_values(target,result))==NO_ERROR);
        printf("Call to homogenize_vector left original vector unchanged: ");
        PASS_OR_FAIL((test_values(to_test,original))==NO_ERROR);

        ow_homogenize_vector(to_test);

        printf("Call to ow_homogenize_vector produced correct results: ");
        PASS_OR_FAIL((test_values(to_test,result))==NO_ERROR);

        printf("\n");
    }

    printf("-----\nError cases: %d\n-----\n", NUM_ERRS);
    for (i=0; i<NUM_ERRS; i++)
    {
        err_str[IDX] = '0'+i;

        read_vector(&to_test,err_str);

        printf("Case %d:\n",i);
        printf("Bad Vector:\t\t");
        for (j=0; j<to_test->length; j++)
        {
            printf("%f\t", to_test->elements[j]);
        }
        printf("\n");

        printf("Call to homogenize_vector returns ERROR: ");
        PASS_OR_FAIL(homogenize_vector(&target,to_test)==ERROR);
        printf("Call to ow_homogenize_vector returns ERROR: ");
        PASS_OR_FAIL(ow_homogenize_vector(to_test)==ERROR);

        printf("\n");
    }

    free_all();
    printf("Total tests failed: %d\n",tests_failed);

    if (tests_failed == 0) 
    {
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_BUG;
    }

}

int test_values(Vector *actual, Vector *expected)
{
    int i;

    if (actual->length != expected->length)
    {
        return ERROR;
    }

    for (i = 0; i < actual->length; i++)
    {
        if (fabs(actual->elements[i] - expected->elements[i])>DBL_EPSILON)
        {
            return ERROR;
        }
    }

    return NO_ERROR;
}

void free_all()
{
    free_vector(to_test);
    free_vector(target);
    free_vector(original);
    free_vector(result);
    free(read_str);
    free(exp_str);
    free(err_str);
}
