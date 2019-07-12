
/* $Id: seg_spots.h 17796 2014-10-21 04:17:21Z predoehl $ */

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

#ifndef SEG_SPOTS_INCLUDED
#define SEG_SPOTS_INCLUDED


#include <l/l_int_matrix.h>
#include <m/m_matrix.h>
#include <m/m_vector.h>
#include <i/i_float.h>
#include <i/i_seq.h>

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

/* -------------------------------------------------------------------------- */

int find_bright_spots_in_image
(
    const KJB_image*    img,
    const Matrix*       background,
    const Matrix*       thresholds,
    int                 min_brightness,
    int                 min_size,
    int                 max_size,
    double              similarity,
    Int_matrix_vector** spots,
    Vector_vector**     centroids
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int find_bright_spots_in_image_sequence
(
    V_v_v**                    points,
    Int_matrix_vector_vector** blobs,
    const KJB_image_sequence*  images,
    int                        min_brightness,
    int                        min_blob_size,
    int                        max_blob_size,
    double                     similarity
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


