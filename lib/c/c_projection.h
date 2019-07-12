
/* $Id: c_projection.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef C_PROJECTION_INCLUDED
#define C_PROJECTION_INCLUDED


#include "m/m_gen.h"
#include "c/c_type.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


typedef enum Invalid_chromaticity_action
{
    ALL_CHROMATICITIES_MUST_BE_VALID,
    ZERO_INVALID_CHROMATICITY,
    NEGATE_INVALID_CHROMATICITY,
    TRUNCATE_INVALID_CHROMATICITY,
    SKIP_INVALID_CHROMATICITY
}
Invalid_chromaticity_action;


int get_projection_matrix
(
    Matrix**                    output_mpp,
    const Matrix*               input_mp,
    Projection_method           projection_method,
    Invalid_chromaticity_action invalid_chromaticity_action,
    double                      max_projection_coordinate,
    double                      absolute_pixel_error,
    double                      relative_pixel_error,
    Matrix**                    error_box_mpp,
    Matrix**                    error_points_mpp
);

int get_divide_by_sum_projection_matrix
(
    Matrix**                    output_mpp,
    const Matrix*               input_mp,
    Invalid_chromaticity_action invalid_chromaticity_action,
    double                      max_projection_coordinate,
    double                      absolute_pixel_error,
    double                      relative_pixel_error,
    Matrix**                    error_points_mpp
);

int project_matrix
(
    Matrix**          output_mpp,        /* Output chromaticity matrix.*/
    const Matrix*     input_mp,          /* Input RGB matrix.          */
    Projection_method projection_method  /* Type of chromaticity.      */
);

int divide_by_sum_project_matrix
(
    Matrix**      output_mpp,
    const Matrix* input_mp
);

int project_matrix_onto_unit_sphere
(
    Matrix**      output_mpp,
    const Matrix* input_mp
);

int coord_plane_project_matrix
(
    Matrix**      output_mpp,
    const Matrix* input_mp,
    int           coord_1,
    int           coord_2
);

int get_projection_vector
(
    Vector**          output_vpp,
    const Vector*     input_vp,
    Projection_method index,
    double            max_projection_coordinate
);

int project_vector
(
    Vector**          output_vpp,
    const Vector*     input_vp,
    Projection_method projection_method
);

int back_project_matrix
(
    Matrix**          output_mpp,
    const Matrix*     input_mp,
    Projection_method index
);

int back_project_vector
(
    Vector**          output_vpp,
    const Vector*     input_vp,
    Projection_method index
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

