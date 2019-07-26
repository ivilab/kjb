#include <slic/basic.h>
#include <slic/image_interp.h>
#include <slic/homography.h>

int back_project_image
(
 const KJB_image *ip,
 const Matrix    *H_mp,
 int             fitting_model,
 KJB_image       **bp_ipp,
 Int_matrix      **mask_impp
 );

int main()
{
    int        result = NO_ERROR;
    KJB_image  *src_ip = NULL;
    KJB_image  *target_ip = NULL;
    Matrix     *H_mp = NULL;
    Int_matrix *mask_imp = NULL;
    FILE       *fp = NULL;
    /* const char *H_filename = "data/frame-to-frame-H/000663"; */
    /* const char *src_filename = "data/frames/000663.jpg"; */
    /* const char *target_filename = "out/000663_bp.jpg"; */
    const char *H_filename = "data/frame-to-slide-H/checkerboard_04_00300_003_m";
    const char *src_filename = "data/frames/checkerboard_04_001575.png";
    const char *target_filename = "out/checkerboard_04_001575_001_bp.png";
    /*
    const char *H_filename = "data/frame-to-frame-H/000517";
    const char *src_filename = "data/frames/000517.jpg";
    const char *target_filename = "out/000517_bp.jpg";
*/

    kjb_init(); 

    if (! is_interactive()) 
    {
        p_stderr("This test program only works interactively.\n");
        return EXIT_CANNOT_TEST;
    }

    fp = kjb_fopen(H_filename, "r");
    if ( fp == NULL )
    {
        warn_pso(" Unable to open homography file '%s'.\n", H_filename);
        kjb_fclose( fp );
        EGC(ERROR);
    }

    result = fp_read_matrix( &H_mp, fp );
    if (result == ERROR) { EGC(result); }
    kjb_fclose( fp );

    db_mat( H_mp );

    pso( "Reading the source image: %s.\n", src_filename);
    result = kjb_read_image( &src_ip, src_filename );
    if (result == ERROR) { EGC(result); }

    pso( "Backprojecting the image.\n");
    result = back_project_image( src_ip, H_mp, HOMOGRAPHY, &target_ip, &mask_imp );
    if (result == ERROR) { EGC(result); }

    pso( "Saving the backprojected image: %s.\n", target_filename);
    result = kjb_write_image( target_ip, target_filename );
    if (result == ERROR) { EGC(result); }


cleanup:
    EPE(result);

    pso("Cleaning up.\n");
    kjb_free_image( src_ip );
    kjb_free_image( target_ip );
    free_matrix( H_mp );
    free_int_matrix( mask_imp );
    pso("Finished.\n");

    return EXIT_SUCCESS;
}


int back_project_image
(
 const KJB_image *ip,
 const Matrix    *H_mp,
 int             fitting_model,
 KJB_image       **bp_ipp,
 Int_matrix      **mask_impp
 )
{ 
    Matrix *img_pos_mp = NULL;
    Matrix *slide_bound_mp = NULL;
    Int_matrix *mask_imp = NULL;
    int result;

    result = get_bound(H_mp, 
                       /*homography_inverse,*/
                       homography_transform,
                       0,  
                       0,  
                       ip->num_cols,
                       ip->num_rows,
                       &img_pos_mp);

    if(result != ERROR)
    {   
        result = get_slide_bound(img_pos_mp, ip->num_cols, 
                                 ip->num_rows, &slide_bound_mp);
    }   

    /* back project images */
    if(result != ERROR)
    {   
        result = transform_image(ip, H_mp, ip->num_cols, ip->num_rows,
                                 slide_bound_mp, fitting_model, bp_ipp, &mask_imp);
    }   

    if(result != ERROR && mask_impp != NULL)
    {   
        ERE(copy_int_matrix(mask_impp, mask_imp));
    }   

    free_matrix(img_pos_mp);
    free_matrix(slide_bound_mp);
    free_int_matrix(mask_imp);

    return  result;
}
