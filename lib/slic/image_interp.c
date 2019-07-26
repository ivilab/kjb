/* $Id: image_interp.c 21596 2017-07-30 23:33:36Z kobus $
 */
#include "l/l_sys_debug.h"   /* For ASSERT */
#include "slic/affine.h"
#include "slic/homography.h"
#include "slic/image_interp.h"

#warning "[code police] Better to use MIN_OF, MAX_OF in lib/l/l_def.h"
#warning "[code police] Also, PI appears unused, but if it is needed, it is"
#warning "[code police] better to use standard symbol M_PI."

/* Struct and Constants*/
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define PI 3.1415926535897932

typedef struct point
{
    double x;
    double y;
} point;


#warning "[code police] Private (file-scope) functions should be made static."
/* These are private functions */
void sum_triangle1 ( const KJB_image * src, double top, double bottom, double left, double right,
             double * dest_r, double * dest_g, double * dest_b, double * dest_w );
void sum_triangle2 ( const KJB_image * src, double top, double bottom, double left, double right,
             double * dest_r, double * dest_g, double * dest_b, double * dest_w );
void sum_triangle3 ( const KJB_image * src, double top, double bottom, double left, double right,
             double * dest_r, double * dest_g, double * dest_b, double * dest_w );
void sum_triangle4 ( const KJB_image * src, double top, double bottom, double left, double right,
             double * dest_r, double * dest_g, double * dest_b, double * dest_w );
int transform ( const KJB_image * src, int dest_x, int dest_y, point * p, const Matrix * trans_mp );



/* 
 *   transform_image
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
int transform_image
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


    ERE(get_target_matrix(&mp, width * height, 2));
    id = 0;
    /* Qiyam: create a (width*height)x2 matrix whose
    *  elements are the points in the frame */
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            mp->elements[id][0] = j; 
            mp->elements[id][1] = i; 
            id++;
        }
    }

    /* Project the frame's coordinates on the slide
    *  Now trans_mp will be the coordinates of where the frame is on the slide*/
    if(inter_method == HOMOGRAPHY)
    {
        result = homography_inverse(t_mp, mp, &trans_mp);
        if (result == ERROR) { EGC(result); }
    }
    else if(inter_method == AFFINE)
    {
        result = affine_inverse(t_mp, mp, &trans_mp);
        if (result == ERROR) { EGC(result); }        
    }

    /*write_matrix(trans_mp, 0);*/

    result = bilinear_inter_image(src_ip, trans_mp, width, height, trans_rect_mp, target_ipp,
      mask_impp);
    if (result == ERROR) { EGC(result); }
     /* ERE(interp(src_ip, trans_mp, width, height, trans_rect_mp, target_ipp,
         mask_impp, t_mp));*/

cleanup:
    free_matrix(mp);
    free_matrix(trans_mp);

    return result;
}

/* same as transform_image, but does it only for a bounding box*/
int transform_image_w_bounding_box
(
    const KJB_image *src_ip,
    const Matrix    *t_mp,
    int             width,
    int             height,
    int             *slide_coords,
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

    ERE(get_target_matrix(&mp, width * height, 2));
    id = 0;
    /* Qiyam: create a (width*height)x2 matrix whose
    *  elements are the points in the frame */
    for(i=0; i<height; i++)
    {
        for(j=0; j<width; j++)
        {
            mp->elements[id][0] = j; 
            mp->elements[id][1] = i; 
            id++;
        }
    }

    /* Project the frame's coordinates on the slide
    *  Now trans_mp will be the coordinates of where the frame is on the slide*/
    if(inter_method == HOMOGRAPHY)
    {
        result = homography_inverse(t_mp, mp, &trans_mp);
        if (result == ERROR) { EGC(result); }        
    }
    else if(inter_method == AFFINE)
    {
        result = affine_inverse(t_mp, mp, &trans_mp);
        if (result == ERROR) { EGC(result); }        
    }

    /*write_matrix(trans_mp, 0);*/

    result = bilinear_inter_image_w_bounding_box(src_ip, trans_mp, width, height, slide_coords, trans_rect_mp, target_ipp,
      mask_impp);
    if (result == ERROR) { EGC(result); }    
     /* ERE(interp(src_ip, trans_mp, width, height, trans_rect_mp, target_ipp,
         mask_impp, t_mp));*/

cleanup:
    free_matrix(mp);
    free_matrix(trans_mp);

    return result;
}


int transform ( const KJB_image * src, int dest_x, int dest_y, point * p, const Matrix * trans_mp )
{
    Vector *v = NULL, *v2 = NULL;
    
    if (trans_mp->num_cols == 2)
    {
        get_target_vector(&v, 2);

        v->elements[0] = dest_x;
        v->elements[1] = dest_y;

        multiply_matrix_and_vector(&v2, trans_mp, v);
        p->x = v2->elements[0];
        p->y = v2->elements[1];
    }
    else
    {
        get_target_vector(&v, 3);

        v->elements[0] = dest_x;
        v->elements[1] = dest_y;
        v->elements[2] = 1.0;

        multiply_matrix_and_vector(&v2, trans_mp, v);
        p->x = v2->elements[0] / v2->elements[2];
        p->y = v2->elements[1] / v2->elements[2];
    }
    
    free_vector(v2);
    free_vector(v);
    
    if ((p->x >= 0.0) && (p->x < src->num_cols) && (p->y >= 0.0) && (p->y < src->num_rows))
        return 1; else
        return 0;
}
/* fixme: combine triangle functions into 1 */

void sum_triangle4 ( const KJB_image * src, double top, double bottom, double left, double right,
    double * dest_r, double * dest_g, double * dest_b, double * dest_w )
{
    /*
    
    \--|
     \ |
      \|
    
    */
    
    int ibottom, itop, ileft, imid, iright, i, j;
    double inv_m, m, x, y_left, y_right, mid, weight, dx, dx2, dx3, dy, dy2, 
        r = 0.0, g = 0.0, b = 0.0, w = 0.0, bb, db;
    
    itop = (int)floor(top);
    ibottom = (int)floor(bottom);
    iright = (int)floor(right);
    
    /* special case: single pixel */
    
    if ((itop == ibottom) && (iright == (int)floor(left)))
    {   
        if ((itop >= 0) && (itop < src->num_rows) && (iright >= 0) && (iright < src->num_cols))
        {
            weight = 0.5 * (right - left) * (top - bottom);
            *dest_r = src->pixels[itop][iright].r * weight;
            *dest_g = src->pixels[itop][iright].g * weight;
            *dest_b = src->pixels[itop][iright].b * weight;
            *dest_w = weight;
        }
        else
        {
            *dest_r = 0.0;
            *dest_g = 0.0;
            *dest_b = 0.0;
            *dest_w = 0.0;
        }
        
        return;
    }
    
    inv_m = (right - left) / (top - bottom);
    m = (top - bottom) / (right - left);
    
    x = left;
    
    /* step through y */
    
    for (i = itop; (i >= ibottom) && (i >= 0); i--)
    {   
        /* step through right x, and find mid = right x for the next y */
        
        if (i == itop)
        {
            bb = top - itop;
            mid = x + inv_m * bb;
            db = itop + 1 - top;
        }
        else
        {
            mid = x + inv_m;
        }
        
        if (i >= src->num_rows)
            goto last;
        
        /*ASSERT(((i == ibottom) || (mid <= right)));*/
        imid = (int)floor(mid);
        ileft = (int)floor(x);
        
        /* section 0: right side */
        
        dx = right - iright;
        
        if (i == ibottom)
        {
            dy = ibottom + 1 - bottom;  
            dx2 = inv_m * dy;
 
            if (dx2 < dx) /* bottom right corner: triangle */
            { 
                weight = 0.5 * dx2 * dy;
            }
            else /* bottom right corner: trapezoid */
            {
                dy2 = dy - dx * m;
                weight = 0.5 * dx * (dy + dy2);
                
                if (i == itop)
                {
                    weight -= dx * db;
                }
            }
        }
        else
        {
            if (imid < iright)
            {
                if (i == itop) /* top right corner: rectangle */
                {
                    weight = bb * dx;
                }
                else /* right side: rectangle */
                {
                    weight = dx;
                }
            }
            else
            {
                dx2 = mid - iright;
                dy = m * dx2;
                
                if (dy < 1.0)
                {
                    if (i == itop)
                    {
                        if (dy > bb) /* top right corner: sideways trapezoid */
                        {
                            dx = right - mid;
                            dx2 = dx + inv_m * bb;
                            weight = 0.5 * bb * (dx + dx2);
                        }
                        else /* top right corner: rectangle minus triangle */
                        {
                            weight = bb * dx - 0.5 * dx2 * dy;
                        }
                    }
                    else /* right side: rectangle minus triangle */
                    {
                        weight = dx - 0.5 * dx2 * dy;
                    }
                }
                else
                {
                    dx = right - mid;
                    
                    if (i == itop) /* top right corner: sideways trapezoid */
                    {           
                        dx2 = dx + inv_m * bb;
                        weight = 0.5 * bb * (dx + dx2);
                    }
                    else /* right side: sideways trapezoid */
                    {
                        dx2 = dx + inv_m;
                        weight = 0.5 * (dx + dx2);
                    }
                }
            }           
        }
        
        if ((iright >= 0) && (iright < src->num_cols))
        {
            ASSERT((weight >= 0.0) && (weight <= 1.0));
            r += src->pixels[i][iright].r * weight;
            g += src->pixels[i][iright].g * weight;
            b += src->pixels[i][iright].b * weight;
            w += weight;
        } 
        
        if (i != ibottom)
        {       
            /* section 1: bottom middle rectangles or non-bottom middle full pixels */

            if (i == itop)
                weight = bb; else
                weight = 1.0;
                
            ASSERT((weight >= 0.0) && (weight <= 1.0));
                
            for (j = min(iright - 1, src->num_cols - 1); (j > imid) && (j >= 0); j--)
            {
                r += src->pixels[i][j].r * weight;
                g += src->pixels[i][j].g * weight;
                b += src->pixels[i][j].b * weight;
                w += weight;
            }

            /* section 2: bottom rectangle or non-bottom full pixel minus 1 triangle below horizontal */

            if ((imid < iright) && (imid != ileft))
            {
                dx = mid - imid;
                dy = m * dx;

                weight = 1.0 - 0.5 * dx * dy;
                
                if (i == itop)
                {
                    weight -= db;
                }
                
                if ((imid >= 0) && (imid < src->num_cols))
                {
                    ASSERT((weight >= 0.0) && (weight <= 1.0));
                    r += src->pixels[i][imid].r * weight;
                    g += src->pixels[i][imid].g * weight;
                    b += src->pixels[i][imid].b * weight;
                    w += weight;
                }
            }   
        }
        
        /* section 3: trapezoids */
        
        if (i == ibottom)
            j = min(iright - 1, src->num_cols - 1); else
            j = min(imid - 1, src->num_cols - 1);
            
        y_left = top - (j + 1 - left) * m;
        
        for (; (j > ileft) && (j >= 0); j--)
        {
            y_right = y_left;
            y_left += m;
            
            weight = 0.5 * (i + 1 - y_left + i + 1 - y_right);
            
            if (i == itop)
            {
                weight -= db;
            }
            
            if ((j >= 0) && (j < src->num_cols))
            {
                ASSERT((weight >= 0.0) && (weight <= 1.0));
                r += src->pixels[i][j].r * weight;
                g += src->pixels[i][j].g * weight;
                b += src->pixels[i][j].b * weight;
                w += weight;
            }
        }
        
        /* section 4: triangle/quad */
        
        if (ileft != iright)
        {
            dx = ileft + 1 - x; 
            dy = m * dx;

            if (i == itop)
            {
                if (imid == ileft) /* top left corner: sideways trapezoid */
                {
                    dx2 = ileft + 1 - mid;
                    weight = 0.5 * bb * (dx + dx2);
                }
                else /* top left corner: triangle */
                {
                    weight = 0.5 * dx * dy;
                }
            }
            else
            {
                if (imid == ileft) /* left end: sideways trapezoid */
                {
                    dx2 = dx - inv_m;
                    weight = 0.5 * (dx + dx2);
                }
                else /* left end: triangle */
                {
                    weight = 0.5 * dx * dy;
                }
            }

            if ((ileft >= 0) && (ileft < src->num_cols))
            {
                ASSERT((weight >= 0.0) && (weight <= 1.0));
                r += src->pixels[i][ileft].r * weight;
                g += src->pixels[i][ileft].g * weight;
                b += src->pixels[i][ileft].b * weight;
                w += weight;
            }
        }
    
last:
        x = mid;
    }
    
    *dest_r = r;
    *dest_g = g;
    *dest_b = b;
    *dest_w = w;
}

void sum_triangle3 ( const KJB_image * src, double top, double bottom, double left, double right,
    double * dest_r, double * dest_g, double * dest_b, double * dest_w )
{
    /*
    
    |--/
    | /
    |/
    
    */
    
    int ibottom, itop, ileft, imid, iright, i, j;
    double inv_m, m, x, y_left, y_right, mid, weight, dx, dx2, dx3, dy, dy2, 
        r = 0.0, g = 0.0, b = 0.0, w = 0.0, bb, db;
    
    itop = (int)floor(top);
    ibottom = (int)floor(bottom);
    ileft = (int)floor(left);
    
    /* special case: single pixel */
    
    if ((itop == ibottom) && (ileft == (int)floor(right)))
    {   
        if ((itop >= 0) && (itop < src->num_rows) && (ileft >= 0) && (ileft < src->num_cols))
        {
            weight = 0.5 * (right - left) * (top - bottom);
            *dest_r = src->pixels[itop][ileft].r * weight;
            *dest_g = src->pixels[itop][ileft].g * weight;
            *dest_b = src->pixels[itop][ileft].b * weight;
            *dest_w = weight;
        }
        else
        {
            *dest_r = 0.0;
            *dest_g = 0.0;
            *dest_b = 0.0;
            *dest_w = 0.0;
        }
        
        return;
    }
    
    inv_m = (right - left) / (top - bottom);
    m = (top - bottom) / (right - left);
    
    x = right;
    
    /* step through y */
    
    for (i = itop; (i >= ibottom) && (i >= 0); i--)
    {   
        /* step through right x, and find mid = right x for the next y */
        
        if (i == itop)
        {
            bb = top - itop;
            mid = x - inv_m * bb;
            db = itop + 1 - top;
        }
        else
        {
            mid = x - inv_m;
        }
        
        if (i >= src->num_rows)
            goto last;
        
        /*ASSERT(((i == ibottom) || (left <= mid)));*/
        imid = (int)floor(mid);
        iright = (int)floor(x);
        
        /* section 0: left side */
        
        dx = ileft + 1 - left;
        
        if (i == ibottom)
        {
            dy = ibottom + 1 - bottom;  
            dx2 = inv_m * dy;
 
            if (dx2 < dx) /* bottom left corner: triangle */
            { 
                weight = 0.5 * dx2 * dy;
            }
            else /* bottom left corner: trapezoid */
            {
                dy2 = dy - dx * m;
                weight = 0.5 * dx * (dy + dy2);
                
                if (i == itop)
                {
                    weight -= dx * db;
                }
            }
        }
        else
        {
            if (imid > ileft)
            {
                if (i == itop) /* top left corner: rectangle */
                {
                    weight = bb * dx;
                }
                else /* left side: rectangle */
                {
                    weight = dx;
                }
            }
            else
            {
                dx2 = ileft + 1 - mid;
                dy = m * dx2;
                
                if (dy < 1.0)
                {
                    if (i == itop)
                    {
                        if (dy > bb) /* top left corner: sideways trapezoid */
                        {
                            dx = mid - left;
                            dx2 = dx + inv_m * bb;
                            weight = 0.5 * bb * (dx + dx2);
                        }
                        else /* top left corner: rectangle minus triangle */
                        {
                            weight = bb * dx - 0.5 * dx2 * dy;
                        }
                    }
                    else /* left side: rectangle minus triangle */
                    {
                        weight = dx - 0.5 * dx2 * dy;
                    }
                }
                else
                {
                    dx = mid - left;
                    
                    if (i == itop) /* top left corner: sideways trapezoid */
                    {           
                        dx2 = dx + inv_m * bb;
                        weight = 0.5 * bb * (dx + dx2);
                    }
                    else /* left side: sideways trapezoid */
                    {
                        dx2 = dx + inv_m;
                        weight = 0.5 * (dx + dx2);
                    }
                }
            }           
        }
        
        if ((ileft >= 0) && (ileft < src->num_cols))
        {
            ASSERT((weight >= 0.0) && (weight <= 1.0));
            r += src->pixels[i][ileft].r * weight;
            g += src->pixels[i][ileft].g * weight;
            b += src->pixels[i][ileft].b * weight;
            w += weight;
        } 
        
        if (i != ibottom)
        {       
            /* section 1: bottom middle rectangles or non-bottom middle full pixels */

            if (i == itop)
                weight = bb; else
                weight = 1.0;
                
            ASSERT((weight >= 0.0) && (weight <= 1.0));
                
            for (j = max(ileft + 1, 0); (j < imid) && (j < src->num_cols); j++)
            {
                r += src->pixels[i][j].r * weight;
                g += src->pixels[i][j].g * weight;
                b += src->pixels[i][j].b * weight;
                w += weight;
            }

            /* section 2: bottom rectangle or non-bottom full pixel minus 1 triangle below horizontal */

            if ((imid > ileft) && (imid != iright))
            {
                dx = (imid + 1) - mid;
                dy = m * dx;

                weight = 1.0 - 0.5 * dx * dy;
                
                if (i == itop)
                {
                    weight -= db;
                }
                
                if ((imid >= 0) && (imid < src->num_cols))
                {
                    ASSERT((weight >= 0.0) && (weight <= 1.0));
                    r += src->pixels[i][imid].r * weight;
                    g += src->pixels[i][imid].g * weight;
                    b += src->pixels[i][imid].b * weight;
                    w += weight;
                }
            }   
        }
        
        /* section 3: trapezoids */
        
        if (i == ibottom)
            j = max(ileft + 1, 0); else
            j = max(imid + 1, 0);
            
        y_right = top - (right - j) * m;
        
        for (; (j < iright) && (j < src->num_cols); j++)
        {
            y_left = y_right;
            y_right += m;
            
            weight = 0.5 * (i + 1 - y_left + i + 1 - y_right);
            
            if (i == itop)
            {
                weight -= db;
            }
            
            if ((j >= 0) && (j < src->num_cols))
            {
                ASSERT((weight >= 0.0) && (weight <= 1.0));
                r += src->pixels[i][j].r * weight;
                g += src->pixels[i][j].g * weight;
                b += src->pixels[i][j].b * weight;
                w += weight;
            }
        }
        
        /* section 4: triangle/quad */
        
        if (ileft != iright)
        {
            dx = x - iright; 
            dy = m * dx;

            if (i == itop)
            {
                if (imid == iright) /* bottom right corner: sideways trapezoid */
                {
                    dx2 = mid - iright;
                    weight = 0.5 * bb * (dx + dx2);
                }
                else /* bottom right corner: triangle */
                {
                    weight = 0.5 * dx * dy;
                }
            }
            else
            {
                if (imid == iright) /* right end: sideways trapezoid */
                {
                    dx2 = dx - inv_m;
                    weight = 0.5 * (dx + dx2);
                }
                else /* right end: triangle */
                {
                    weight = 0.5 * dx * dy;
                }
            }

            if ((iright >= 0) && (iright < src->num_cols))
            {
                ASSERT((weight >= 0.0) && (weight <= 1.0));
                r += src->pixels[i][iright].r * weight;
                g += src->pixels[i][iright].g * weight;
                b += src->pixels[i][iright].b * weight;
                w += weight;
            }
        }
    
last:
        x = mid;
    }
    
    *dest_r = r;
    *dest_g = g;
    *dest_b = b;
    *dest_w = w;
}

void sum_triangle2 ( const KJB_image * src, double top, double bottom, double left, double right,
    double * dest_r, double * dest_g, double * dest_b, double * dest_w )
{
    /*
    
      /|
     / |
    /__|
    
    */
    
    int ibottom, itop, ileft, imid, iright, i, j;
    double inv_m, m, x, y_left, y_right, mid, weight, dx, dx2, dx3, dy, dy2, 
        r = 0.0, g = 0.0, b = 0.0, w = 0.0, bb, db;
    
    itop = (int)floor(top);
    ibottom = (int)floor(bottom);
    iright = (int)floor(right);
    
    /* special case: single pixel */
    
    if ((itop == ibottom) && (iright == (int)floor(left)))
    {   
        if ((itop >= 0) && (itop < src->num_rows) && (iright >= 0) && (iright < src->num_cols))
        {
            weight = 0.5 * (right - left) * (top - bottom);
            *dest_r = src->pixels[itop][iright].r * weight;
            *dest_g = src->pixels[itop][iright].g * weight;
            *dest_b = src->pixels[itop][iright].b * weight;
            *dest_w = weight;
        }
        else
        {
            *dest_r = 0.0;
            *dest_g = 0.0;
            *dest_b = 0.0;
            *dest_w = 0.0;
        }
        
        return;
    }
    
    inv_m = (right - left) / (top - bottom);
    m = (top - bottom) / (right - left);
    
    x = left;
    
    /* step through y */
    
    for (i = ibottom; (i <= itop) && (i < src->num_rows); i++)
    {   
        /* step through right x, and find mid = right x for the next y */
        
        if (i == ibottom)
        {
            bb = ibottom + 1 - bottom;
            mid = x + inv_m * bb;
            db = bottom - ibottom;
        }
        else
        {
            mid = x + inv_m;
        }
        
        if (i < 0)
            goto last;
        
        /*ASSERT(((i == itop) || (mid <= right)));*/
        imid = (int)floor(mid);
        ileft = (int)floor(x);
        
        /* section 0: right side */
        
        dx = right - iright;
        
        if (i == itop)
        {
            dy = top - itop;    
            dx2 = inv_m * dy;
 
            if (dx2 < dx) /* top right corner: triangle */
            { 
                weight = 0.5 * dx2 * dy;
            }
            else /* top right corner: trapezoid */
            {
                dy2 = dy - dx * m;
                weight = 0.5 * dx * (dy + dy2);
                
                if (i == ibottom)
                {
                    weight -= dx * db;
                }
            }
        }
        else
        {
            if (imid < iright)
            {
                if (i == ibottom) /* bottom right corner: rectangle */
                {
                    weight = bb * dx;
                }
                else /* right side: rectangle */
                {
                    weight = dx;
                }
            }
            else
            {
                dx2 = mid - iright;
                dy = m * dx2;
                
                if (dy < 1.0)
                {
                    if (i == ibottom)
                    {
                        if (dy > bb) /* bottom right corner: sideways trapezoid */
                        {
                            dx = right - mid;
                            dx2 = dx + inv_m * bb;
                            weight = 0.5 * bb * (dx + dx2);
                        }
                        else /* bottom right corner: rectangle minus triangle */
                        {
                            weight = bb * dx - 0.5 * dx2 * dy;
                        }
                    }
                    else /* right side: rectangle minus triangle */
                    {
                        weight = dx - 0.5 * dx2 * dy;
                    }
                }
                else
                {
                    dx = right - mid;
                    
                    if (i == ibottom) /* bottom right corner: sideways trapezoid */
                    {           
                        dx2 = dx + inv_m * bb;
                        weight = 0.5 * bb * (dx + dx2);
                    }
                    else /* right side: sideways trapezoid */
                    {
                        dx2 = dx + inv_m;
                        weight = 0.5 * (dx + dx2);
                    }
                }
            }           
        }
        
        if ((iright >= 0) && (iright < src->num_cols))
        {
            ASSERT((weight >= 0.0) && (weight <= 1.0));
            r += src->pixels[i][iright].r * weight;
            g += src->pixels[i][iright].g * weight;
            b += src->pixels[i][iright].b * weight;
            w += weight;
        } 
        
        if (i != itop)
        {       
            /* section 1: bottom middle rectangles or non-bottom middle full pixels */

            if (i == ibottom)
                weight = bb; else
                weight = 1.0;
                
            ASSERT((weight >= 0.0) && (weight <= 1.0));
                
            for (j = min(iright - 1, src->num_cols - 1); (j > imid) && (j >= 0); j--)
            {
                r += src->pixels[i][j].r * weight;
                g += src->pixels[i][j].g * weight;
                b += src->pixels[i][j].b * weight;
                w += weight;
            }

            /* section 2: bottom rectangle or non-bottom full pixel minus 1 triangle below horizontal */

            if ((imid < iright) && (imid != ileft))
            {
                dx = mid - imid;
                dy = m * dx;

                weight = 1.0 - 0.5 * dx * dy;
                
                if (i == ibottom)
                {
                    weight -= db;
                }
                
                if ((imid >= 0) && (imid < src->num_cols))
                {
                    ASSERT((weight >= 0.0) && (weight <= 1.0));
                    r += src->pixels[i][imid].r * weight;
                    g += src->pixels[i][imid].g * weight;
                    b += src->pixels[i][imid].b * weight;
                    w += weight;
                }
            }   
        }
        
        /* section 3: trapezoids */
        
        if (i == itop)
            j = min(iright - 1, src->num_cols - 1); else
            j = min(imid - 1, src->num_cols - 1);
            
        y_left = (j + 1 - left) * m + bottom;
        
        for (; (j > ileft) && (j >= 0); j--)
        {
            y_right = y_left;
            y_left -= m;
            
            weight = 0.5 * (y_left - i + y_right - i);
            
            if (i == ibottom)
            {
                weight -= db;
            }
            
            if ((j >= 0) && (j < src->num_cols))
            {
                ASSERT((weight >= 0.0) && (weight <= 1.0));
                r += src->pixels[i][j].r * weight;
                g += src->pixels[i][j].g * weight;
                b += src->pixels[i][j].b * weight;
                w += weight;
            }
        }
        
        /* section 4: triangle/quad */
        
        if (ileft != iright)
        {
            dx = ileft + 1 - x; 
            dy = m * dx;

            if (i == ibottom)
            {
                if (imid == ileft) /* bottom left corner: sideways trapezoid */
                {
                    dx2 = ileft + 1 - mid;
                    weight = 0.5 * bb * (dx + dx2);
                }
                else /* bottom left corner: triangle */
                {
                    weight = 0.5 * dx * dy;
                }
            }
            else
            {
                if (imid == ileft) /* left end: sideways trapezoid */
                {
                    dx2 = dx - inv_m;
                    weight = 0.5 * (dx + dx2);
                }
                else /* left end: triangle */
                {
                    weight = 0.5 * dx * dy;
                }
            }

            if ((ileft >= 0) && (ileft < src->num_cols))
            {
                ASSERT((weight >= 0.0) && (weight <= 1.0));
                r += src->pixels[i][ileft].r * weight;
                g += src->pixels[i][ileft].g * weight;
                b += src->pixels[i][ileft].b * weight;
                w += weight;
            }
        }
    
last:
        x = mid;
    }
    
    *dest_r = r;
    *dest_g = g;
    *dest_b = b;
    *dest_w = w;
}

void sum_triangle1 ( const KJB_image * src, double top, double bottom, double left, double right,
    double * dest_r, double * dest_g, double * dest_b, double * dest_w )
{
    /*
    
    |\
    | \
    |__\
    
    */
    
    int ibottom, itop, ileft, imid, iright, i, j;
    double inv_m, m, x, y_left, y_right, mid, weight, dx, dx2, dx3, dy, dy2, 
        r = 0.0, g = 0.0, b = 0.0, w = 0.0, bb, db;
    
    itop = (int)floor(top);
    ibottom = (int)floor(bottom);
    ileft = (int)floor(left);
    
    /* special case: single pixel */
    
    if ((itop == ibottom) && (ileft == (int)floor(right)))
    {   
        if ((itop >= 0) && (itop < src->num_rows) && (ileft >= 0) && (ileft < src->num_cols))
        {
            weight = 0.5 * (right - left) * (top - bottom);
            *dest_r = src->pixels[itop][ileft].r * weight;
            *dest_g = src->pixels[itop][ileft].g * weight;
            *dest_b = src->pixels[itop][ileft].b * weight;
            *dest_w = weight;
        }
        else
        {
            *dest_r = 0.0;
            *dest_g = 0.0;
            *dest_b = 0.0;
            *dest_w = 0.0;
        }
        
        return;
    }
    
    inv_m = (right - left) / (top - bottom);
    m = (top - bottom) / (right - left);
    
    x = right;
    
    /* step through y */
    
    for (i = ibottom; (i <= itop) && (i < src->num_rows); i++)
    {   
        /* step through right x, and find mid = right x for the next y */
        
        if (i == ibottom)
        {
            bb = ibottom + 1 - bottom;
            mid = x - inv_m * bb;
            db = bottom - ibottom;
        }
        else
        {
            mid = x - inv_m;
        }
        
        if (i < 0)
            goto last;
        
        /*ASSERT(((i == itop) || (left <= mid)));*/
        imid = (int)floor(mid);
        iright = (int)floor(x);
        
        /* section 0: left side */
        
        dx = ileft + 1 - left;
        
        if (i == itop)
        {
            dy = top - itop;    
            dx2 = inv_m * dy;
 
            if (dx2 < dx) /* top left corner: triangle */
            { 
                weight = 0.5 * dx2 * dy;
            }
            else /* top left corner: trapezoid */
            {
                dy2 = dy - dx * m;
                weight = 0.5 * dx * (dy + dy2);
                
                if (i == ibottom)
                {
                    weight -= dx * db;
                }
            }
        }
        else
        {
            if (imid > ileft)
            {
                if (i == ibottom) /* bottom left corner: rectangle */
                {
                    weight = bb * dx;
                }
                else /* left side: rectangle */
                {
                    weight = dx;
                }
            }
            else
            {
                dx2 = ileft + 1 - mid;
                dy = m * dx2;
                
                if (dy < 1.0)
                {
                    if (i == ibottom)
                    {
                        if (dy > bb) /* bottom left corner: sideways trapezoid */
                        {
                            dx = mid - left;
                            dx2 = dx + inv_m * bb;
                            weight = 0.5 * bb * (dx + dx2);
                        }
                        else /* bottom left corner: rectangle minus triangle */
                        {
                            weight = bb * dx - 0.5 * dx2 * dy;
                        }
                    }
                    else /* left side: rectangle minus triangle */
                    {
                        weight = dx - 0.5 * dx2 * dy;
                    }
                }
                else
                {
                    dx = mid - left;
                    
                    if (i == ibottom) /* bottom left corner: sideways trapezoid */
                    {           
                        dx2 = dx + inv_m * bb;
                        weight = 0.5 * bb * (dx + dx2);
                    }
                    else /* left side: sideways trapezoid */
                    {
                        dx2 = dx + inv_m;
                        weight = 0.5 * (dx + dx2);
                    }
                }
            }           
        }
        
        if ((ileft >= 0) && (ileft < src->num_cols))
        {
            ASSERT((weight >= 0.0) && (weight <= 1.0));
            r += src->pixels[i][ileft].r * weight;
            g += src->pixels[i][ileft].g * weight;
            b += src->pixels[i][ileft].b * weight;
            w += weight;
        } 
        
        if (i != itop)
        {       
            /* section 1: bottom middle rectangles or non-bottom middle full pixels */

            if (i == ibottom)
                weight = bb; else
                weight = 1.0;
                
            ASSERT((weight >= 0.0) && (weight <= 1.0));
                
            for (j = max(ileft + 1, 0); (j < imid) && (j < src->num_cols); j++)
            {
                r += src->pixels[i][j].r * weight;
                g += src->pixels[i][j].g * weight;
                b += src->pixels[i][j].b * weight;
                w += weight;
            }

            /* section 2: bottom rectangle or non-bottom full pixel minus 1 triangle below horizontal */

            if ((imid > ileft) && (imid != iright))
            {
                dx = (imid + 1) - mid;
                dy = m * dx;

                weight = 1.0 - 0.5 * dx * dy;
                
                if (i == ibottom)
                {
                    weight -= db;
                }
                
                if ((imid >= 0) && (imid < src->num_cols))
                {
                    ASSERT((weight >= 0.0) && (weight <= 1.0));
                    r += src->pixels[i][imid].r * weight;
                    g += src->pixels[i][imid].g * weight;
                    b += src->pixels[i][imid].b * weight;
                    w += weight;
                }
            }   
        }
        
        /* section 3: trapezoids */
        
        if (i == itop)
            j = max(ileft + 1, 0); else
            j = max(imid + 1, 0);
            
        y_right = (right - j) * m + bottom;
        
        for (; (j < iright) && (j < src->num_cols); j++)
        {
            y_left = y_right;
            y_right -= m;
            
            weight = 0.5 * (y_left - i + y_right - i);
            
            if (i == ibottom)
            {
                weight -= db;
            }
            
            if ((j >= 0) && (j < src->num_cols))
            {
                ASSERT((weight >= 0.0) && (weight <= 1.0));
                r += src->pixels[i][j].r * weight;
                g += src->pixels[i][j].g * weight;
                b += src->pixels[i][j].b * weight;
                w += weight;
            }
        }
        
        /* section 4: triangle/quad */
        
        if (ileft != iright)
        {
            dx = x - iright; 
            dy = m * dx;

            if (i == ibottom)
            {
                if (imid == iright) /* bottom right corner: sideways trapezoid */
                {
                    dx2 = mid - iright;
                    weight = 0.5 * bb * (dx + dx2);
                }
                else /* bottom right corner: triangle */
                {
                    weight = 0.5 * dx * dy;
                }
            }
            else
            {
                if (imid == iright) /* right end: sideways trapezoid */
                {
                    dx2 = dx - inv_m;
                    weight = 0.5 * (dx + dx2);
                }
                else /* right end: triangle */
                {
                    weight = 0.5 * dx * dy;
                }
            }

            if ((iright >= 0) && (iright < src->num_cols))
            {
                ASSERT((weight >= 0.0) && (weight <= 1.0));
                r += src->pixels[i][iright].r * weight;
                g += src->pixels[i][iright].g * weight;
                b += src->pixels[i][iright].b * weight;
                w += weight;
            }
        }
    
last:
        x = mid;
    }
    
    *dest_r = r;
    *dest_g = g;
    *dest_b = b;
    *dest_w = w;
}

void sum_rectangle ( const KJB_image * src, double bottom, double top, double left, double right,
    double * dest_r, double * dest_g, double * dest_b, double * dest_w )
{
    int itop, ileft, iright, ibottom, i, j;
    double weight, dx_left, dx_right, dy_top, dy_bottom;
    double r = 0.0, g = 0.0, b = 0.0, w = 0.0;

    itop = (int)floor(top);
    ibottom = (int)floor(bottom);
    ileft = (int)floor(left);
    iright = (int)floor(right);

    if ((ibottom >= src->num_rows) || (ileft >= src->num_cols))
    {
        *dest_r = 0.0;
        *dest_g = 0.0;
        *dest_b = 0.0;
        *dest_w = 0.0;
        return;
    }
    
    /* special case: single pixel */
    
    if ((ileft == iright) && (itop == ibottom))
    {   
        if ((ileft >= 0) && (ibottom >= 0))
        {
            weight = (top - bottom) * (right - left);
            *dest_r = src->pixels[ibottom][ileft].r * weight;
            *dest_g = src->pixels[ibottom][ileft].g * weight;
            *dest_b = src->pixels[ibottom][ileft].b * weight;
            *dest_w = weight;
        }
        else
        {
            *dest_r = 0.0;
            *dest_g = 0.0;
            *dest_b = 0.0;
            *dest_w = 0.0;
        }
        
        return;
    }

    dx_left = ileft + 1 - left;
    dx_right = right - iright;
    dy_top = top - itop;
    dy_bottom = ibottom + 1 - bottom;
    
    /* special case: single pixel row */
    
    if (ibottom == itop)
    {
        if (ibottom >= 0)
        {
            dy_bottom = top - bottom;
        
            /* left side */

            if (ileft >= 0)
            {
                weight = dx_left * dy_bottom;
                r = src->pixels[ibottom][ileft].r * weight;
                g = src->pixels[ibottom][ileft].g * weight;
                b = src->pixels[ibottom][ileft].b * weight;
                w = weight;
            }

            /* middle row */

            for (i = max(ileft + 1, 0); (i < iright) && (i < src->num_cols); i++)
            {
                r += src->pixels[ibottom][i].r * dy_bottom;
                g += src->pixels[ibottom][i].g * dy_bottom;
                b += src->pixels[ibottom][i].b * dy_bottom;     
                w += dy_bottom;         
            }

            /* right side */

            if ((iright >= 0) && (iright < src->num_cols))
            {
                weight = dx_right * dy_bottom;
                r += src->pixels[ibottom][iright].r * weight;
                g += src->pixels[ibottom][iright].g * weight;
                b += src->pixels[ibottom][iright].b * weight;
                w += weight;
            }
        }
        
        *dest_r = r;
        *dest_g = g;
        *dest_b = b;
        *dest_w = w;
        
        return;
    }
    
    /* special case: single pixel column */
    
    if (ileft == iright)
    {
        if (ileft >= 0)
        {
            dx_left = right - left;
        
            /* bottom side */

            if (ibottom >= 0)
            {
                weight = dx_left * dy_bottom;
                r = src->pixels[ibottom][ileft].r * weight;
                g = src->pixels[ibottom][ileft].g * weight;
                b = src->pixels[ibottom][ileft].b * weight;
                w = weight;
            }

            /* middle column */

            for (i = max(ibottom + 1, 0); (i < itop) && (i < src->num_rows); i++)
            {
                r += src->pixels[i][ileft].r * dx_left;
                g += src->pixels[i][ileft].g * dx_left;
                b += src->pixels[i][ileft].b * dx_left;     
                w += dx_left;           
            }

            /* top side */

            if ((itop >= 0) && (itop < src->num_rows))
            {
                weight = dx_left * dy_top;
                r += src->pixels[itop][ileft].r * weight;
                g += src->pixels[itop][ileft].g * weight;
                b += src->pixels[itop][ileft].b * weight;
                w += weight;
            }
        }
        
        *dest_r = r;
        *dest_g = g;
        *dest_b = b;
        *dest_w = w;
        
        return;
    }
    
    /* 
        
        general case
        
    */

    if ((itop >= 0) && (itop < src->num_rows))
    {
        /* top left corner */

        if (ileft >= 0)
        {
            weight = dx_left * dy_top;
            r = src->pixels[itop][ileft].r * weight;
            g = src->pixels[itop][ileft].g * weight;
            b = src->pixels[itop][ileft].b * weight;
            w = weight;
        }

        /* top middle row */
        
        for (i = max(ileft + 1, 0); (i < iright) && (i < src->num_cols); i++)
        {           
            r += src->pixels[itop][i].r * dy_top;
            g += src->pixels[itop][i].g * dy_top;
            b += src->pixels[itop][i].b * dy_top;       
            w += dy_top;
        }
        
        /* top right corner */
        
        if ((iright >= 0) && (iright < src->num_cols))
        {
            weight = dx_right * dy_top;
            r += src->pixels[itop][iright].r * weight;
            g += src->pixels[itop][iright].g * weight;
            b += src->pixels[itop][iright].b * weight;
            w += weight;
        }
    }

    /* left middle column */
    
    if (ileft >= 0)
    {
        for (i = max(ibottom + 1, 0); (i < itop) && (i < src->num_rows); i++)
        {           
            r += src->pixels[i][ileft].r * dx_left;
            g += src->pixels[i][ileft].g * dx_left;
            b += src->pixels[i][ileft].b * dx_left;     
            w += dx_left;
        }
    }

    /* center area */
    
    for (i = max(ibottom + 1, 0); (i < itop) && (i < src->num_rows); i++)
    {   
        for (j = max(ileft + 1, 0); (j < iright) && (j < src->num_cols); j++)
        {       
            r += src->pixels[i][j].r;
            g += src->pixels[i][j].g;
            b += src->pixels[i][j].b;
            w += 1.0;
        }
    }
    
    /* right middle column */
    
    if ((iright >= 0) && (iright < src->num_cols))
    {
        for (i = max(ibottom + 1, 0); (i < itop) && (i < src->num_rows); i++)
        {
            r += src->pixels[i][iright].r * dx_right;
            g += src->pixels[i][iright].g * dx_right;
            b += src->pixels[i][iright].b * dx_right;       
            w += dx_right;
        }
    }
    
    /* bottom left corner */

    if (ibottom >= 0)
    {
        if (ileft >= 0)
        {
            weight = dx_left * dy_bottom;
            r += src->pixels[ibottom][ileft].r * weight;
            g += src->pixels[ibottom][ileft].g * weight;
            b += src->pixels[ibottom][ileft].b * weight;
            w += weight;
        }

        /* bottom middle row */

        for (i = max(ileft + 1, 0); (i < iright) && (i < src->num_cols); i++)
        {
            r += src->pixels[ibottom][i].r * dy_bottom;
            g += src->pixels[ibottom][i].g * dy_bottom;
            b += src->pixels[ibottom][i].b * dy_bottom;     
            w += dy_bottom;
        }

        /* bottom right corner */

        if ((iright >= 0) && (iright < src->num_cols))
        {
            weight = dx_right * dy_bottom;
            r += src->pixels[ibottom][iright].r * weight;
            g += src->pixels[ibottom][iright].g * weight;
            b += src->pixels[ibottom][iright].b * weight;
            w += weight;
        }
    }
    
    *dest_r = r;
    *dest_g = g;
    *dest_b = b;
    *dest_w = w;
}

int interp
(
    const KJB_image *src_ip,
    const Matrix     *trans_mp,
    int              width,
    int              height,
    const Matrix     *trans_rect_mp,
    KJB_image        **target_ipp,
    Int_matrix       **mask_impp,
    const Matrix    *t_mp
)
{
    int result = NO_ERROR;
    int i, j, k, k_next, bounds;
    int id;
    KJB_image *target_ip = NULL;
    double x, y;
    double normx, normy;
    int p1x, p1y;
    Pixel p1, p2;
    double t1_r, t1_g, t1_b, t2_r, t2_g, t2_b;
    int num_rows = src_ip->num_rows;
    int num_cols = src_ip->num_cols;
    Int_matrix *mask_imp = NULL;
    int startx, starty, endx, endy;
    Matrix *inv_mp = NULL;
    point corners[4], p;
    double box_left, box_right, box_top, box_bottom, r_sum, g_sum, b_sum, w_sum, r, g, b, w;

    if(target_ipp == NULL) return NO_ERROR;

    startx = 0;
    starty = 0;
    endx = width - 1;
    endy = height -1;
    if(trans_rect_mp != NULL)
    {
        startx = (int)trans_rect_mp->elements[0][0];
        starty = (int)trans_rect_mp->elements[0][1];
        endx = startx + (int)trans_rect_mp->elements[1][0] - 1;
        endy = starty + (int)trans_rect_mp->elements[1][1] - 1;
        pso("%d %d %d %d\n", startx, starty, endx, endy);
        if(startx < 0 || startx >= width ||
           endx < 0 || endx >= width ||
           starty < 0 || starty >= height ||
           endy < 0 || endy >= height)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    result = (get_zero_image(target_ipp, height, width));
    if (result == ERROR) { EGC(result); }

    result = (get_initialized_int_matrix(&mask_imp, height, width, 0));
    if (result == ERROR) { EGC(result); }

    result = (get_matrix_inverse(&inv_mp, t_mp));
    if (result == ERROR) { EGC(result); }    

    target_ip = *target_ipp;

    for (i = starty; i <= endy; i++)
    {
        for (j = startx; j <= endx; j++)
        {
            /*  transform to get source quad */

            bounds = transform(src_ip, j, i, &corners[0], inv_mp);
            bounds += transform(src_ip, j, i + 1, &corners[1], inv_mp);
            bounds += transform(src_ip, j + 1, i + 1, &corners[2], inv_mp);
            bounds += transform(src_ip, j + 1, i, &corners[3], inv_mp);

            if (bounds == 0)
                continue;

            /* get bounding rectangle */

            box_left = corners[0].x;
            box_right = corners[0].x;
            box_top = corners[0].y;
            box_bottom = corners[0].y;

            /*printf("(%f,%f) (%f,%f) (%f,%f) (%f,%f)\n", corners[0].x, corners[0].y,
                corners[1].x, corners[1].y, corners[2].x, corners[2].y,
                corners[3].x, corners[3].y);*/

            for (k = 1; k < 4; k++)
            {
                if (corners[k].x < box_left)
                    box_left = corners[k].x;
                else if (corners[k].x > box_right)
                    box_right = corners[k].x;
                if (corners[k].y < box_bottom)
                    box_bottom = corners[k].y;
                else if (corners[k].y > box_top)
                    box_top = corners[k].y;
            }

            sum_rectangle(src_ip, box_bottom, box_top, box_left, box_right, &r_sum, &g_sum, &b_sum, &w_sum);

            if (w_sum == 0.0)
            {               
                continue;
            }

            /* subtract outside triangles and rectangles */

            for (k = 0; k < 4; k++)
            {
                k_next = (k + 1) % 4;

                if (corners[k_next].y > corners[k].y)
                {
                    if (corners[k_next].x > corners[k].x)
                    {
                        sum_triangle3(src_ip, corners[k_next].y, corners[k].y, corners[k].x, 
                            corners[k_next].x, &r, &g, &b, &w);

                        r_sum -= r;
                        g_sum -= g;
                        b_sum -= b;
                        w_sum -= w;
                        ASSERT(w_sum > 0.0);

                        if (fabs(corners[k_next].y - box_top) > 1e-8)
                        {
                            sum_rectangle(src_ip, corners[k_next].y, box_top, box_left, corners[k_next].x,
                                &r, &g, &b, &w);

                            r_sum -= r;
                            g_sum -= g;
                            b_sum -= b;
                            w_sum -= w;
                            ASSERT(w_sum > 0.0);
                        }
                    }
                    else
                    {
                        sum_triangle1(src_ip, corners[k_next].y, corners[k].y, corners[k_next].x,
                            corners[k].x, &r, &g, &b, &w);

                        r_sum -= r;
                        g_sum -= g;
                        b_sum -= b;
                        w_sum -= w;
                        ASSERT(w_sum > 0.0);

                        if (fabs(corners[k_next].x - box_left) > 1e-8)
                        {
                            sum_rectangle(src_ip, box_bottom, corners[k_next].y, box_left, corners[k_next].x,
                                &r, &g, &b, &w);

                            r_sum -= r;
                            g_sum -= g;
                            b_sum -= b;
                            w_sum -= w;
                            ASSERT(w_sum > 0.0);
                        }
                    }
                }
                else
                {
                    if (corners[k_next].x > corners[k].x)
                    {
                        sum_triangle4(src_ip, corners[k].y, corners[k_next].y, corners[k].x, 
                            corners[k_next].x, &r, &g, &b, &w);

                        r_sum -= r;
                        g_sum -= g;
                        b_sum -= b;
                        w_sum -= w;
                        ASSERT(w_sum > 0.0);

                        if (fabs(corners[k_next].x - box_right) > 1e-8)
                        {
                            sum_rectangle(src_ip, corners[k_next].y, box_top, corners[k_next].x, box_right,
                                &r, &g, &b, &w);

                            r_sum -= r;
                            g_sum -= g;
                            b_sum -= b;
                            w_sum -= w;
                            ASSERT(w_sum > 0.0);
                        }
                    }
                    else
                    {
                        sum_triangle2(src_ip, corners[k].y, corners[k_next].y, corners[k_next].x, 
                            corners[k].x, &r, &g, &b, &w);

                        r_sum -= r;
                        g_sum -= g;
                        b_sum -= b;
                        w_sum -= w;
                        ASSERT(w_sum > 0.0);

                        if (fabs(corners[k_next].y - box_bottom) > 1e-8)
                        {
                            sum_rectangle(src_ip, box_bottom, corners[k_next].y, corners[k_next].x, box_right,
                                &r, &g, &b, &w);

                            r_sum -= r;
                            g_sum -= g;
                            b_sum -= b;
                            w_sum -= w;
                            ASSERT(w_sum > 0.0);
                        }
                    }
                }
            }

            target_ip->pixels[i][j].r = r_sum / w_sum;
            target_ip->pixels[i][j].g = g_sum / w_sum;
            target_ip->pixels[i][j].b = b_sum / w_sum;
            mask_imp->elements[i][j] = 1;
        }
    }
    
    if (mask_impp != NULL)
    {
        result = copy_int_matrix(mask_impp, mask_imp);
        if (result == ERROR) { EGC(result); }        
    }
      
cleanup:
    free_int_matrix(mask_imp);
    free_matrix(inv_mp);

    return result;
}

/* interpolate a grid by using the biliear interpolation approach
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
int bilinear_inter_image
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

    /*result = (get_zero_image(target_ipp, height, width));*/
    result = (get_initialized_image_2(target_ipp, height, width, WHITE, WHITE, WHITE ));
    if (result == ERROR) { EGC(result); }

    result = (get_initialized_int_matrix(&mask_imp, height, width, 0));
    if (result == ERROR) { EGC(result); }    

    target_ip = *target_ipp;
    
    /* Note on trans_mp: trans_mp[id][0] and trans_mp[id][1] are the
     * xy coordinates of the frame's id'th pixel (where id = row*width + col)
     */
    for(i=starty; i<=endy; i++)
    {
        for(j=startx; j<=endx; j++)
        {
            id = i * width + j;
            x = trans_mp->elements[id][0];
            y = trans_mp->elements[id][1];
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
                /*   pso("%f %f %f\n", r, g, b);*/
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


/* same as bilinear_inter_image, but 
 * applies it to a bounding box.
 * */
int bilinear_inter_image_w_bounding_box
(
    const KJB_image *src_ip,
    const Matrix     *trans_mp,
    int              width,
    int              height,
    int              *slide_box_coords,
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
    int min_x   = slide_box_coords[0];
    int max_x   = slide_box_coords[1];
    int min_y   = slide_box_coords[2];
    int max_y   = slide_box_coords[3];

    Int_matrix *mask_imp = NULL;
    int startx, starty, endx, endy;

    if(target_ipp == NULL) return NO_ERROR;

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

    result = (get_zero_image(target_ipp, height, width));
    if (result == ERROR) { EGC(result); }

    result = (get_initialized_int_matrix(&mask_imp, height, width, 0));
    if (result == ERROR) { EGC(result); }    

    target_ip = *target_ipp;
    
    /* Note on trans_mp: trans_mp[id][0] and trans_mp[id][1] are the
     * xy coordinates of the frame's id'th pixel (where id = row*width + col)
     */
    for(i=starty; i<=endy; i++)
    {
        for(j=startx; j<=endx; j++)
        {
            id = i * width + j;
            x = trans_mp->elements[id][0];
            y = trans_mp->elements[id][1];
            /* If this is not a point in the slide, then ignore*/
            if(x < min_x || y < min_y || x > max_x-1 || y > max_y -1) 
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
                /*   pso("%f %f %f\n", r, g, b);*/
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
        ERE(copy_int_matrix(mask_impp, mask_imp));
    }

cleanup:    
    free_int_matrix(mask_imp);

    return result;
}

/*int get_transformed_bound
(
    const Matrix *a_mp,
    int (*trans_func)(const Matrix *, const Matrix *, Matrix **),
    const Matrix *orig_mp,
    int          bound_width,
    int          bound_height,
    Matrix       **dim_mpp
)
{
    Matrix *mp = NULL;
    Matrix *trans_mp = NULL;
    int         top_left_x;
    int         top_left_y;
    int          width;
    int          height;

    if(dim_mpp == NULL) return NO_ERROR;
    top_left_x = orig_mp->elements[0][0];
    top_left_y = orig_mp->elements[0][1];
    width = orig_mp->elements[1][0];
    height = orig_mp->elements[1][1];

    ERE(get_target_matrix(&mp, 4, 2));
    mp->elements[0][0] = top_left_x;
    mp->elements[0][1] = top_left_y;
    mp->elements[1][0] = width - 1;
    mp->elements[1][1] = top_left_y;
    mp->elements[2][0] = top_left_x;
    mp->elements[2][1] = top_left_y + height - 1;
    mp->elements[3][0] = top_left_x  + width - 1;
    mp->elements[3][1] = top_left_y + height - 1;

    ERE(trans_func(a_mp, mp, &trans_mp));
    ERE(get_bound(trans_mp, bound_width,bound_height, dim_mpp));
    
    free_matrix(mp);
    free_matrix(trans_mp);

    return NO_ERROR;
}*/

/*             get_slide_bound
 * 
 *  This method gets the min/max y/x coordinates of the 
 *  image's projection on the frame. Note that this is different from
 *  get_bound, which returns the projected coordinates of the corners
 *  on the frame.   
 *
 *  Note: It limits the bounds to the min and max of the 
 *        frame's width and height. 
 * 
 *  Author: Quanfu or Jay Torkkola
 *
 */
int get_slide_bound
(
    const Matrix *slide_pos_mp,
    int    width,
    int    height,
    Matrix **bound_mpp
)
{
   double minx;
   double miny;
   double maxx;
   double maxy;
   int i;
   minx = DBL_HALF_MOST_POSITIVE;
   miny = DBL_HALF_MOST_POSITIVE;
   maxx = DBL_HALF_MOST_NEGATIVE;
   maxy = DBL_HALF_MOST_NEGATIVE;

   for(i=0; i<4; i++)
   {
       if(minx > slide_pos_mp->elements[i][0]) minx = slide_pos_mp->elements[i][0];
       if(maxx < slide_pos_mp->elements[i][0]) maxx = slide_pos_mp->elements[i][0];
       if(miny > slide_pos_mp->elements[i][1]) miny = slide_pos_mp->elements[i][1];
       if(maxy < slide_pos_mp->elements[i][1]) maxy = slide_pos_mp->elements[i][1];
   }

   if(minx < 0) minx = 0;
   if(minx >= width)  minx = width - 1;  
   if(miny < 0) miny = 0;
   if(miny >= height) miny = height - 1;
   if(maxx < 0) minx = 0;
   if(maxx >= width)  maxx = width - 1;  
   if(maxy < 0) miny = 0;
   if(maxy >= height) maxy = height - 1;

   ERE(get_target_matrix(bound_mpp, 2, 2));
   if(maxx < minx ) maxx = minx;
   if(maxy < miny ) maxy = miny;

   (*bound_mpp)->elements[0][0] = minx;
   (*bound_mpp)->elements[0][1] = miny;
   (*bound_mpp)->elements[1][0] = maxx - minx + 1;
   (*bound_mpp)->elements[1][1] = maxy - miny + 1;

    return NO_ERROR;
}


/*             get_bound
 * 
 *  Given the bounding coordinates of the slide, 
 *  this returns the resulting bounding coordinates once the slide
 *  is transformed onto the frame.
 *  It seems that the top_left_x, top_left_y, width, and height
 *  are usually (0,0) slide_width and slide_height
 *  but this can be used to take a piece of the slide (like a bullet)
 *
 *  a_mp: the homography the mapes frame->slide (if your trans_func argument
 *        is homography_inverse).
 * 
 *  Author: Quanfu or Jay Torkkola
 *
 */

int get_bound
(
    const Matrix *a_mp,
    int (*trans_func)(const Matrix *, const Matrix *, Matrix **),
    int         top_left_x,
    int         top_left_y,
    int          width,
    int          height,
    Matrix       **dim_mpp
)
{
    Matrix *mp = NULL;
    int result = NO_ERROR;

    if(dim_mpp == NULL) return NO_ERROR;

    ERE(get_target_matrix(&mp, 4, 2));
    mp->elements[0][0] = top_left_x;
    mp->elements[0][1] = top_left_y;
    mp->elements[1][0] = top_left_x + width - 1;
    mp->elements[1][1] = top_left_y;
    mp->elements[2][0] = top_left_x;
    mp->elements[2][1] = top_left_y + height - 1;
    mp->elements[3][0] = top_left_x  + width - 1;
    mp->elements[3][1] = top_left_y + height - 1;

    result = trans_func(a_mp, mp, dim_mpp);
    if (result == ERROR) { EGC(result); }

cleanup:
    free_matrix(mp);
    return result;
}

int get_transformation_from_bound
(
    Matrix **a_mpp,
    const Matrix *bound_mp,
    int (*fit_func)(const Matrix *, const Matrix *, Matrix **, double*),
    int         top_left_x,
    int         top_left_y,
    int          width,
    int          height
)
{
    Matrix *mp = NULL;
    int result = NO_ERROR;

    if(bound_mp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_matrix(&mp, 4, 2));
    mp->elements[0][0] = top_left_x;
    mp->elements[0][1] = top_left_y;
    mp->elements[1][0] = top_left_x + width - 1;
    mp->elements[1][1] = top_left_y;
    mp->elements[2][0] = top_left_x;
    mp->elements[2][1] = top_left_y + height - 1;
    mp->elements[3][0] = top_left_x  + width - 1;
    mp->elements[3][1] = top_left_y + height - 1;

    result = fit_func(bound_mp, mp, a_mpp, NULL);
    if (result == ERROR) { EGC(result); }
    
cleanup:
    free_matrix(mp);
    return result;
}


int combine_image
(
    KJB_image *img1, 
    KJB_image *img2,
    Matrix *x_mp,
    Matrix *y_mp,
    KJB_image **combined_img
)
{
    int i;
    int num_rows;
    int num_cols;

    if(combined_img == NULL) return NO_ERROR;
    
    num_rows = img1->num_rows + img2->num_rows;
    num_cols = MAX_OF(img1->num_cols, img2->num_cols);
   
    ERE(get_zero_image(combined_img, num_rows, num_cols));
    ERE(image_draw_image(*combined_img, img1, 0, 0, 1));
    ERE(image_draw_image(*combined_img, img2, img1->num_rows, 0, 1));

    if(x_mp != NULL && y_mp != NULL)
    {
        for(i=0; i<x_mp->num_rows; i++)
        {
            /*if(x_mp->elements[i][1] <120) continue;*/
            /*if(i%2 != 0) continue;*/
        ERE(image_draw_segment_2(*combined_img, (int)(x_mp->elements[i][1]),
            (int)(x_mp->elements[i][0]), (int)(y_mp->elements[i][1] +
            img1->num_rows), (int)(y_mp->elements[i][0]), 1, 255, 0, 0));
        }
    }

    return NO_ERROR;
}

int ow_overlap_images
(
    KJB_image  *target_ip,
    KJB_image  *src_ip,
    Int_matrix *mask_imp
)
{
    register int i, j;
    int num_rows = target_ip->num_rows;
    int num_cols = target_ip->num_cols;
    
    if(num_rows != src_ip->num_rows || num_rows != mask_imp->num_rows ||
       num_cols != src_ip->num_cols || num_cols != mask_imp->num_cols)
    {
        add_error("Image sizes do NOT match!\n");
        return ERROR;
    }

    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            if(!mask_imp->elements[i][j])
            {
                (target_ip->pixels[i][j]).r = (src_ip->pixels[i][j]).r;
                (target_ip->pixels[i][j]).g = (src_ip->pixels[i][j]).g;
                (target_ip->pixels[i][j]).b = (src_ip->pixels[i][j]).b;
            }
        }
    }
    
    return NO_ERROR;
}

/*
 *  Like ow_overlap_images, but
 * the target image is merged with the source image.
 * That is, whereas overlap_images only 
 * copied pixels from the source image where the mask was 0,
 * This method only copies pixels from the image where the mask is 1.
 * 
 *  
 */
int ow_merge_images
(
    KJB_image  *target_ip,
    KJB_image  *src_ip,
    Int_matrix *mask_imp
)
{
    register int i, j;
    int num_rows = target_ip->num_rows;
    int num_cols = target_ip->num_cols;
    
    if(num_rows != src_ip->num_rows || num_rows != mask_imp->num_rows ||
       num_cols != src_ip->num_cols || num_cols != mask_imp->num_cols)
    {
        add_error("Image sizes do NOT match!\n");
        return ERROR;
    }

    for(i=0; i<num_rows; i++)
    {
        for(j=0; j<num_cols; j++)
        {
            if(mask_imp->elements[i][j])
            {
                (target_ip->pixels[i][j]).r = (src_ip->pixels[i][j]).r;
                (target_ip->pixels[i][j]).g = (src_ip->pixels[i][j]).g;
                (target_ip->pixels[i][j]).b = (src_ip->pixels[i][j]).b;
            }
        }
    }
    
    return NO_ERROR;
}

void draw_slide_region
(
    KJB_image *ip,
    Matrix    *slide_pos_mp,
    int       r,
    int       g,
    int       b
    )
{
    image_draw_segment_2(ip,
                         (int)slide_pos_mp->elements[0][1],
                         (int)slide_pos_mp->elements[0][0],
                         (int)slide_pos_mp->elements[1][1],
                         (int)slide_pos_mp->elements[1][0],
                         1,
                         r,
                         g,
                         b);
    image_draw_segment_2(ip,
                         (int)slide_pos_mp->elements[1][1],
                         (int)slide_pos_mp->elements[1][0],
                         (int)slide_pos_mp->elements[3][1],
                         (int)slide_pos_mp->elements[3][0],
                         1,
                         r,
                         g,
                         b);
    image_draw_segment_2(ip,
                         (int)slide_pos_mp->elements[3][1],
                         (int)slide_pos_mp->elements[3][0],
                         (int)slide_pos_mp->elements[2][1],
                         (int)slide_pos_mp->elements[2][0],
                         1,
                         r,
                         g,
                         b);
    image_draw_segment_2(ip,
                         (int)slide_pos_mp->elements[2][1],
                         (int)slide_pos_mp->elements[2][0],
                         (int)slide_pos_mp->elements[0][1],
                         (int)slide_pos_mp->elements[0][0],
                         1,
                         r,
                         g,
                         b);
}


int ow_SM_subtract_images(KJB_image* in1_ip, const KJB_image* in2_ip, const
Int_matrix* mask_imp)
{
    Pixel* in1_pos;
    Pixel* in2_pos;
    int    i, j, num_rows, num_cols;


    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if(mask_imp && !(mask_imp->elements[i][j])) 
            {
                in1_pos->r = 255;
                in1_pos->g = 0;
                in1_pos->b = 0;
            }
            else
            {
                in1_pos->r -= in2_pos->r;
                in1_pos->g -= in2_pos->g;
                in1_pos->b -= in2_pos->b;

                in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
                in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
                in1_pos->extra.invalid.r     |= in2_pos->extra.invalid.r;
                in1_pos->extra.invalid.pixel |= in2_pos->extra.invalid.pixel;
            }

            in1_pos++;
            in2_pos++;
        }
    }

    return NO_ERROR;
}

int SM_subtract_images
(
    KJB_image**       out_ipp,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip,
    const Int_matrix *mask_imp
)
{
    KJB_image* out_ip;
    Pixel*     in1_pos;
    Pixel*     in2_pos;
    Pixel*     out_pos;
    int        i, j, num_rows, num_cols;
   

    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;
    
    ERE(get_target_image(out_ipp, num_rows, num_cols));
    out_ip = *out_ipp;

    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];
        out_pos =  out_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if(mask_imp && !(mask_imp->elements[i][j])) 
            {
                out_pos->r = 255;
                out_pos->g = 0;
                out_pos->b = 0;
            }
            else
            {
                out_pos->r = fabs(in1_pos->r - in2_pos->r);
                out_pos->g = fabs(in1_pos->g - in2_pos->g);
                out_pos->b = fabs(in1_pos->b - in2_pos->b);
            }

            in1_pos++;
            in2_pos++;
            out_pos++;
        }
    }

    return NO_ERROR;
}

int SM_get_images_difference
(
    double           *diff_ptr,
    const KJB_image  *in1_ip,
    const KJB_image  *in2_ip,
    const Int_matrix *mask_imp
)
{
    Pixel*     in1_pos;
    Pixel*     in2_pos;
    int        i, j, num_rows, num_cols;
    double r;
    double sum;
    int count;

    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    count = 0;
    sum = 0.0;
    for (i=0; i<num_rows; i++)
    {
        in1_pos  = in1_ip->pixels[ i ];
        in2_pos  = in2_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if(mask_imp && !(mask_imp->elements[i][j])) 
            {
                continue;
            }
            else
            {
                r = in1_pos->r - in2_pos->r;
                sum += (r * r);
                r = in1_pos->g - in2_pos->g;
                sum += (r * r);
                r = in1_pos->b - in2_pos->b;
                sum += (r * r);
                count++;

            }

            in1_pos++;
            in2_pos++;
        }
    }

    if(diff_ptr) *diff_ptr = sqrt(sum);

    return count;
}

int SM_get_images_difference_1
(
    double           *diff_ptr,
    const KJB_image  *in1_ip,
    const KJB_image  *in2_ip,
    const Matrix     *slide_bound_mp,
    const Int_matrix *mask_imp
)
{
    int        i, j, num_rows, num_cols;
    double r;
    double sum;
    int sr, er;
    int sc, ec;
    int count;

    ERE(check_same_size_image(in1_ip, in2_ip));

    num_rows = in1_ip->num_rows;
    num_cols = in1_ip->num_cols;

    sr = (int)(slide_bound_mp->elements[0][1]);
    sc = (int)(slide_bound_mp->elements[0][0]);
    er = sr + (int)(slide_bound_mp->elements[1][1]);
    ec = sc + (int)(slide_bound_mp->elements[1][0]);
    if(er > num_rows) er = num_rows;
    if(ec > num_cols) ec = num_cols;

    count = 0;
    sum = 0.0;
    for (i=sr; i<er; i++)
    {
        for (j=sc; j<ec; j++)
        {
            if(mask_imp && !(mask_imp->elements[i][j])) 
            {
                continue;
            }
            else 
            {
                r = (in1_ip->pixels[i][j]).r - (in2_ip->pixels[i][j]).r;
                sum += (r * r);
                r = (in1_ip->pixels[i][j]).g - (in2_ip->pixels[i][j]).g;
                sum += (r * r);
                r = (in1_ip->pixels[i][j]).b - (in2_ip->pixels[i][j]).b;
                sum += (r * r);
                count++;
            }
        }
    }

    if(diff_ptr) *diff_ptr = sqrt(sum);

    return count;
}

