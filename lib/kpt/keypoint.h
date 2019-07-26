/* $Id */

#ifndef LIB_KEYPOINT_DEFINED_H_
#define LIB_KEYPOINT_DEFINED_H_

#include <m/m_incl.h>
#include <i/i_incl.h>

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define KEYPOINT_DESCRIP_LENGTH 128

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION
/*#    ifdef PROGRAMMER_IS_kobus */
         /* This a bit dangerous as often freeing non-initialized memory is not
          * really a bug. */
#        define CHECK_KEYPOINT_INITIALZATION_ON_FREE
/*#    endif */
#endif

/* -------------------------------------------------------------------------- */
typedef struct Keypoint
{
    float   row;        /* Subpixel location of keypoint */
    float   col;
    float   scale;
    float   ori;        /* Orientation: range [-PI, PI] */
    Vector  *descrip;   /* vector of descriptor values */
} 
Keypoint;

typedef struct Keypoint_vector
{
    int         length;
    Keypoint**  elements;
} 
Keypoint_vector;

typedef struct Keypoint_vector_vector
{
    int                length;
    Keypoint_vector**  elements;
} 
Keypoint_vector_vector;

int set_keypoint_options(const char* option, const char* value);

int get_target_keypoint (Keypoint ** target_kpp);
/*
int set_keypoint
( 
    Keypoint*     kpt_ptr,
    const float   row,
    const float   col,
    const float   scale,
    const float   ori,
    const int     img_id,
    const Vector  *descr
);
*/
Keypoint    *create_keypoint( void );


int set_keypoint_from_vector
( 
    Keypoint**      kvpp,
    const Vector    *kpt_val_vp
);

int copy_keypoint
(
    Keypoint** target_kpp,
    const Keypoint* source_kp
);


void free_keypoint( Keypoint *kpt_ptr );

int read_vl_keypoints_into_matrix
(
    const char      *fname,
    Matrix**        kpt_mpp
);

int extract_keypoints_positions
(
    Matrix**                  pos_mpp,
    const Keypoint_vector     *kvp
);

int extract_selected_keypoints_positions
(
    Matrix**                  pos_mpp,
    const Keypoint_vector     *kvp,
    const Int_vector          *mask_ivp
);

int copy_keypoint_vector
(
    Keypoint_vector** target_kvpp,
    const Keypoint_vector* source_kvp
);


int copy_keypoint_vector_selected_rows
(
    Keypoint_vector** target_kvpp,
    const Keypoint_vector* source_kvp,
    const Int_vector* mask_ivp
);

void free_keypoint_vector (Keypoint_vector* kvp);
void free_keypoint_vector_descriptors (Keypoint_vector* kvp);

Keypoint_vector* create_keypoint_vector(int length);

int get_target_keypoint_vector(Keypoint_vector** kvpp, int length);


int get_keypoint_vector_from_matrix 
(
    Keypoint_vector** kvpp,
    const Matrix* mp
);

int read_vl_keypoint_vector_from_file
(
    const char *                fname,
    Keypoint_vector**           output_kvpp
);

int write_vl_keypoint_vector
(
    const Keypoint_vector* kvp,
    const char* file_name
);

int get_matrix_from_keypoint_vector
(
    Matrix** mp,
    const Keypoint_vector* kvp
);

int get_vector_from_keypoint
(
    Vector** vp,
    const Keypoint* kvp
);

int get_vector_vector_from_keypoint_vector
(
    Vector_vector** vvp,
    const Keypoint_vector* kvp
);

int get_target_keypoint_vector_vector(Keypoint_vector_vector** kvvpp, int length);

Keypoint_vector_vector* create_keypoint_vector_vector(int length);

void free_keypoint_vector_vector(Keypoint_vector_vector* kvvp);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
int draw_oriented_keypoint 
( 
    KJB_image* ip, 
    float x, 
    float y, 
    float scale, 
    float rad 
); 

int draw_oriented_keypoint_1 
( 
    KJB_image* ip, 
    float x, 
    float y, 
    float scale, 
    float rad,
    int red, int green, int blue  
);

int draw_vl_keypoints_from_file (KJB_image* ip, char *keypoint_filename);

int draw_ubc_keypoints_from_file (KJB_image* ip, char *keypoint_filename);

int fp_draw_vl_keypoints(FILE* fp, KJB_image* ip);

int draw_keypoints_from_keypoint_vector
(
  KJB_image* ip, 
  const Keypoint_vector* keypoint_kvp
);

int draw_vl_keypoint_vector_with_mask
(
  KJB_image*                ip, 
  const Keypoint_vector*    keypoint_kvp,
  const Int_vector*         mask_ivp
);


int draw_vl_keypoint_vector_with_mask_value
(
  KJB_image*                ip, 
  const Keypoint_vector*    keypoint_kvp,
  const Int_vector*         mask_ivp,
  const int                 val
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */
 
int keypoint_euclidean_distance
(
    Vector* kpt1_descr_vp,
     Vector* kpt2_descr_vp,
     double* distance_ptr
 );
 
int get_keypoint_match 
(
    const Keypoint        *target_kpt,
    const Keypoint_vector *candidate_kvp,
    const double          dist_ratio
);

int get_local_keypoint_match 
(
    const Keypoint        *target_kpt,
    const Keypoint_vector *candidate_kvp,
    const double          dist_ratio,
    const double          dist_thresh
);
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int draw_keypoint_correspondences
(
    const KJB_image           *img1_ip,
    const KJB_image           *img2_ip,
    const Keypoint_vector     *img1_kvp,
    const Keypoint_vector     *img2_kvp,
    KJB_image                 **result_ip
);

int draw_keypoint_matches
(
    const KJB_image           *img1_ip,
    const KJB_image           *img2_ip,
    const Keypoint_vector     *img1_kvp,
    const Keypoint_vector     *img2_kvp,
    const Int_matrix          *match_imp,
    KJB_image                 **result_ip
);

int draw_keypoint_matches_1
(
    const KJB_image           *img1_ip,
    const KJB_image           *img2_ip,
    const Keypoint_vector     *img1_kvp,
    const Keypoint_vector     *img2_kvp,
    const Int_vector          *match_ivp,
    KJB_image                 **result_ip
);
/*
int set_keypoint_color
(
    const int red,
    const int green,
    const int blue
);
*/
int set_keypoint_color
(
    const char* red,
    const char* green,
    const char* blue
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif /* LIB_KEYPOINT_DEFINED_H_ */
