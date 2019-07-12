#include <slic/basic.h>
#include <slic/image_interp.h>
#include <slic/homography.h>
#include <m/m_incl.h>

static int back_project_image
(
   const KJB_image *ip,
   const Matrix    *H_mp,
   int             fitting_model,
   KJB_image       **bp_ipp,
   Int_matrix      **mask_impp
);

static int transform_scaled_image
(
    const KJB_image *src_ip,
    const Matrix    *t_mp,
    int             width,
    int             height,
    const Matrix     *trans_rect_mp,
    int             inter_method,
    KJB_image       **target_ipp,
    Int_matrix      **mask_impp
);

/* =========================================================================== *
   This program computes the transformation for the rotated camera, then 
   backprojects and saves an input image that was rotated by \theta degrees.
* =========================================================================== */

int main()
{
    int        result = NO_ERROR;
    KJB_image  *src_ip = NULL;
    KJB_image  *target_ip = NULL;
    Matrix     *H_mp = NULL;
    Matrix     *inv_tmp_mp = NULL;
    Matrix     *tmp_mp = NULL;
    Matrix     *tmp1_mp = NULL;
    Matrix     *result_mp = NULL;
    Matrix     *V_P_mp = NULL;
    Matrix     *V_C_mp = NULL;
    Matrix     *E_C_mp = NULL;
    Matrix     *E_P_mp = NULL;
    Matrix     *T_P_mp = NULL;
    Matrix     *T_C_mp = NULL;
    Matrix     *R_mp = NULL;
    Matrix     *N_mp = NULL;
    Matrix     *L_mp = NULL;
    Int_matrix *mask_imp = NULL;
    /*const char *result_filename = "/space/ykk/2010-05-24-sally-aitken-01/out/011701_rotated.jpg";*/
    /*const char *src_filename = "/space/ykk/2010-05-24-sally-aitken-01/input/slide_img/008.jpg";*/
    const char *src_filename = "data/slides/008.jpg";
    const char *result_filename = "out/011701_rotated.jpg";
    /*double PI = 3.1415928;*/
    double PI = M_PI;
    /*double theta = PI/180.0; */
    double theta = -PI/5.0; 
    /*double theta = PI/4.0; */
    /*double theta = 0.2*PI/180.0; */
    /*double theta = 10*PI/180.0; */
    /*double theta = PI/1.9; */
    /*double theta = 0; */
    /*double theta = PI; */

    double d_C_w = 3;
    double P_z = 1;
    double C_x;
    double C_z;

    int             width;
    int             height;
    int i;
    int j;

    Matrix *offset_mp = NULL;
    Matrix *i_offset_mp = NULL;
    Matrix *coord_mp = NULL;
    Matrix *scale_mp = NULL;

    kjb_init(); 

    if (! is_interactive()) 
    {
        p_stderr("This test program only works interactively.\n");
        return EXIT_CANNOT_TEST;
    }
    
    /* Compute the camera center location */
    C_x = d_C_w * sin(theta);
    C_z = d_C_w * cos(theta);

    pso("d_C = %f\n", d_C_w);
    pso("theta = %f\n", theta);
    /*pso("C_x = %f, C_z = %f\n", C_x, C_z);*/
    /* Initialize the matrices */
    result = get_zero_matrix( &N_mp, 3, 4 );
    if (result == ERROR) { EGC(result); }
    N_mp->elements[0][0] = 1;
    N_mp->elements[1][1] = 1;
    N_mp->elements[2][2] = 1;
    /*db_mat(N_mp);*/

    result = get_zero_matrix( &L_mp, 4, 3 );
    if (result == ERROR) { EGC(result); }
    L_mp->elements[0][0] = 1;
    L_mp->elements[1][1] = 1;
    L_mp->elements[3][2] = 1;
    /*db_mat(L_mp);*/

    result = get_identity_matrix( &T_P_mp, 4);
    if (result == ERROR) { EGC(result); }
    T_P_mp->elements[2][3] = -P_z;
    /*db_mat(T_P_mp);*/
   
    /* Flip the X and the Z axes */
    result = get_identity_matrix( &R_mp, 4);
    if (result == ERROR) { EGC(result); }
    R_mp->elements[0][0] = -1;
    R_mp->elements[2][2] = -1;
    /*db_mat(R_mp);*/

    result = multiply_matrices( &E_P_mp, R_mp, T_P_mp );
    if (result == ERROR) { EGC(result); }
    /*db_mat(E_P_mp);*/

    /* Compute V_p */
    result = multiply_matrices( &tmp_mp, N_mp, E_P_mp );
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &V_P_mp, tmp_mp, L_mp );
    if (result == ERROR) { EGC(result); }
    /*db_mat(V_P_mp);*/


    result = get_identity_matrix( &T_C_mp, 4);
    if (result == ERROR) { EGC(result); }
    T_C_mp->elements[0][3] = -C_x;
    T_C_mp->elements[2][3] = -C_z;
    /*db_mat(T_C_mp);*/

    result = get_identity_matrix( &R_mp, 4);
    if (result == ERROR) { EGC(result); }
    R_mp->elements[0][0] = -cos(theta);
    R_mp->elements[0][2] = sin(theta);
    R_mp->elements[2][0] = -sin(theta);
    R_mp->elements[2][2] = -cos(theta);
    /*db_mat(R_mp);*/

    result = multiply_matrices( &E_C_mp, R_mp, T_C_mp );
    if (result == ERROR) { EGC(result); }
    /*db_mat(E_C_mp);*/

    /* Now, compute V_c */
    result = multiply_matrices( &tmp_mp, N_mp, E_C_mp );
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &V_C_mp, tmp_mp, L_mp );
    if (result == ERROR) { EGC(result); }
    /*db_mat(V_C_mp);*/
   
    /*pso( " Get the inverse \n");*/
    result = get_matrix_inverse(&inv_tmp_mp, V_P_mp);
    if (result == ERROR) { EGC(result); }
    /*db_mat( inv_tmp_mp );*/
    result = copy_matrix( &V_P_mp, inv_tmp_mp );
    if (result == ERROR) { EGC(result); }
    
    /*pso( "Multiply matrices \n");*/
    result = multiply_matrices( &result_mp, V_C_mp, V_P_mp);
    if (result == ERROR) { EGC(result); }
    /*db_mat( result_mp );*/
 
    pso( "Reading the source image: %s\n", src_filename);
    result = kjb_read_image( &src_ip, src_filename );
    if (result == ERROR) { EGC(result); }

    /* compute the offset O */
    /* Move the center into the image center */
    /* Scale the image to 2x2 */
    /* then calculate O^(-1) S^(-1) * H * S * O */
    width = src_ip->num_cols;
    height = src_ip->num_rows;
    
    result = get_identity_matrix( &scale_mp, 3 );
    if (result == ERROR) { EGC(result); }
    scale_mp->elements[0][0] = 2.0/ (double)width;
    scale_mp->elements[1][1] = 2.0/ (double)height;

    ERE( get_identity_matrix( &offset_mp, 3) );
    offset_mp->elements[0][2] = -width/2.0;
    offset_mp->elements[1][2] = -height/2.0;

    result = ( get_matrix_inverse( &i_offset_mp, offset_mp) );
    if (result == ERROR) { EGC(result); }

    result = get_matrix_inverse( &inv_tmp_mp, scale_mp );
    if (result == ERROR) { EGC(result); }
    
    result = multiply_matrices( &tmp_mp, scale_mp, offset_mp);
    if (result == ERROR) { EGC(result); }
    EGC( multiply_matrices( &tmp1_mp, result_mp, tmp_mp ));
    EGC( multiply_matrices( &tmp_mp, inv_tmp_mp, tmp1_mp));
    EGC( multiply_matrices( &result_mp, i_offset_mp, tmp_mp ));
    if (result == ERROR) { EGC(result); }
    
    /* SANITY CHECK: Check the coordinates of the corners and the center of the image */
    result = get_target_matrix( &coord_mp, 3, 5 );
    if ( result == ERROR ) { EGC(result); }
    /* (0, 0, 1) */
    coord_mp->elements[0][0] = 0;
    coord_mp->elements[1][0] = 0;
    coord_mp->elements[2][0] = 1;
    /* (w, h, 1) */
    coord_mp->elements[0][1] = width;
    coord_mp->elements[1][1] = height;
    coord_mp->elements[2][1] = 1;
    /* (0, h, 1) */
    coord_mp->elements[0][2] = 0;
    coord_mp->elements[1][2] = height;
    coord_mp->elements[2][2] = 1;
    /* (w, 0, 1) */
    coord_mp->elements[0][3] = width;
    coord_mp->elements[1][3] = 0;
    coord_mp->elements[2][3] = 1;
    /* center */
    coord_mp->elements[0][4] = width/2.0;
    coord_mp->elements[1][4] = height/2.0;
    coord_mp->elements[2][4] = 1;
    db_mat(coord_mp);

    EGC( multiply_matrices( &tmp_mp, result_mp, coord_mp ) );
    /* Scale by the last coordinate */
    /*db_mat( tmp_mp );*/
    for (i = 0; i < coord_mp->num_cols; i++)
    {
        for (j=0; j < coord_mp->num_rows; j++)
        {
            tmp_mp->elements[j][i] /= tmp_mp->elements[2][i];
        }
    }
    db_mat( tmp_mp );

    pso( "Backprojecting the image.\n");
    result = back_project_image( src_ip, result_mp, HOMOGRAPHY, &target_ip, &mask_imp );
    if (result == ERROR) { EGC(result); }

    pso( "Saving the backprojected image: %s\n", result_filename);
    result = kjb_write_image( target_ip, result_filename );
    if (result == ERROR) { EGC(result); }

cleanup:
    EPE(result);

    pso("Cleaning up.\n");
    kjb_free_image( src_ip );
    kjb_free_image( target_ip );
    free_matrix( H_mp );
    free_matrix( inv_tmp_mp );
    free_matrix( tmp_mp );
    free_matrix( result_mp );
    free_matrix( V_C_mp );
    free_matrix( V_P_mp );
    free_matrix( E_C_mp );
    free_matrix( E_P_mp );
    free_matrix( T_P_mp );
    free_matrix( T_C_mp );
    free_matrix( R_mp );
    free_matrix( N_mp );
    free_matrix( L_mp );
    free_int_matrix( mask_imp );

    free_matrix(i_offset_mp);
    free_matrix(offset_mp);
    free_matrix(coord_mp);
    free_matrix(scale_mp);
    free_matrix( tmp1_mp );

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

    db_mat( slide_bound_mp );

    /* back project images */
    if(result != ERROR)
    {   
        result = transform_image(ip, H_mp, ip->num_cols, ip->num_rows,
                                 slide_bound_mp, fitting_model, bp_ipp, &mask_imp);
    }   

    if(result != ERROR && mask_impp != NULL)
    {   
        EGC(copy_int_matrix(mask_impp, mask_imp));
    }   

cleanup:
    EPE(result);
    free_matrix(img_pos_mp);
    free_matrix(slide_bound_mp);
    free_int_matrix(mask_imp);

    return  result;
}
