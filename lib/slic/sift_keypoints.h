/* $Id: sift_keypoints.h 15688 2013-10-14 08:46:32Z predoehl $
 */
#ifndef SLIC_SIFT_KEYPOINTS_INCLUDED
#define SLIC_SIFT_KEYPOINTS_INCLUDED


#include "m/m_gen.h"
#include "i/i_type.h"
#include "i/i_float.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

#include "i/i_type.h"
#include "i/i_float.h"

typedef struct SIFT_kp
{
  int num_keypoints;
  float row;
  float col;
  float scale;
  float orientation;
  Int_vector  *elements;   
}
SIFT_kp;

typedef struct SIFT_kp_list
{
  int length;
  SIFT_kp **elements;  
}
SIFT_kp_list;

int get_target_SIFT_kp(SIFT_kp **kp, 
               float row,
               float col,
               float scale,
               float orientation,
               Int_vector* vp);

int get_target_SIFT_Kp_list(SIFT_kp_list **sift_kp_list_pp, int num_elements);   

int read_SIFT_kp_list(SIFT_kp_list **sift_kp_list_pp, char *keypoint_filename);

int free_SIFT_kp_list(SIFT_kp_list *sift_kp_p);

int free_SIFT_kp(SIFT_kp *sift_kp);

int draw_keypoints_on_image(char *image_filename, char *keypoints_filename);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


