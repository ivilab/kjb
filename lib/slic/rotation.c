/* $Id$ */
/* =========================================================================== *
 * Original code is in /slic/test/rotate.c
 * =========================================================================== */
#include "slic/rotation.h"
#include "n/n_invert.h"

int compute_rotation
(
    double angle,
    double d_C_w,
    double P_z ,
    int    width,
    int    height,
    Matrix **T_mpp
)
{
    int result = NO_ERROR;
    double theta = 0; 
    double C_x;
    double C_z;
    Matrix     *inv_tmp_mp = NULL;
    Matrix     *tmp_mp = NULL;
    Matrix     *tmp1_mp = NULL;
    Matrix     *result_mp = NULL;
    Matrix     *V_P_mp = NULL;
    Matrix     *V_C_mp = NULL;
    Matrix     *E_C_mp = NULL;
    Matrix     *E_P_mp = NULL;
    Matrix     *T_Pw_mp = NULL;
    Matrix     *T_Cw_mp = NULL;
    Matrix     *R_mp = NULL;
    /*Matrix     *R_pi_mp = NULL;*/
    Matrix     *N_mp = NULL;
    Matrix     *L_mp = NULL;

    Matrix *offset_mp = NULL;
    Matrix *i_offset_mp = NULL;
    Matrix *coord_mp = NULL;
    Matrix *scale_mp = NULL;
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

    result = get_identity_matrix( &T_Pw_mp, 4);
    if (result == ERROR) { EGC(result); }
    T_Pw_mp->elements[2][3] = -P_z;
    /*db_mat(T_Pw_mp);*/

    /* Flip the X and the Z axes */
    result = get_identity_matrix( &R_mp, 4);
    if (result == ERROR) { EGC(result); }
    R_mp->elements[0][0] = -1;
    R_mp->elements[2][2] = -1;
    /*db_mat(R_mp);*/

    result = multiply_matrices( &E_P_mp, R_mp, T_Pw_mp );
    if (result == ERROR) { EGC(result); }
    /*db_mat(E_P_mp);*/

    /* Compute V_p */
    result = multiply_matrices( &tmp_mp, N_mp, E_P_mp );
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &V_P_mp, tmp_mp, L_mp );
    if (result == ERROR) { EGC(result); }
    /*db_mat(V_P_mp);*/
    /* -------------------------------------------------- */
    theta = M_PI * angle/180.0 ; /* convert to radians */
    /* Compute the camera center location */
    C_x = d_C_w * sin(theta);
    C_z = d_C_w * cos(theta);

    verbose_pso(6, "d_C = %f\n", d_C_w);
    verbose_pso(6, "angle = %f\n", angle);
    verbose_pso(6, "theta = %f\n", theta);
    /*pso("C_x = %f, C_z = %f\n", C_x, C_z);*/

    result = get_identity_matrix( &T_Cw_mp, 4);
    if (result == ERROR) { EGC(result); }
    T_Cw_mp->elements[0][3] = -C_x;
    T_Cw_mp->elements[2][3] = -C_z;
    /*db_mat(T_Cw_mp);*/

    result = get_identity_matrix( &R_mp, 4);
    if (result == ERROR) { EGC(result); }
    /* Use the negative angle, which means the transpose of the original rotation matrix */
    R_mp->elements[0][0] = cos(theta);
    R_mp->elements[0][2] = -sin(theta);
    R_mp->elements[2][0] = sin(theta);
    R_mp->elements[2][2] = cos(theta);
    /*db_mat(R_mp);*/

    result = multiply_matrices( &E_C_mp, R_mp, T_Cw_mp );
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

    /*verbose_pso(5, " | Reading the source image: %s\n", src_filename);*/
    /*result = kjb_read_image( &src_ip, src_filename );*/
    /*if (result == ERROR) { EGC(result); }*/

    /* compute the offset O */
    /* Move the center into the image center */
    /* Scale the image to 2x2 */
    /* then calculate O^(-1) S^(-1) * H * S * O */
    /*width = src_ip->num_cols;*/
    /*height = src_ip->num_rows;*/

    result = get_identity_matrix( &scale_mp, 3 );
    if (result == ERROR) { EGC(result); }
 /*
    scale_mp->elements[0][0] = 2.0/ (double)width;
    scale_mp->elements[1][1] = 2.0/ (double)height;
    */
    scale_mp->elements[0][0] = 2.0/ (double)height;
    scale_mp->elements[1][1] = 2.0/ (double)width;

    ERE( get_identity_matrix( &offset_mp, 3) );
   /*
    offset_mp->elements[0][2] = -(double)width/2.0;
    offset_mp->elements[1][2] = -(double)height/2.0;
    */
    offset_mp->elements[0][2] = -(double)height/2.0;
    offset_mp->elements[1][2] = -(double)width/2.0;


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

    result = copy_matrix( T_mpp, result_mp );
    if (result == ERROR) { EGC(result); }

cleanup:
    free_matrix( inv_tmp_mp );
    free_matrix( tmp_mp );
    free_matrix( result_mp );
    free_matrix( V_C_mp );
    free_matrix( V_P_mp );
    free_matrix( E_C_mp );
    free_matrix( E_P_mp );
    free_matrix( T_Pw_mp );
    free_matrix( T_Cw_mp );
    free_matrix( R_mp );
    free_matrix( N_mp );
    free_matrix( L_mp );

    free_matrix(i_offset_mp);
    free_matrix(offset_mp);
    free_matrix(coord_mp);
    free_matrix(scale_mp);
    free_matrix( tmp1_mp );

    return result;
}


int compute_rotation_set_focal_length
(
    double angle,
    double d_C_w,
    double P_z ,
    double focal_length,
    int    width,
    int    height,
    Matrix **T_mpp
)
{
    int result = NO_ERROR;
    double theta = 0; 
    double C_x;
    double C_z;
    
    double new_scale = 0;
    double max_width = 0;
    double cur_width = 0;
    double max_height = 0;
    double cur_height = 0;
    double m_scale = 0;
    int num_corners = 4;
    int corner = 0;
    
    Matrix     *inv_tmp_mp = NULL;
    Matrix     *tmp_mp = NULL;
    Matrix     *tmp1_mp = NULL;
    Matrix     *result_mp = NULL;
    Matrix     *V_P_mp = NULL;
    Matrix     *inv_V_P_mp = NULL;
    Matrix     *V_C_mp = NULL;
    Matrix     *inv_V_C_mp = NULL;
    Matrix     *E_C_mp = NULL;
    Matrix     *E_P_mp = NULL;
    Matrix     *Tr_P_mp = NULL;
    Matrix     *Tr_C_mp = NULL;
    Matrix     *T_Pw_mp = NULL;
    Matrix     *T_Cw_mp = NULL;
    Matrix     *R_mp = NULL;
    Matrix     *R_pi_mp = NULL;
    Matrix     *N_mp = NULL;
    Matrix     *L_mp = NULL;
    Matrix     *K_C_mp = NULL;
    Matrix     *K_P_mp = NULL;
    Matrix     *inv_K_P_mp = NULL;
    Matrix     *S_P_mp = NULL;
    Matrix     *S_C_mp = NULL;
    Matrix     *H_mp = NULL;

    Matrix *axis_flip_C_mp = NULL;
    Matrix *axis_flip_P_mp = NULL;

    Matrix *offset_mp = NULL;
    Matrix *i_offset_mp = NULL;
    Matrix *coord_mp = NULL;
    Matrix *scale_mp = NULL;
    Matrix *scale_undo_mp = NULL;
    Matrix *corner_coord_mp = NULL;
    Matrix *new_coord_mp = NULL;
    Matrix *bp_coord_mp = NULL;
    /* Initialize the matrices */
    /*
    result = get_zero_matrix( &K_C_mp, 3, 3 );
    if (result == ERROR) { EGC(result); }
    K_C_mp->elements[0][0] = focal_length;
    K_C_mp->elements[1][1] = focal_length;
    K_C_mp->elements[2][2] = 1;
    */

    result = get_zero_matrix( &N_mp, 3, 4 );
    if (result == ERROR) { EGC(result); }
    N_mp->elements[0][0] = focal_length;
    N_mp->elements[1][1] = focal_length;
    N_mp->elements[2][2] = 1;
    /*db_mat(N_mp);*/

    result = get_zero_matrix( &L_mp, 4, 3 );
    if (result == ERROR) { EGC(result); }
    L_mp->elements[0][0] = 1;
    L_mp->elements[1][1] = 1;
    L_mp->elements[3][2] = 1;
    /*db_mat(L_mp);*/

    /* Intrinsics */
    result = get_identity_matrix( &Tr_P_mp, 3);
    if (result == ERROR) { EGC(result); }
    /*
    Tr_P_mp->elements[0][3] = -width/2.0;;
    Tr_P_mp->elements[1][3] = -height/2.0;;
    */
    Tr_P_mp->elements[1][2] = -width/2.0;;
    Tr_P_mp->elements[0][2] = -height/2.0;;
    db_mat(Tr_P_mp);

    result = get_identity_matrix( &S_P_mp, 3);
    if (result == ERROR) { EGC(result); }
    S_P_mp->elements[1][1] = 2.0/width;
    S_P_mp->elements[0][0] = 2.0/height;
    db_mat(S_P_mp);

    result = get_zero_matrix( &axis_flip_P_mp, 3, 3);
    if (result == ERROR) { EGC(result); }
    axis_flip_P_mp->elements[0][1] = 1;
    axis_flip_P_mp->elements[1][0] = -1;
    axis_flip_P_mp->elements[2][2] = 1;
    db_mat(axis_flip_P_mp);
    
    result = get_matrix_inverse( &axis_flip_C_mp, axis_flip_P_mp);
    if (result == ERROR) { EGC(result); }
    db_mat(axis_flip_C_mp);

    /*result = multiply_matrices( &K_P_mp, Tr_P_mp, S_P_mp );*/
    result = multiply_matrices( &tmp_mp, S_P_mp, Tr_P_mp );
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &K_P_mp, axis_flip_P_mp, tmp_mp );
    if (result == ERROR) { EGC(result); }
    db_mat(K_P_mp);

    result = get_matrix_inverse( &Tr_C_mp, Tr_P_mp );
    if (result == ERROR) { EGC(result); }
    db_mat(Tr_C_mp);
    
    result = get_matrix_inverse( &S_C_mp, S_P_mp );
    if (result == ERROR) { EGC(result); }

    /*result = multiply_matrices( &K_C_mp, Tr_C_mp, S_C_mp );*/
    result = multiply_matrices( &tmp_mp, S_C_mp, axis_flip_C_mp );
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &K_C_mp, Tr_C_mp, tmp_mp );
    if (result == ERROR) { EGC(result); }
    db_mat(K_C_mp);

    /* Extrinsics */
    result = get_identity_matrix( &T_Pw_mp, 4);
    if (result == ERROR) { EGC(result); }
    T_Pw_mp->elements[2][3] = -P_z;
    /*db_mat(T_Pw_mp);*/

    /* Flip the X and the Z axes */
    result = get_identity_matrix( &R_pi_mp, 4);
    if (result == ERROR) { EGC(result); }
    R_pi_mp->elements[0][0] = -1;
    R_pi_mp->elements[2][2] = -1;
    /*db_mat(R_mp);*/

    result = multiply_matrices( &E_P_mp, R_pi_mp, T_Pw_mp );
    if (result == ERROR) { EGC(result); }
    /*db_mat(E_P_mp);*/

    /* Compute V_p */
    result = multiply_matrices( &tmp_mp, N_mp, E_P_mp );
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &V_P_mp, tmp_mp, L_mp );
    if (result == ERROR) { EGC(result); }
    db_mat(V_P_mp);
    /* -------------------------------------------------- */
    theta = M_PI * angle/180.0 ; /* convert to radians */
    /* Compute the camera center location */
    C_x = d_C_w * sin(theta);
    C_z = d_C_w * cos(theta);

    verbose_pso(6, "d_C = %f\n", d_C_w);
    verbose_pso(6, "angle = %f\n", angle);
    verbose_pso(6, "theta = %f\n", theta);
    pso("C_x = %f, C_z = %f\n", C_x, C_z);

    result = get_identity_matrix( &T_Cw_mp, 4);
    if (result == ERROR) { EGC(result); }
    T_Cw_mp->elements[0][3] = -C_x;
    T_Cw_mp->elements[2][3] = -C_z;
    db_mat(T_Cw_mp);

    result = get_identity_matrix( &R_mp, 4);
    if (result == ERROR) { EGC(result); }
    R_mp->elements[0][0] = cos(theta);
    R_mp->elements[0][2] = -sin(theta);
    R_mp->elements[2][0] = sin(theta);
    R_mp->elements[2][2] = cos(theta);
    db_mat(R_mp);

    /*result = multiply_matrices( &E_C_mp, R_mp, T_Cw_mp );*/
    result = multiply_matrices( &tmp_mp, R_pi_mp, R_mp );
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &E_C_mp, tmp_mp, T_Cw_mp );
    if (result == ERROR) { EGC(result); }
    db_mat(E_C_mp);

    /* Now, compute V_c */
    result = multiply_matrices( &tmp_mp, N_mp, E_C_mp );
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &V_C_mp, tmp_mp, L_mp );
    if (result == ERROR) { EGC(result); }
    db_mat(V_C_mp);

    /*pso( " Get the inverse \n");*/
    result = get_matrix_inverse(&inv_V_P_mp, V_P_mp);
    if (result == ERROR) { EGC(result); }

    result = get_matrix_inverse(&inv_K_P_mp, K_P_mp);
    if (result == ERROR) { EGC(result); }

     /* Compute the homography */
    result = multiply_matrices( &tmp_mp, K_C_mp, V_C_mp);
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &result_mp, tmp_mp, inv_V_P_mp);
    if (result == ERROR) { EGC(result); }
    /*result = multiply_matrices( &H_mp, result_mp, inv_K_P_mp);*/
    result = multiply_matrices( &H_mp, result_mp, K_P_mp);
    if (result == ERROR) { EGC(result); }

    /* Peek at where the corners will end up, and 
       scale the result to fit the longest side 
       within the resulting image */

    result = get_zero_matrix( &corner_coord_mp, 3, num_corners);
    if (result == ERROR) { EGC(result); }

    result = get_zero_matrix( &bp_coord_mp, 3, num_corners);
    if (result == ERROR) { EGC(result); }

    for (corner = 0; corner < num_corners; corner++)
    {
        corner_coord_mp->elements[2][corner] = 1; /* homogeneous coordinates */
    }
    /*  
        0   w   0   w
        0   0   h   h
        1   1   1   1
     */
    /*
    corner_coord_mp->elements[1][2] = height-1;
    corner_coord_mp->elements[1][3] = height-1;
    corner_coord_mp->elements[0][1] = width-1;
    corner_coord_mp->elements[0][3] = width-1;
    */

    /*  
        0   0   h   h
        0   w   0   w
        1   1   1   1
     */
    corner_coord_mp->elements[1][1] = width-1;
    corner_coord_mp->elements[1][3] = width-1;
    corner_coord_mp->elements[0][2] = height-1;
    corner_coord_mp->elements[0][3] = height-1;

    /*result = multiply_matrices(&new_coord_mp, result_mp, corner_coord_mp);*/
    result = multiply_matrices(&new_coord_mp, H_mp, corner_coord_mp);
    if (result == ERROR) { EGC(result); }

    /*db_mat( corner_coord_mp );*/
    /*db_mat( new_coord_mp );*/

    for (corner = 0; corner < num_corners; corner++)
    {
        bp_coord_mp->elements[0][corner] = new_coord_mp->elements[0][corner]/new_coord_mp->elements[2][corner]; 
        bp_coord_mp->elements[1][corner] = new_coord_mp->elements[1][corner]/new_coord_mp->elements[2][corner]; 
        bp_coord_mp->elements[2][corner] = 1; 
    }
    db_mat(bp_coord_mp);

    /* Find the maximum current height */
    for (corner = 0; corner < num_corners; corner++)
    {
        /*
        cur_width = (bp_coord_mp->elements[0][corner]);
        cur_height = (bp_coord_mp->elements[1][corner]);
        */
        cur_width = (bp_coord_mp->elements[1][corner]);
        cur_height = (bp_coord_mp->elements[0][corner]);
        max_height = fabs((double)height/2.0 - cur_height)/height;
        max_width = fabs((double)width/2.0 - cur_width)/width;
        if (m_scale < max_height)
        {
            m_scale = max_height;
        }
        if (m_scale < max_width)
        {
            m_scale = max_width;
        }
    }

    /* NOW, adjust the scale and recompute the result */
    result = get_identity_matrix( &scale_undo_mp, 3 );
    if (result == ERROR) { EGC(result); }
    new_scale = 0.45 / m_scale ;
    verbose_pso(5, "scale = %f\n", new_scale);
    scale_undo_mp->elements[0][0] = ((double)height)/2.0 * new_scale;
    scale_undo_mp->elements[1][1] = ((double)width)/2.0 * new_scale;
    db_mat( scale_undo_mp );
/*
    result = ( get_matrix_inverse( &i_offset_mp, offset_mp) );
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &tmp_mp, scale_undo_mp, result_mp);
    if (result == ERROR) { EGC(result); }
    EGC( multiply_matrices( &result_mp, i_offset_mp, tmp_mp ));
*/
    /*
    result = multiply_matrices( &K_C_mp, Tr_C_mp, scale_undo_mp );
    if (result == ERROR) { EGC(result); }
    */
    /*result = multiply_matrices( &tmp_mp, S_P_mp, axis_flip_C_mp );*/
    result = multiply_matrices( &tmp_mp, scale_undo_mp, axis_flip_C_mp );
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &K_C_mp, Tr_C_mp, tmp_mp );
    if (result == ERROR) { EGC(result); }
    db_mat(K_C_mp);

     /* Compute the homography */
    result = multiply_matrices( &tmp_mp, K_C_mp, V_C_mp);
    if (result == ERROR) { EGC(result); }
    result = multiply_matrices( &result_mp, tmp_mp, inv_V_P_mp);
    if (result == ERROR) { EGC(result); }
    /*result = multiply_matrices( &H_mp, result_mp, inv_K_P_mp);*/
    result = multiply_matrices( &H_mp, result_mp, K_P_mp);
    if (result == ERROR) { EGC(result); }

    result = copy_matrix( &result_mp, H_mp );
    if (result == ERROR) { EGC(result); }

    result = multiply_matrices(&new_coord_mp, result_mp, corner_coord_mp);
    if (result == ERROR) { EGC(result); }

    /*db_mat( corner_coord_mp );*/
    /*db_mat( new_coord_mp );*/

    for (corner = 0; corner < num_corners; corner++)
    {
        bp_coord_mp->elements[0][corner] = new_coord_mp->elements[0][corner]/new_coord_mp->elements[2][corner]; 
        bp_coord_mp->elements[1][corner] = new_coord_mp->elements[1][corner]/new_coord_mp->elements[2][corner]; 
        bp_coord_mp->elements[2][corner] = 1; 
    }
    db_mat(bp_coord_mp);


    /* Save the final result */
    result = copy_matrix( T_mpp, result_mp );
    if (result == ERROR) { EGC(result); }

cleanup:
    free_matrix( inv_tmp_mp );
    free_matrix( tmp_mp );
    free_matrix( result_mp );
    free_matrix( V_C_mp );
    free_matrix( inv_V_C_mp );
    free_matrix( V_P_mp );
    free_matrix( inv_V_P_mp );
    free_matrix( E_C_mp );
    free_matrix( E_P_mp );
    free_matrix( T_Pw_mp );
    free_matrix( Tr_P_mp );
    free_matrix( Tr_C_mp );
    free_matrix( T_Cw_mp );
    free_matrix( R_mp );
    free_matrix( R_pi_mp );
    free_matrix( N_mp );
    free_matrix( L_mp );
    free_matrix( K_C_mp );
    free_matrix( K_P_mp );
    free_matrix( inv_K_P_mp );
    free_matrix( S_P_mp );
    free_matrix( S_C_mp );
    free_matrix( H_mp );
    
    free_matrix( axis_flip_C_mp );
    free_matrix( axis_flip_P_mp );

    free_matrix(corner_coord_mp);
    free_matrix(new_coord_mp);
    free_matrix(bp_coord_mp);
    free_matrix(i_offset_mp);
    free_matrix(offset_mp);
    free_matrix(coord_mp);
    free_matrix(scale_mp);
    free_matrix( scale_undo_mp );
    free_matrix( tmp1_mp );

    return result;
}
