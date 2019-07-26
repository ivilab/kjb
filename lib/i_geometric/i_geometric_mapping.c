
#include <m/m_incl.h>
#include <i_geometric/i_geometric_mapping.h>

/*
 * We need to move slice stuff elsewhere and move lib/slic to src/projects/SLIC.
*/
#include <slic/homography.h>
#include <slic/basic.h>

/*
#include <slic/image_interp.h>
#include <slic/rotation.h>
*/

/* 
 * Kobus: 2019/09/06.
 * This code is more or less duplicated in lib/slic. It is not clear which is
 * newer. Both could use rationalization. My guess is that we have an unfinished
 * attempt to put slic code that is not secficic to slic elsewhere. Renaming
 * this version of "back_project_image" to "back_project_image_new". 
*/

int back_project_image_new
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

    verbose_pso(5, "Static version of '%s' in '%s'.\n", __FUNCTION__, __FILE__);

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

    /*db_mat( slide_bound_mp );*/

    /* back project images */
    if(result != ERROR)
    {   
        /*
        result = transform_image(ip, H_mp, ip->num_cols, ip->num_rows,
                                 slide_bound_mp, fitting_model, bp_ipp, &mask_imp);
        */
        result = transform_image_homography(ip, H_mp, ip->num_cols, ip->num_rows,
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

/* 
 *   Based on transform_image() in /lib/slic/image_interp.c
 *
 *   This method projects an image (src_ip) onto 
 *   target_ipp, a width x height transparent image
 *
 *  src_ip: image to project 
 *  t_mp: the homography that will project src_ip to target_ipp
 *        the homography that transforms the slide to the frame. 
 *  width,height: width and height of the image where src_ip will be projected
 *  inter_method: interpolation method? Usually set to HOMOGRAPHY
 *                HOMOGRAPHY is defined in the slic/basic.h file
 *  target_ipp: the image where src_ip will be projected
 *  mask_impp: will be the matrix of 0's and 1's that says where 
 *  the image is transparent. 0's indicate transparency
 * 
 * 
 *  trans_rect_mp: the slide's corner coordinates in frame     
 *  
 *  TODO: remove the width and height parameters, since they can be
 *        obtained inside the function
 *  Author: QuanFu (probably)
*/
int transform_image_homography
(
    const KJB_image *src_ip,
    const Matrix    *t_mp,
    int             width,
    int             height,
    const Matrix     *trans_rect_mp,
    int             inter_method,
    KJB_image       **target_ipp,
    Int_matrix      **mask_impp
)
{
    Matrix *mp = NULL;
    Matrix *trans_mp = NULL;
    int id;
    register int i, j;
    int result = NO_ERROR;
    
    if(width <=0 || height <= 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    pso(" Static version of %s in compute_rotation.c\n", __FUNCTION__);

    ERE(get_target_matrix(&mp, width * height, 2));
    id = 0;
    /* Qiyam: create a (width*height)x2 matrix whose
    *  elements are the points in the frame */
    for(i=0; i < height; i++) 
    {
        for(j=0; j<width; j++)
        {
            /*i,j generate a correctly rotated image*/
            mp->elements[id][0] = i; 
            mp->elements[id][1] = j; 
            id++;
        }
    }

    /* Project the frame's coordinates on the slide
    *  Now trans_mp will be the coordinates of where the frame is on the slide*/
    if(inter_method == HOMOGRAPHY)
    {
        result = homography_inverse(t_mp, mp, &trans_mp);
        /*result = homography_transform(t_mp, mp, &trans_mp);*/
        if (result == ERROR) { EGC(result); }
    }
    else
    {
        /* TODO */
        EGC(ERROR);
    }
    /*
    else if(inter_method == AFFINE)
    {
        result = affine_inverse(t_mp, mp, &trans_mp);
        if (result == ERROR) { EGC(result); }        
    }
    */

    /*result = bilinear_inter_image(src_ip, trans_mp, width, height, trans_rect_mp, target_ipp,mask_impp);*/
    
    /*result = bilinear_interpolation(src_ip, trans_mp, width, height, trans_rect_mp, target_ipp,mask_impp);*/
    /*Testing TODO*/
    result = bilinear_interpolation(src_ip, trans_mp, width, height, NULL, target_ipp,mask_impp);
    if (result == ERROR) { EGC(result); }
     /* ERE(interp(src_ip, trans_mp, width, height, trans_rect_mp, target_ipp, mask_impp, t_mp));*/

cleanup:
    free_matrix(mp);
    free_matrix(trans_mp);

    return result;
}

/* Function adapted from lib/slic.
   The main difference is in the order in which the image points
   are processed.

   interpolate a grid by using the biliear interpolation approach
 * An example of how the code works:
 * Assuming we want to backproject a slide image back to a frame, 
 * these are what the parameters are:
 *
 *  src_ip: slide image
 *  trans_mp: a projection of all the points of the frame 
 *            on the slide image. 
 *  width: frame's width
 *  height: frame's height
 *  trans_rect_mp: a 2x2 matrix with the slide's corner coordinates in frame
 *                 (upper left and lower right corner coordinates)
 *  target_ipp: the image where the transformed slide will be.
 *  mask_impp: the mask that tells us which pixels the transformed slide
 *             occupies in target_ipp.
 *
 */
int bilinear_interpolation
(
    const KJB_image *src_ip,
    const Matrix     *trans_mp,
    int              width,
    int              height,
    const Matrix     *trans_rect_mp,
    KJB_image        **target_ipp,
    Int_matrix       **mask_impp
)
{
    int result = NO_ERROR;
    register int i, j;
    int id;
    KJB_image *target_ip = NULL;
    double x, y;
    double normx, normy;
    int p1x, p1y;
    Pixel p1, p2;
    double r, g, b;
    double t1_r, t1_g, t1_b, t2_r, t2_g, t2_b;
    int num_rows = src_ip->num_rows;
    int num_cols = src_ip->num_cols;
    Int_matrix *mask_imp = NULL;
    int startx, starty, endx, endy;
    int WHITE = 255;

    if(target_ipp == NULL) return NO_ERROR;
    
    verbose_pso(5, " Static version of %s in '%s'.\n", __FUNCTION__, __FILE__);

    startx = 0;
    starty = 0;
    endx = width - 1;
    endy = height -1;
    /* Get the max/min x/y's so that we don't waste time drawing 
     * points that cannot be seen. */
    if(trans_rect_mp != NULL)
    {
        startx = (int)trans_rect_mp->elements[0][0];
        starty = (int)trans_rect_mp->elements[0][1];
        endx = startx + (int)trans_rect_mp->elements[1][0] - 1;
        endy = starty + (int)trans_rect_mp->elements[1][1] - 1;
     /*   pso("%d %d %d %d\n", startx, starty, endx, endy);*/
        if(startx < 0 || startx >= width ||
           endx < 0 || endx >= width ||
           starty < 0 || starty >= height ||
           endy < 0 || endy >= height)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }
    else
    {
        verbose_pso(5, "Not using trans_rect_mp.\n");
    }

    /*result = (get_zero_image(target_ipp, height, width));*/
    result = (get_initialized_image_2(target_ipp, height, width, WHITE, WHITE, WHITE ));
    if (result == ERROR) { EGC(result); }

    result = (get_initialized_int_matrix(&mask_imp, height, width, 0));
    if (result == ERROR) { EGC(result); }    

    target_ip = *target_ipp;
    
    /* Note on trans_mp: trans_mp[id][0] and trans_mp[id][1] are the
     * xy coordinates of the frame's id'th pixel (where id = row*width + col)
     and [0] indexes rows? while [1] indexes columns
     */
    for(i=starty; i<=endy; i++)
    {
        for(j=startx; j<=endx; j++)
        {
            id = i * width + j;
            /* Divergent implementation from lib/slic/image_interp.c */
            /* Note: unlike the original bilinear_image_interp(), this is indexing by column first. */
            y = trans_mp->elements[id][0];
            x = trans_mp->elements[id][1];
            /* If this is not a point in the slide, then ignore*/
            if(x < 0 || y < 0 || x > num_cols-1 || y > num_rows -1) 
            {
                r = 0;
                g = 0;
                b = 0;
            }
            else
            {
               p1x=(int)x;
               p1y=(int)y;
               normx = x-p1x;
               normy = y-p1y;
               /*pso("%f %f\n", normx, normy);*/
               if (normx == 0 && normy == 0)
               {
                   r = (src_ip->pixels[p1y][p1x]).r;
                   g = (src_ip->pixels[p1y][p1x]).g;
                   b = (src_ip->pixels[p1y][p1x]).b;
               }
               else if (normx == 0)
               {
                   p1 = src_ip->pixels[p1y][p1x];
                   p2 = src_ip->pixels[p1y+1][p1x];
                   r = p1.r + (p2.r - p1.r) * normy;
                   g = p1.g + (p2.g - p1.g) * normy;
                   b = p1.b + (p2.b - p1.b) * normy;
               }
               else if (normy == 0)
               {
                   p1 = src_ip->pixels[p1y][p1x];
                   p2 = src_ip->pixels[p1y][p1x+1];
                   r = p1.r + (p2.r - p1.r) * normx;
                   g = p1.g + (p2.g - p1.g) * normx;
                   b = p1.b + (p2.b - p1.b) * normx;
               }
               else
               {
                   p1 = src_ip->pixels[p1y][p1x];
                   p2 = src_ip->pixels[p1y][p1x+1];
                   t1_r = p1.r + (p2.r - p1.r) * normx;
                   t1_g = p1.g + (p2.g - p1.g) * normx;
                   t1_b = p1.b + (p2.b - p1.b) * normx;
                   p1 = src_ip->pixels[p1y+1][p1x];
                   p2 = src_ip->pixels[p1y+1][p1x+1];
                   t2_r = p1.r + (p2.r - p1.r) * normx;
                   t2_g = p1.g + (p2.g - p1.g) * normx;
                   t2_b = p1.b + (p2.b - p1.b) * normx;

                   r = t1_r + (t2_r - t1_r) * normy;
                   g = t1_g + (t2_g - t1_g) * normy;
                   b = t1_b + (t2_b - t1_b) * normy;
               }
               (target_ip->pixels[i][j]).r = r;
               (target_ip->pixels[i][j]).g = g;
               (target_ip->pixels[i][j]).b = b;
                mask_imp->elements[i][j] = 1;
           }
        }
    }

    if(mask_impp != NULL)
    {
        result = (copy_int_matrix(mask_impp, mask_imp));
        if (result == ERROR) { EGC(result); }        
    }
        
cleanup:
    free_int_matrix(mask_imp);

    return result;
}


