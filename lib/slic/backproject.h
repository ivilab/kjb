#ifndef SLIC_BACKPROJECT_DEFINED_H_
#define SLIC_BACKPROJECT_DEFINED_H_


#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

/* Kobus. Dot H files should be stand alone. */

#include "i/i_incl.h" 

int back_project_frame_slide_to_slide
 (
  const KJB_image *frame_ip,
  int slide_width,
  int slide_height,
  const Matrix    *H_mp,
  int             fitting_model,
  KJB_image       **bp_ipp,
  Int_matrix      **mask_impp
   );

/* int back_project_frame_slide_to_slide_enlarged */
/*  ( */
/*   const KJB_image *frame_ip, */
/*   int slide_width, */
/*   int slide_height, */
/*   int enlarged_slide_width, */
/*   int enlarged_slide_height, */
/*   const Matrix    *H_mp, */
/*   int             fitting_model, */
/*   KJB_image       **bp_ipp, */
/*   Int_matrix      **mask_impp */
/*    ); */


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
   );

/* 
 * Kobus: 2019/09/06.
 * This code is more or less duplicated in lib/i_geometric/i_geometric_maping.c.
 * It is not clear which is newer. Both could use rationalization. My guess is
 * that we have an unfinished attempt to put slic code that is not secficic to
 * slic elsewhere. For now, I have renamed the version in
 * lib/i_geometric/i_geometric_maping.c to "back_project_image_new". 
*/
int back_project_image
(
 const KJB_image *ip,
 const Matrix    *H_mp,
 int             fitting_model,
 KJB_image       **bp_ipp,
 Int_matrix      **mask_impp
 );


int get_enlarged_frame_dimensions
  (
   const Matrix *slide_to_frame_mp,
   int slide_width,
   int slide_height,
   int frame_width,
   int frame_height,
   Int_matrix **enlarged_frame_dimensions
  );

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif
