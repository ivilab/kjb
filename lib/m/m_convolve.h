
/* $Id: m_convolve.h 5797 2010-04-08 01:39:17Z ksimek $ */

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

#ifndef M_CONVOLVE_INCLUDED
#define M_CONVOLVE_INCLUDED


#include "m/m_def.h"
#include "m/m_type.h"
#include "m/m_matrix.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int gauss_convolve_matrix
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    double        sigma
);

int convolve_matrix
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Matrix* mask_mp
);

int x_convolve_matrix
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Vector* mask_vp
);

int y_convolve_matrix
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Vector* mask_vp
);

int convolve_vector
(
    Vector**      out_vpp,
    const Vector* in_vp,
    const Vector* mask_vp
);

int get_2D_gaussian_mask
(
    Matrix** mask_mpp,  /* Output gaussian smoothing mask. */
    int      mask_size, /* Number of elements per axis. Must be odd. */
    double   sigma      /* Standard deviation in bin units.         */
);

int get_2D_gaussian_mask_2
(
    Matrix** mask_mpp,  /* Output gaussian smoothing mask. */
    int      num_rows, /* Number of rows. Must be odd. */
    int      num_cols, /* Number of cols. Must be odd. */
    double   row_sigma,      /* Standard deviation in bin units.         */
    double   col_sigma      /* Standard deviation in bin units.         */
);

int get_2D_gaussian_dx_mask
(
    Matrix** mask_mpp,  /* Output gaussian smoothing mask.   */
    int      num_rows,  /* Number of rows in mask.      */
    int      num_cols,  /* Number of cols in mask.      */
    double   row_sigma,  /* Standard deviation in row direction in bin units.  */
    double   col_sigma  /* Standard deviation in column direction bin units.  */
);

int get_2D_gaussian_dy_mask
(
    Matrix** mask_mpp,  /* Output gaussian smoothing mask.   */
    int      num_rows,  /* Number of rows in mask.      */
    int      num_cols,  /* Number of cols in mask.      */
    double   row_sigma,  /* Standard deviation in row direction in bin units.  */
    double   col_sigma  /* Standard deviation in column direction bin units.  */
);

int get_1D_gaussian_mask
(
    Vector** mask_vpp,  /* Output gaussian smoothing mask.           */
    int      mask_size, /* Number of elements per axis. Must be odd. */
    double   sigma      /* Standard deviation in bin units.         */
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


