#include <g/g_plucker.h>
#include <g/g_point_projection.h>

#define TEST_DIR "conversion_test/"
#define CAMERA_FILE "camera_0.txt"
#define NUM_TESTS 10
#define STR_SZ 29
#define IDX 23

Matrix *p_proj=0;
Matrix *l_proj=0;
Matrix *new_proj=0;
Matrix *scale_proj=0;
char *read_str=0;
int tests_failed=0;

int runTest(int);
void freeAll();

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
    int i;

    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    read_str = (char*)malloc(sizeof(char)*STR_SZ);
    read_str = strcpy(read_str,TEST_DIR);
    read_str = strcat(read_str,CAMERA_FILE);

    printf("This tests the conversion of camera matrices between R^3 and Plucker space. This test program uses data in directory \"%s\".\n\n",TEST_DIR);
    printf("-----\nNumber of tests: %d\n-----\n\n", NUM_TESTS);
    for (i=0; i<NUM_TESTS; i++) {
        runTest(i);
    }

    freeAll();

    printf("Total tests failed: %d\n", tests_failed);
    
    if (tests_failed == 0) 
    {
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_BUG;
    }

}

int runTest(int test_id)
{
    int check,i,j;
    double scalar1,scalar2,scalar3;

    printf("Test %d:\n\n", test_id);
    read_str[IDX] = '0'+test_id;
    read_matrix(&p_proj,read_str);

    ERE(line_projection_matrix_from_point_project_matrix(&l_proj, p_proj));
    ERE(point_projection_matrix_from_line_projection_matrix(&new_proj, l_proj));

    printf("The original point projection values are:\n");
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            printf("%f\t", p_proj->elements[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("The line projection values are:\n");
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 6; j++) {
            printf("%f\t", l_proj->elements[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    
    printf("The new point projection values are:\n");
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            printf("%f\t", new_proj->elements[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    for(i = 0; i < 4; i++)
    {
    	if(fabs(p_proj->elements[0][i]) > FLT_EPSILON && fabs(new_proj->elements[0][i]) > FLT_EPSILON)
    	{
    		scalar1 = p_proj->elements[0][i]/new_proj->elements[0][i];
    		break;
    	}
    }
    for(i = 0; i < 4; i++)
    {
    	if(fabs(p_proj->elements[1][i]) > FLT_EPSILON && fabs(new_proj->elements[1][i]) > FLT_EPSILON)
    	{
    		scalar2 = p_proj->elements[1][i]/new_proj->elements[1][i];
    		break;
    	}
    }
    for(i = 0; i < 4; i++)
    {
    	if(fabs(p_proj->elements[2][i]) > FLT_EPSILON && fabs(new_proj->elements[2][i]) > FLT_EPSILON)
    	{
    		scalar3 = p_proj->elements[2][i]/new_proj->elements[2][i];
    		break;
    	}
    }

    printf("The scale between the first rows is: %f\n", scalar1);
    printf("The scale between the second rows is: %f\n",scalar2);
    printf("The scale between the third rows is: %f\n", scalar3);

    printf("Testing for equality between scalars ... "); PASS_OR_FAIL(fabs(scalar1-scalar2)<FLT_EPSILON && fabs(scalar1-scalar3)<FLT_EPSILON && fabs(scalar2-scalar3)<FLT_EPSILON);
    printf("\n");

    ERE(get_target_matrix(&scale_proj, 3, 4));
    printf("The new point projection values after scaling are:\n");
    for (i = 0; i < 3; i++) {
        for (j=0; j < 4; j++) {
            scale_proj->elements[i][j]=new_proj->elements[i][j]*scalar1;
            printf("%f\t", scale_proj->elements[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    printf("Testing equality between original matrix and scaled matrix ... ");
    check = 1;
    for (i = 0; i < 3; i++) {
        for (j=0; j < 4; j++) {
            if (fabs(scale_proj->elements[i][j]-p_proj->elements[i][j])>FLT_EPSILON) {
                check = 0;
            }
        }
    }
    PASS_OR_FAIL(check);
    printf("\n");

    return NO_ERROR;
}

void freeAll()
{
    free_matrix(p_proj);
    free_matrix(l_proj);
    free_matrix(new_proj);
    free_matrix(scale_proj);
    free(read_str);
}
