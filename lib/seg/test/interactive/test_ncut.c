
/* $Id: test_ncut.c 21491 2017-07-20 13:19:02Z kobus $ */


/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003, by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               ||
|        Kobus Barnard.                                                        |
|        Prasad Gabbur.                                                        |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */

#include "i/i_incl.h"
#include "i/i_float_io.h" 
#include "seg/seg_incl.h" 

/* -------------------------------------------------------------------------- */


int main(int argc, char** argv)
{

    /* *************************************************
     * Test NCuts code.

    Matrix* Weight_mp = NULL;
    Vector* Soft_bipart_vp = NULL;
    int i;
    */
    
    /* *************************************************
     * Test connected component labeling code.
    */ 
    KJB_image* input_ip  = NULL;
    KJB_image* output_ip = NULL;

    Int_matrix* input_mp  = NULL;
    Int_matrix* output_mp = NULL;

    KJB_region_list* region_list_rlp  = NULL;
    Segmentation_t3* segmentation_ptr = NULL;

    int num_rows, num_cols;
    int num_connected_regions;
    int max_region_label;
    int i, j;


    kjb_init();   /* Best to do this if using KJB library. */

    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    /* *************************************************
     * Test NCuts code.
    ERE(get_zero_matrix(&Weight_mp, 4, 4));
    
    Weight_mp->elements[0][0] = 1.0;
    Weight_mp->elements[1][1] = 1.0;
    Weight_mp->elements[2][2] = 1.0;
    Weight_mp->elements[3][3] = 1.0;
    Weight_mp->elements[0][1] = 1.0;
    Weight_mp->elements[1][0] = 1.0;
    Weight_mp->elements[2][3] = 0.0;
    Weight_mp->elements[3][2] = 0.0;
    Weight_mp->elements[0][2] = 1.0;
    Weight_mp->elements[1][2] = 1.0;
  
    ERE(ncut_dense_bipartition(&Soft_bipart_vp, Weight_mp));

    for (i = 0; i < 4; i++)
    {
        pso("Soft_bipart_vp[%d] = %f\n", i, Soft_bipart_vp->elements[i]);
    }

    free_matrix(Weight_mp);
    free_vector(Soft_bipart_vp);
    */

    
    /* *************************************************
     * Test connected component labeling code.
    */

    if (argc != 3)
    {
        set_error("Insufficient number of arguments:\nUsage: test_ncut <input_image> <output_image>");
        kjb_print_error();
        return EXIT_FAILURE;
    }
 
    EPETE(kjb_read_image_2(&input_ip, argv[1]));

    num_rows = input_ip->num_rows;
    num_cols = input_ip->num_cols;
    
    EPETE(get_zero_int_matrix(&input_mp, num_rows, num_cols));
    
    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            float r = input_ip->pixels[i][j].r;
            float g = input_ip->pixels[i][j].g;
            float b = input_ip->pixels[i][j].b;

            input_mp->elements[i][j] = (int) (r+g+b/3.0);
        }
    }
    
    /* OLD CODE.
    regions_rp = (KJB_region*) KJB_MALLOC(num_rows*num_cols*sizeof(KJB_region));
    */

    /*
    EPETE( label_four_connected_regions(&output_mp,
                                        &region_list_rlp,
                                        &segmentation_ptr,
                                        &num_connected_regions,
                                        &max_region_label,
                                        input_ip,
                                        input_mp,
                                        1) );
    */

    /*
    EPETE( label_four_connected_regions(&output_mp,
                                        NULL,
                                        NULL,
                                        &num_connected_regions,
                                        &max_region_label,
                                        NULL,
                                        input_mp,
                                        0) );
    */

    EPETE( label_eight_connected_regions(&output_mp,
                                         NULL,
                                         NULL,
                                         &num_connected_regions,
                                         &max_region_label,
                                         NULL,
                                         input_mp,
                                         0) );

    /* Only for allocating output_ip. */
    EPETE(kjb_copy_image(&output_ip, input_ip));
    
    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            float gray_value = (255.0f / num_connected_regions) * output_mp->elements[i][j]; 
            
            output_ip->pixels[i][j].r = gray_value;
            output_ip->pixels[i][j].g = gray_value;
            output_ip->pixels[i][j].b = gray_value;
        }
    }

    EPETE(kjb_write_image(output_ip, argv[2]));

    kjb_free_image(input_ip);
    kjb_free_image(output_ip);
    free_int_matrix(input_mp);
    free_int_matrix(output_mp);
    free_region_list(region_list_rlp);
    t3_free_segmentation(segmentation_ptr);
    
    kjb_cleanup(); /* Almost never needed, but doing it twice is OK. */

    return EXIT_SUCCESS; 
} 

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
