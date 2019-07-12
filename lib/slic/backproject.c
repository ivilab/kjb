#include <slic/basic.h>
#include <slic/image_interp.h>
#include <slic/homography.h>
#include <slic/backproject.h>
#include "m/m_mat_arith.h"
#include "l/l_def.h"
#include "l/l_int_matrix.h"

/* 
 *         back_project_frame_slide_to_slide
 *
 *  This method backprojects the image of the slide in the frame to the
 *  slide. 
 *  
 *  Preconditions: homography must map the correct frame dimensions to slide dimension
 */

/* 
 * Kobus: 2019/09/06.
 * This code is more or less duplicated in lib/i_geometric/i_geometric_maping.c.
 * It is not clear which is newer. Both could use rationalization. My guess is
 * that we have an unfinished attempt to put slic code that is not secficic to
 * slic elsewhere. For now, I have renamed the version in
 * lib/i_geometric/i_geometric_maping.c to "back_project_image_new". 
*/
int back_project_frame_slide_to_slide
 (
  const KJB_image *frame_ip,
  int slide_width,
  int slide_height,
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
    KJB_image       *tmp_bp_ip = NULL;
    int i,j;

    result = get_bound(H_mp, 
                       /*homography_inverse,*/
                       homography_transform,
                       0,  
                       0,  
                       frame_ip->num_cols,
                       frame_ip->num_rows,
                       &img_pos_mp);

    if(result != ERROR)
    {   
        result = get_slide_bound(img_pos_mp, frame_ip->num_cols, 
                                 frame_ip->num_rows, &slide_bound_mp);
    }   

    /* back project images */
    if(result != ERROR)
    {   
        result = transform_image(frame_ip, H_mp, frame_ip->num_cols, frame_ip->num_rows,
                                 slide_bound_mp, fitting_model, &tmp_bp_ip, &mask_imp);
    }   

    if(result != ERROR && mask_impp != NULL)
    {   
        ERE(copy_int_matrix(mask_impp, mask_imp));
    }   
    /* Crop image*/
    ERE(get_target_image(bp_ipp, slide_height, slide_width));
    for (i = 0; i < slide_height; i++)
    {
      for (j = 0; j < slide_width; j++)
      {
        (*bp_ipp)->pixels[i][j] = tmp_bp_ip->pixels[i][j];
      }
    }

    kjb_free_image(tmp_bp_ip);
    free_matrix(img_pos_mp);
    free_matrix(slide_bound_mp);
    free_int_matrix(mask_imp);

    return  result;
}

int get_enlarged_frame_dimensions
  (
   const Matrix *slide_to_frame_mp,
   int target_slide_width,
   int target_slide_height,
   int frame_width,
   int frame_height,
   Int_matrix **enlarged_frame_dimensions_impp)
{
  Matrix *img_pos_mp = NULL;
  int min_x, max_x, min_y, max_y;
  int frame_slide_width, frame_slide_height;
  double frame_slide_to_slide_width_ratio, frame_slide_to_slide_height_ratio;
  Int_matrix *enlarged_frame_dimensions_imp = NULL;

  ERE(get_target_int_matrix(enlarged_frame_dimensions_impp, 2, 1));
  enlarged_frame_dimensions_imp = *enlarged_frame_dimensions_impp;

  ERE(get_bound(slide_to_frame_mp, 
                homography_transform,
                0,  
                0,  
                target_slide_width,
                target_slide_height,
                &img_pos_mp));
  /* Get the smallest rectangle (not bounding rectangle)*/
  min_x = (int)MIN_OF(img_pos_mp->elements[0][0], img_pos_mp->elements[2][0]);
  max_x = (int)(MIN_OF(img_pos_mp->elements[1][0], img_pos_mp->elements[3][0]) + 0.5);
  min_y = (int)MIN_OF(img_pos_mp->elements[0][1], img_pos_mp->elements[1][1]);
  max_y = (int)(MIN_OF(img_pos_mp->elements[2][1], img_pos_mp->elements[3][1]) + 0.5);

  frame_slide_width = max_x - min_x;
  frame_slide_height = max_y - min_y;

  frame_slide_to_slide_width_ratio = (double)target_slide_width/(double)frame_slide_width;
  frame_slide_to_slide_height_ratio = (double)target_slide_height/(double)frame_slide_height;

  if (   frame_slide_to_slide_width_ratio < 1. 
      && frame_slide_to_slide_height_ratio < 1. 
     )
  {
    enlarged_frame_dimensions_imp->elements[0][0] = frame_width;
    enlarged_frame_dimensions_imp->elements[1][0] = frame_height;
  }
  else 
  {
    double ratio = MAX_OF(frame_slide_to_slide_width_ratio, frame_slide_to_slide_height_ratio);
    enlarged_frame_dimensions_imp->elements[0][0] = frame_width*ratio;
    enlarged_frame_dimensions_imp->elements[1][0] = frame_height*ratio;
  }

}

int back_project_frame_slide_to_slide_enlarged
 (
  const KJB_image *frame_ip,
  int slide_width,
  int slide_height,
  int enlarged_slide_width,
  int enlarged_slide_height,
  int frame_width, 
  int frame_height,
  const Matrix    *H_mp,
  int             fitting_model,
  KJB_image       **bp_ipp,
  Int_matrix      **mask_impp
 )
{
    Matrix *slide_to_frame_mp = NULL;
    Matrix *img_pos_mp = NULL;
    Matrix *slide_bound_mp = NULL;
    Int_matrix *mask_imp = NULL;
    int result;
    KJB_image       *tmp_bp_ip = NULL;
    KJB_image       *scaled_frame_ip = NULL;
    int i,j;
    double enlarged_frame_width, enlarged_frame_height;
    Int_matrix *enlarged_frame_dimensions = NULL;
    double frame_ratio;
    Matrix    *scaled_H_mp = NULL;
    Matrix *scale_slide_mp = NULL;
    Matrix *scale_frame_mp = NULL;
    ERE(get_target_matrix(&scale_slide_mp, 3, 3)); 
    ERE(get_target_matrix(&scale_frame_mp, 3, 3)); 

    scale_slide_mp->elements[0][0] = (double)enlarged_slide_width/(double)slide_width;
    scale_slide_mp->elements[0][1] = 0.0;
    scale_slide_mp->elements[0][2] = 0.0;
    scale_slide_mp->elements[1][0] = 0.0;
    scale_slide_mp->elements[1][1] = (double)enlarged_slide_height/(double)slide_height;
    scale_slide_mp->elements[1][2] = 0.0;
    scale_slide_mp->elements[2][0] = 0.0;
    scale_slide_mp->elements[2][1] = 0.0;
    scale_slide_mp->elements[2][2] = 1.0;

    
    ERE(multiply_matrices(&scaled_H_mp, scale_slide_mp, H_mp));

    /* Do some computation on how large the slide is 
    *  Get bounding box and use min*/
    ERE(get_matrix_inverse(&slide_to_frame_mp, scaled_H_mp));
    ERE(get_enlarged_frame_dimensions(slide_to_frame_mp,
                                      enlarged_slide_width,
                                      enlarged_slide_height,
                                      frame_width,
                                      frame_height,
                                      &enlarged_frame_dimensions));
    frame_ratio = (double)enlarged_frame_dimensions->elements[0][0]/(double)frame_width;

    scale_frame_mp->elements[0][0] = 1./frame_ratio;
    scale_frame_mp->elements[0][1] = 0.0;
    scale_frame_mp->elements[0][2] = 0.0;
    scale_frame_mp->elements[1][0] = 0.0;
    scale_frame_mp->elements[1][1] = 1./frame_ratio;
    scale_frame_mp->elements[1][2] = 0.0;
    scale_frame_mp->elements[2][0] = 0.0;
    scale_frame_mp->elements[2][1] = 0.0;
    scale_frame_mp->elements[2][2] = 1.0;
    ERE(multiply_matrices(&scaled_H_mp, scaled_H_mp, scale_frame_mp));

    /* The code that backprojects one image to another assumes the source image
     *  is larger than the target image, so we will need to scale the frame */
    ERE(scale_image_size(&scaled_frame_ip, frame_ip, frame_ratio));

    
    free_int_matrix(enlarged_frame_dimensions);
    free_matrix(slide_to_frame_mp);
    free_matrix(scale_slide_mp);
    free_matrix(scale_frame_mp);

    result = get_bound(scaled_H_mp, 
                       /*homography_inverse,*/
                       homography_transform,
                       0,  
                       0,  
                       scaled_frame_ip->num_cols,
                       scaled_frame_ip->num_rows,
                       &img_pos_mp);

    if(result != ERROR)
    {   
        result = get_slide_bound(img_pos_mp, 
                                 scaled_frame_ip->num_cols, 
                                 scaled_frame_ip->num_rows, 
                                 &slide_bound_mp);
    }   

    /* back project images */
    if(result != ERROR)
    {   
        result = transform_image(scaled_frame_ip, scaled_H_mp, scaled_frame_ip->num_cols, scaled_frame_ip->num_rows,
                                 slide_bound_mp, fitting_model, &tmp_bp_ip, &mask_imp);
    }   

    if(result != ERROR && mask_impp != NULL)
    {   
        ERE(copy_int_matrix(mask_impp, mask_imp));
    }   
    /* Crop image*/
    ERE(get_target_image(bp_ipp, enlarged_slide_height, enlarged_slide_width));
    for (i = 0; i < enlarged_slide_height; i++)
    {
      for (j = 0; j < enlarged_slide_width; j++)
      {
        (*bp_ipp)->pixels[i][j] = tmp_bp_ip->pixels[i][j];
      }
    }
    
    kjb_free_image(tmp_bp_ip);
    kjb_free_image(scaled_frame_ip);
    free_matrix(img_pos_mp);
    free_matrix(slide_bound_mp);
    free_int_matrix(mask_imp);

    return  result;
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





