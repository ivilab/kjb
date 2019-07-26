
/* $Id: ncc.c 21491 2017-07-20 13:19:02Z kobus $ */



#include "i/i_float.h"
#include "i/i_float_io.h"
#include "i/i_convolve.h"
#include "i/i_matrix.h"

#include "m2/m2_ncc.h"

/* Kobus: Moved main to top. */
static void test_convolve(void);
static void test_convolve_image(void);
static void test_ncc(void);


int main(int argc, char *argv[]) 
{

    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    test_convolve();
    test_convolve_image();
    test_ncc();

    exit(EXIT_SUCCESS); 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int convolve_matrix1
(
 Matrix**      out_mpp,
 const Matrix* in_mp,
 const Matrix* mask_mp
)
{
    const Matrix* out_mp;
    Matrix *tmp_mp = NULL;
    int     num_rows, num_cols, i, j, mi, mj, m, n;
    int     mask_rows  = mask_mp->num_rows;
    int     mask_cols  = mask_mp->num_cols;
    int     mask_row_offset = mask_rows / 2;
    int     mask_col_offset = mask_cols / 2;

    /* TODO: Better error checking here, not just ASSERT. */
    ASSERT(mask_mp->num_rows <= in_mp->num_rows);
    ASSERT(mask_mp->num_cols <= in_mp->num_cols);

    num_rows = in_mp->num_rows;
    num_cols = in_mp->num_cols;

    ERE(get_target_matrix(out_mpp, num_rows, num_cols));
    out_mp = *out_mpp;

    ERE(get_target_matrix(&tmp_mp, mask_rows, mask_cols));
    for (i=0; i<num_rows; i++)

    {
        for (j=0; j<num_cols; j++)

        {
            double sum      = 0.0;
            double weight;

            for (mi = 0; mi < mask_rows; mi++)

            {
                for (mj = 0; mj < mask_cols; mj++)

                {
                    m = i + mask_row_offset - mi;
                    n = j + mask_col_offset - mj;

                    /*
                     * TODO: Easy performance gain: Handle the boundary cases
                     * separately.
                     */
                    if (m < 0 || m >= num_rows) tmp_mp->elements[mi][mj] = 0.0;
                    else if( n < 0 || n >= num_cols) tmp_mp->elements[mi][mj] = 0.0;
                    else 
                    {
                        tmp_mp->elements[mi][mj] = in_mp->elements[m][n];
                    }

                    weight = mask_mp->elements[mi][mj];

                    sum += tmp_mp->elements[mi][mj] * weight;
                }
            }
            out_mp->elements[i][j] = sum;
        }
    }

    free_matrix(tmp_mp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int variance_normalization_matrix1(Matrix **mpp, const Matrix *in_mp,
                                   const Matrix *t_mp) 
{
    const Matrix* out_mp = NULL;
    int     num_rows, num_cols, i, j, mi, mj, m, n;
    int     mask_rows  = t_mp->num_rows;
    int     mask_cols  = t_mp->num_cols;
    Matrix *tmp_mp = NULL;
    int num;
    int mask_row_offset, mask_col_offset;

    /* TODO: Better error checking here, not just ASSERT. */
    ASSERT(mask_rows <= in_mp->num_rows);
    ASSERT(mask_cols <= in_mp->num_cols);

    num_rows = in_mp->num_rows;
    num_cols = in_mp->num_cols;
    mask_row_offset = t_mp->num_rows / 2;
    mask_col_offset = t_mp->num_cols / 2;

    ERE(get_target_matrix(mpp, num_rows, num_cols));
    out_mp = *mpp;

    tmp_mp = create_matrix(mask_rows, mask_cols); 
    for (i=0; i<num_rows; i++)

    {
        for (j=0; j<num_cols; j++)

        {
            for (mi = 0; mi < mask_rows; mi++)

            {
                for (mj = 0; mj < mask_cols; mj++)

                {
                    m = i - mask_row_offset + mi;
                    n = j - mask_col_offset +  mj;

                    if(m < 0 || n < 0)
                        tmp_mp->elements[mi][mj] = 0;
                    else if(m >= num_rows || n >= num_cols)
                        tmp_mp->elements[mi][mj] = 0;
                    else                                                                       
                        tmp_mp->elements[mi][mj] = in_mp->elements[m][n];
                }
            }

            if((i==2 && j==5) || (i==4 && j==5)) 
            {
                printf("%d %d\n", i,j);
                write_matrix(tmp_mp, NULL);
            } 
            ncc_matrix(&(out_mp->elements[i][j]), tmp_mp, t_mp);
        }
    }

    free_matrix(tmp_mp);                                                                                   
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int is_same_image(const KJB_image *ip1, const KJB_image *ip2, int gutter) 
{
    int num_rows, num_cols;
    int i, j;

    num_rows = ip1->num_rows;
    num_cols = ip2->num_cols;
    for(i=gutter; i<num_rows- gutter; i++) 
    {
        for(j=gutter; j<num_cols-gutter; j++) 
        {
            if(
                fabs(ip1->pixels[i][j].r - ip2->pixels[i][j].r) > NCC_EPSLON ||
                fabs(ip1->pixels[i][j].g - ip2->pixels[i][j].g) > NCC_EPSLON ||
                fabs(ip1->pixels[i][j].b - ip2->pixels[i][j].b) > NCC_EPSLON
                ) 
            {
                printf("fail at pixel: %d %d\n", i,j);
/*                pso("image 1\n"); */
/*                write_image(ip1, NULL); */
/*                pso("image 2\n"); */
/*                write_image(ip2, NULL); */


                    kjb_display_image(ip1, "slow convolve");
                    kjb_display_image(ip2, "fast convolve");

                    sleep(30);
                    exit(1);

            }
        }
    }
    return 1;
}

int is_same(const Matrix *mp1, const Matrix *mp2) 
{
    int num_rows, num_cols;
    int i, j;

    num_rows = mp1->num_rows;
    num_cols = mp2->num_cols;
    for(i=0; i<num_rows; i++) 
    {
        for(j=0; j<num_cols; j++) 
        {
            if(fabs(mp1->elements[i][j] - mp2->elements[i][j]) > NCC_EPSLON) 
            {
                printf("%d %d\n", i,j);
                pso("matrix 1\n");
                write_matrix(mp1, NULL);
                pso("matrix 2\n");
                write_matrix(mp2, NULL);
                exit(1);
            }
        }
    }
    return 1;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void test_convolve() 
{
    Matrix *in_mp = NULL, *mask_mp = NULL, *conv_mp1 = NULL, *conv_mp2=NULL;
    int i, j;

    get_random_matrix(&in_mp, 17 , 17);
    ow_multiply_matrix_by_scalar(in_mp, 255);
    for(i=1; i<=17; i++) 
    {
        for(j=1; j<=17; j++) 
        {
            get_random_matrix(&mask_mp, i, j);
            convolve_matrix1(&conv_mp1, in_mp, mask_mp);
            fourier_convolve_matrix(&conv_mp2, in_mp, mask_mp);
            if(is_same(conv_mp1, conv_mp2)) 
            {
                printf("%d %d\n", i,j);
            }
            else
            {
                puts("FAILED");
            }

            /*
            if(j == 2)
            {
                display_matrix(conv_mp1, "slow convolve");
                display_matrix(conv_mp2, "fast convolve");

                sleep(10);
                exit(1);

            }
            */
        }
    }    

    free_matrix(mask_mp);
    free_matrix(in_mp);
    free_matrix(conv_mp1);
    free_matrix(conv_mp2);
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void test_convolve_image() 
{
    KJB_image* in_ip = NULL;
    KJB_image* conv_ip1 = NULL;
    KJB_image* conv_ip2 = NULL;

    Matrix* mask_mp = NULL;
    Matrix* r_mp = NULL;
    Matrix* g_mp = NULL;
    Matrix* b_mp = NULL;
    int i, j;

    get_random_matrix(&r_mp, 255 , 255);
    ow_multiply_matrix_by_scalar(r_mp, 255);
    get_random_matrix(&g_mp, 255 , 255);
    ow_multiply_matrix_by_scalar(g_mp, 255);
    get_random_matrix(&b_mp, 255 , 255);
    ow_multiply_matrix_by_scalar(b_mp, 255);

    rgb_matrices_to_image(r_mp, g_mp, b_mp, &in_ip);

    for(i=1; i<=32; i += 3) 
    {
        for(j=1; j<=32; j += 3) 
        {
            get_random_matrix(&mask_mp, i, j);
            convolve_image(&conv_ip1, in_ip, mask_mp);
            fourier_convolve_image(&conv_ip2, in_ip, mask_mp);

            if(is_same_image(conv_ip1, conv_ip2, (i > j ? i : j))) 
            {
                printf("%d %d\n", i,j);
            }
        }
    }    

    free_matrix(mask_mp);
    free_matrix(r_mp);
    free_matrix(g_mp);
    free_matrix(b_mp);
    kjb_free_image(in_ip);
    kjb_free_image(conv_ip1);
    kjb_free_image(conv_ip2);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void test_ncc() 
{
    Matrix *in_mp = NULL, *mask_mp = NULL, *corr_mp1 = NULL, *corr_mp2=NULL;
    int i, j;

    get_random_matrix(&in_mp, 15, 15);
    for(i=1; i<=15; i++) 
    {
        for(j=1; j<=15; j++) 
        {
            get_random_matrix(&mask_mp, i, j);
            variance_normalization_matrix1(&corr_mp1, in_mp, mask_mp);
            fourier_ncc_template_matrix(&corr_mp2, in_mp, mask_mp);
            if(is_same(corr_mp1, corr_mp2)) 
            {
                printf("%d %d\n", i,j);
            }
        }
    }    
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

