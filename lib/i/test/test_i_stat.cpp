#include <iostream>
#include <i/i_float.h>
#include <i/i_stat.h>
#include <i/i_float.h>
#include <m/m_vector.h>

using namespace std;
using namespace kjb_c;


KJB_image *fake_ip1;
KJB_image *fake_ip2;
KJB_image *invalid_ip;


KJB_image *construct_fake_img1(void)
{
    KJB_image *img = new KJB_image;

    img->num_rows = 2;
    img->num_cols = 2;
    img->read_only = 0;
    img->pixels = new Pixel*[2];
    img->pixels[0] = new Pixel[2];
    img->pixels[1] = new Pixel[2];

    Invalid_pixel valid = {0,0,0,0};
    Invalid_pixel invalid = {1,1,1,1};

    Pixel_extra validPE;
    Pixel_extra invalidPE;

    validPE.invalid = valid;
    invalidPE.invalid = invalid;

    img->pixels[0][0] = {0.5, 1.0, 1.0,validPE};
    img->pixels[0][1] = {FLT_MAX, FLT_MAX, FLT_MAX, invalidPE};
    img->pixels[1][0] = {1.5, 0.0, 1.0,validPE};
    img->pixels[1][1] = {0.0, 1.0, 1.0,validPE};

    return img;
}


KJB_image *construct_fake_img2(void)
{
    KJB_image *img = new KJB_image;
    
        img->num_rows = 2;
        img->num_cols = 2;
        img->read_only = 0;
        img->pixels = new Pixel*[2];
        img->pixels[0] = new Pixel[2];
        img->pixels[1] = new Pixel[2];
    
        Invalid_pixel valid = {0,0,0,0};
        Invalid_pixel invalid = {1,0,1,1};
    
        Pixel_extra validPE;
        Pixel_extra invalidPE;
    
        validPE.invalid = valid;
        invalidPE.invalid = invalid;
    
        img->pixels[0][0] = {0.5, 1.0, 1.0,validPE};
        img->pixels[0][1] = {FLT_MAX, FLT_MAX, FLT_MAX, validPE};
        img->pixels[1][0] = {1.5, 0.0, 1.0,validPE};
        img->pixels[1][1] = {0.0, 1.0, 1.0,invalidPE};
    
        return img;
}


KJB_image *construct_invalid_img()
{
    KJB_image *img = new KJB_image;
    img->num_rows = 1;
    img->num_cols = 1;
    img->read_only = 0;
    img->pixels = new Pixel*[1];
    img->pixels[0] = new Pixel[1];
    Pixel_extra invalidPE;

    Invalid_pixel invalid = {1,1,0,1};
    invalidPE.invalid = invalid;

    img->pixels[0][0] = {0.5, 0.0, -FLT_MAX/2-1,invalidPE};

    return img;
}


int test_get_ave_rgb(
    const KJB_image *in_ip = construct_fake_img1(), 
    double ave_r = 2.0/3.0,
    double ave_g = 2.0/3.0,
    double ave_b = 1.0
)
{
    Vector *out_rgb_vp = NULL;
    int rc;

    

    rc = get_ave_rgb(&out_rgb_vp,in_ip);
    if (rc == ERROR)
    {
        cerr << __func__ 
            << " reported unexpected ERROR or got a system/user failure" 
            << endl;
        kjb_exit(EXIT_BUG);
    }

    

    ASSERT_IS_EQUAL_DBL(out_rgb_vp->elements[ 0 ], ave_r)
    ASSERT_IS_EQUAL_DBL(out_rgb_vp->elements[ 1 ], ave_g)
    ASSERT_IS_EQUAL_DBL(out_rgb_vp->elements[ 2 ], ave_b)

    free_vector(out_rgb_vp);

    cout << __func__ << " passed" << endl;
    return NO_ERROR;
}

// test error reporting in get_ave_rgb()
int test_error_get_ave_rgb(const KJB_image *in_ip = construct_invalid_img()) 
{
    Vector *out_rgb_vp = NULL;
    int rc;
    
    rc = get_ave_rgb(&out_rgb_vp,in_ip);

    ASSERT_IS_EQUAL_INT(rc, ERROR)

    free_vector(out_rgb_vp);
    
    cout << __func__ << " passed" << endl;
    return NO_ERROR;
}


int test_get_max_rgb(
    const KJB_image *in_ip = construct_fake_img1(), 
    double max_r = FLT_MAX,
    double max_g = FLT_MAX,
    double max_b = FLT_MAX
)
{
    Vector *out_rgb_vp = NULL;
    int rc;

    rc = get_max_rgb(&out_rgb_vp,in_ip);
    if (rc == ERROR)
    {
        cerr << __func__ 
            << " reported unexpected ERROR or got a system/user failure" 
            << endl;
        kjb_exit(EXIT_FAILURE);
    }

    ASSERT_IS_EQUAL_DBL(out_rgb_vp->elements[ 0 ], max_r)
    ASSERT_IS_EQUAL_DBL(out_rgb_vp->elements[ 1 ], max_g)
    ASSERT_IS_EQUAL_DBL(out_rgb_vp->elements[ 2 ], max_b)

    free_vector(out_rgb_vp);

    cout << __func__ << " passed" << endl;
    return NO_ERROR;
}



// test error reporting in get_ave_rgb()
int test_error_get_max_rgb(const KJB_image *in_ip = construct_invalid_img())
{
    Vector *out_rgb_vp = NULL;
    int rc;

    rc = get_max_rgb(&out_rgb_vp,in_ip);

    ASSERT_IS_EQUAL_INT(rc, ERROR)

    free_vector(out_rgb_vp);
    
    cout << __func__ << " passed" << endl;
    return NO_ERROR;
}


int test_error_get_image_window_stats()
{

    Vector *mean_vp = NULL;
    Vector *stdev_vp = NULL;

    ERE(get_target_vector(&mean_vp,1));
    ERE(get_target_vector(&stdev_vp,1));

    int rc;

    rc = get_image_window_stats(
        NULL,
        NULL,
        NULL,
        fake_ip1,
        0,
        0,
        0,
        0
    );

    ASSERT_IS_EQUAL_INT(rc, ! ERROR);
    



    rc = get_image_window_stats(
        NULL,
        &mean_vp,
        &stdev_vp,
        invalid_ip,
        0,
        0,
        0,
        0
    );

    ASSERT_IS_EQUAL_INT(rc, ERROR);

    free_vector(mean_vp);
    free_vector(stdev_vp);


    cout.flush() << __func__ << " passed" << endl;
    return NO_ERROR;
}



//
int test_get_image_window_stats(
    const   KJB_image * source_ip = fake_ip1,
    int     row_offset = 0,
    int     col_offset = 0,
    int     num_target_rows = 2,
    int     num_target_cols = 2,
    int     num_valid = 3,
    double  mean_r = 2.0/3.0,
    double  mean_g = 2.0/3.0,
    double  mean_b = 1.0,
    double  stdev_r = 7.638e-01,
    double  stdev_g = 5.774e-01,
    double  stdev_b = 0
)
{
    int num_valid_pixels;
    Vector *mean_vp = NULL;
    Vector *stdev_vp = NULL;

    ERE(get_target_vector(&mean_vp,1));
    ERE(get_target_vector(&stdev_vp,1));

    int rc;

    rc = get_image_window_stats(
        &num_valid_pixels,
        &mean_vp,
        &stdev_vp,
        source_ip,
        row_offset,
        col_offset,
        num_target_rows,
        num_target_cols
    );

    if (rc == ERROR)
    {
        cerr << __func__ 
            << " reported unexpected ERROR or got a system/user failure" 
            << endl;
        kjb_exit(EXIT_FAILURE);
    }


    ASSERT_IS_EQUAL_INT(num_valid_pixels,num_valid);

    ASSERT_IS_NEARLY_EQUAL_DBL(mean_vp->elements[ 0 ], mean_r,1.0e-4);
    ASSERT_IS_NEARLY_EQUAL_DBL(mean_vp->elements[ 1 ], mean_g,1.0e-4);
    ASSERT_IS_NEARLY_EQUAL_DBL(mean_vp->elements[ 2 ], mean_b,1.0e-4);

    ASSERT_IS_NEARLY_EQUAL_DBL(stdev_vp->elements[ 0 ], stdev_r,1.0e-4);
    ASSERT_IS_NEARLY_EQUAL_DBL(stdev_vp->elements[ 1 ], stdev_g,1.0e-4);
    ASSERT_IS_NEARLY_EQUAL_DBL(stdev_vp->elements[ 2 ], stdev_b,1.0e-4);


    free_vector(mean_vp);
    free_vector(stdev_vp);

    cout.flush() << __func__ << " passed" << endl;
    return NO_ERROR;
}




int test_get_ave_ratio_without_invalid(
    const   KJB_image *in1_ip = construct_fake_img1(),
    const   KJB_image *in2_ip = construct_fake_img2(),
    double  threshold = -1,
    int     min_num_good_point = -1,
    double  ave_ratio_r = 1.0,
    double  ave_ratio_g = 1.0,
    double  ave_ratio_b = 1.0
)
{
    Vector *results_vp = NULL;
    int rc;

    rc = get_ave_ratio_without_invalid(&results_vp,in1_ip,in2_ip,threshold,min_num_good_point);

    if (rc == ERROR)
    {
        cerr << __func__ 
        << " reported unexpected ERROR or got a system/user failure" 
        << endl;
        kjb_exit(EXIT_FAILURE);
    }    

    ASSERT_IS_EQUAL_DBL(results_vp->elements[ 0 ], ave_ratio_r)
    ASSERT_IS_EQUAL_DBL(results_vp->elements[ 1 ], ave_ratio_g)
    ASSERT_IS_EQUAL_DBL(results_vp->elements[ 2 ], ave_ratio_b)

    free_vector(results_vp);

    cout.flush() << __func__ << " passed" << endl;
    return NO_ERROR;
}


int test_error_get_ave_ratio_without_invalid(
    const   KJB_image *in1_ip = construct_fake_img1(),
    const   KJB_image *in2_ip = construct_fake_img2()
)
{
    Vector *results_vp = NULL;
    int rc;

    rc = get_ave_ratio_without_invalid(
        &results_vp,
        in1_ip,
        in2_ip,
        100,
        0);

    ASSERT_IS_EQUAL_INT(rc, ERROR)

    rc = get_ave_ratio_without_invalid(
        &results_vp,
        in1_ip,
        in2_ip,
        0,
        3);
        
    ASSERT_IS_EQUAL_INT(rc, ERROR)

    free_vector(results_vp);
        
    cout << __func__ << " passed" << endl;
    return NO_ERROR;

}


int test_get_ave_sum_ratio_without_invalid(
    const   KJB_image *in1_ip = construct_fake_img1(),
    const   KJB_image *in2_ip = construct_fake_img2(),
    double  threshold = -1,
    int     min_num_good_point = -1,
    double  theRatio = 1
)
{
    double result = 0;
    int rc;

    rc = get_ave_sum_ratio_without_invalid(&result,in1_ip,in2_ip,threshold,min_num_good_point);

    if (rc == ERROR)
    {
        cerr << __func__ 
        << " reported unexpected ERROR or got a system/user failure" 
        << endl;
        kjb_exit(EXIT_FAILURE);
    }    

    ASSERT_IS_EQUAL_DBL(result,theRatio)

    cout << __func__ << " passed" << endl;
    return NO_ERROR;
}


int test_error_get_ave_sum_ratio_without_invalid(
    const   KJB_image *in1_ip = construct_fake_img1(),
    const   KJB_image *in2_ip = construct_fake_img2()
)
{
    double result;
    int rc;

    rc = get_ave_sum_ratio_without_invalid(
        &result,
        in1_ip,
        in2_ip,
        100.0,
        0);

    ASSERT_IS_EQUAL_INT(rc, ERROR)

    rc = get_ave_sum_ratio_without_invalid(
        &result,
        in1_ip,
        in2_ip,
        0.0,
        3);
        
    ASSERT_IS_EQUAL_INT(rc, ERROR)
        
    cout << __func__ << " passed" << endl;
    return NO_ERROR;
}


int main(int argc, char const *argv[])
{
    if (argc == 1)
        cout << "using fake data inside test program" << endl
            << "error detection for this rountine will also be tested" << endl;
    else
        cout << "using input specified in arguments" << endl;


    fake_ip1 = construct_fake_img1();
    fake_ip2 = construct_fake_img2();
    invalid_ip = construct_invalid_img();



    test_get_ave_rgb();
    test_error_get_ave_rgb();
    test_get_max_rgb();
    test_error_get_max_rgb();


    test_get_image_window_stats();
    test_error_get_image_window_stats();
    


    test_get_ave_ratio_without_invalid();
    test_error_get_ave_ratio_without_invalid();
    test_get_ave_sum_ratio_without_invalid();
    test_error_get_ave_sum_ratio_without_invalid();



    kjb_exit(EXIT_SUCCESS);
    
}