
/* $Id: i_seq.h 10617 2011-09-29 19:50:47Z predoehl $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
* =========================================================================== */

#ifndef I_SEQ_INCLUDED
#define I_SEQ_INCLUDED

#include "l/l_incl.h"
#include "m/m_matrix.h"
#include "i/i_float.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

/* -------------------------------------------------------------------------- */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                   KJB_image_sequence
 *
 * Sequence of images type
 *
 * A type which implements a sequence of images
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
 */

typedef struct KJB_image_sequence
{
    int length;
    KJB_image** elements;
}
KJB_image_sequence;

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_target_image_sequence(KJB_image_sequence** target_ispp, int length);

int read_image_sequence(KJB_image_sequence** target_ispp, const Word_list* filenames);

void free_image_sequence(KJB_image_sequence* isp);

int average_bw_images
(
    KJB_image**               avg_img,
    const KJB_image_sequence* images
);

int std_dev_bw_images
(
    KJB_image**               std_dev_img,
    const KJB_image_sequence* images,
    const KJB_image*          avg_img
);

int average_bw_images_2
(
    Matrix**                  avg_img_mat,
    const KJB_image_sequence* images
);

int std_dev_bw_images_2
(
    Matrix**                  std_dev_img_mat,
    const KJB_image_sequence* images,
    const Matrix*             avg_img_mat
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


