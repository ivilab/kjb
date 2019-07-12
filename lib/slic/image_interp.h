/* $Id: image_interp.h 15688 2013-10-14 08:46:32Z predoehl $
 */
#ifndef SLIC_IMAGE_INTERP_DEFINED_H_
#define SLIC_IMAGE_INTERP_DEFINED_H_

#include "slic/basic.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif



int transform_image
(
    const KJB_image *src_ip,
    const Matrix    *t_mp,
    int             width,
    int             height,
    const Matrix     *trans_rect_mp,
    int             inter_method,
    KJB_image       **target_ipp,
    Int_matrix      **map_impp
);
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
);


int bilinear_inter_image
(
    const KJB_image *src_ip,
    const Matrix     *trans_mp,
    int             width,
    int             height,
    const Matrix     *trans_rect_mp,
    KJB_image        **target_ipp,
    Int_matrix      **map_impp
);

int bilinear_inter_image_w_bounding_box
(
    const KJB_image *src_ip,
    const Matrix     *trans_mp,
    int              width,
    int              height,
    int              *slide_coords,
    const Matrix     *trans_rect_mp,
    KJB_image        **target_ipp,
    Int_matrix       **mask_impp
);

int interp
(
    const KJB_image *src_ip,
    const Matrix     *trans_mp,
    int             width,
    int             height,
    const Matrix     *trans_rect_mp,
    KJB_image        **target_ipp,
    Int_matrix      **map_impp,
    const Matrix    *t_mp
);

/*int get_transformed_bound
(
    const Matrix *a_mp,
    int (*trans_func)(const Matrix *, const Matrix *, Matrix **),
    const Matrix *orig_mp,
    int          bound_width,
    int          bound_height,
    Matrix       **dim_mpp
);
int get_bound
(
    const Matrix *slide_pos_mp,
    int    width,
    int    height,
    Matrix **bound_mpp
);*/

int get_slide_bound
(
    const Matrix *slide_pos_mp,
    int    width,
    int    height,
    Matrix **bound_mpp
);


int get_bound
(
    const Matrix *a_mp,
    int (*trans_func)(const Matrix *, const Matrix *, Matrix **),
    int          top_left_x,
    int          top_left_y,
    int          width,
    int          height,
    Matrix       **dim_mpp
);

int get_transformation_from_bound
(
    Matrix **a_mpp,
    const Matrix *bound_mp,
    int (*fit_func)(const Matrix *, const Matrix *, Matrix **, double*),
    int         top_left_x,
    int         top_left_y,
    int          width,
    int          height
);

int combine_image
(
    KJB_image *img1, 
    KJB_image *img2,
    Matrix *x_mp,
    Matrix *y_mp,
    KJB_image **combined_img
);

int ow_merge_images
(
    KJB_image  *target_ip,
    KJB_image  *src_ip,
    Int_matrix *mask_imp
  );
 
int ow_overlap_images
(
    KJB_image       *target_ip,
    KJB_image       *src_ip,
    Int_matrix      *mask_imp
);

void draw_slide_region
(
    KJB_image *ip,
    Matrix    *slide_pos_mp,
    int r,
    int g,
    int b
);

int SM_subtract_images
(
    KJB_image**      out_ipp,
    const KJB_image* in1_ip,
    const KJB_image* in2_ip,
    const Int_matrix *mask_imp
);
int ow_SM_subtract_images
(
    KJB_image*        in1_ip, 
    const KJB_image*  in2_ip, 
    const Int_matrix* mask_imp
);

int SM_get_images_difference
(
    double           *diff_ptr,
    const KJB_image  *in1_ip,
    const KJB_image  *in2_ip,
    const Int_matrix *mask_imp
);

int SM_get_images_difference_1
(
    double           *diff_ptr,
    const KJB_image  *in1_ip,
    const KJB_image  *in2_ip,
    const Matrix     *slide_pos_mp,
    const Int_matrix *mask_imp
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
