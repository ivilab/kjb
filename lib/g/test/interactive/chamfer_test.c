#include <m/m_matrix.h>
#include <i/i_matrix.h>
#include <i/i_float.h>
#include <i/i_float_io.h>
#include <g/g_chamfer.c>

/**
 * This is a simple test of the chamfer transform function.  It reads a binary 
 * edge image and outputs two output images, one grayscale image representing 
 * the image map, and another color image where the red channel is the image 
 * map, and the green and blue channels are the x and y locations of the closest
 * edge, respectively. 
 *
 * I modified Joe's viewer application to use this output to highlight the 
 * clostest edge point when you hover over a pixel, but the code is written 
 * in Joe's jwsc framework, so it won't compile under kjb and isn't included 
 * here.  The code is available in svn under chamfer_viewer. 
 *
 * A better test would directly test each pixel for its closest edge pixel and 
 * the corresponding distance, but we'd need to account for the chamfer 
 * transform's approximation error.  For now, the output appears qualitatively 
 * correct, according to the chamfer_viewer application.
 */

const char* IMG_FNAME = "in.tiff";
const char* BW_IMG_FNAME = "bw_out.sun";
const char* THREE_IMG_FNAME = "3_out.sun";

int main()
{
    /* read image */
    KJB_image* img = NULL;
    KJB_image* bw_out = NULL;
    KJB_image* three_out = NULL;

    Matrix* m_img = NULL;
    Matrix* distances = NULL;
    Matrix* u = NULL;
    Matrix* v = NULL;
    Int_matrix_vector* locations = NULL;

    ERE(kjb_init());

    /* Not ready to do anything in batch mode.  */
    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    

/*    scale-by-max-rgb = true; */

    ERE(kjb_read_image(&img, IMG_FNAME));
    ERE(image_to_matrix(img, &m_img));

    /* basic chamfer xform */
    ERE(chamfer_transform(m_img, 7, &distances, &locations));

    /* convert distance and locations (u,v) to rgb image */
    EPETE(copy_int_matrix_to_matrix(&u, locations->elements[0]));
    EPETE(copy_int_matrix_to_matrix(&v, locations->elements[1]));


    /* output xformed image */
    EPETE(matrix_to_bw_image(distances, &bw_out));
    EPETE(rgb_matrices_to_image(distances, u, v, &three_out));
    kjb_write_image(bw_out, BW_IMG_FNAME);
    kjb_write_image(three_out, THREE_IMG_FNAME);

    free_matrix(u);
    free_matrix(v);
    free_matrix(m_img);
    free_matrix(distances);

    kjb_free_image(img);
    kjb_free_image(bw_out);
    kjb_free_image(three_out);

    free_int_matrix_vector(locations);

    kjb_cleanup();

    return 0;
}
