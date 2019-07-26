
/* $Id: c_colour_space.h 4727 2009-11-16 20:53:54Z kobus $ */

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

#ifndef C_COLOUR_SPACE_INCLUDED
#define C_COLOUR_SPACE_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#define LAB_F(arg)  \
            ((arg > 0.008856) ? pow(arg, 1.0/3.0) : 7.787 * arg + 16.0 / 116.0)


int set_colour_space_options(const char* option, const char* value);
int print_colour_space_options(void);

int get_rgb_to_xyz_matrix(Matrix** mpp);

int get_reciprocal_of_white_xyz
(
    Vector**      reciprocal_of_white_xyz_vpp,
    const Vector* white_rgb_vp
);

int convert_matrix_rgb_to_lab
(
    Matrix**      out_mpp,
    const Matrix* in_mp,
    const Vector* white_rgb_vp
);

int convert_matrix_rgb_to_xyz(Matrix** out_ipp, const Matrix* in_ip);

int convert_vector_rgb_to_lab
(
    Vector**      out_vpp,
    const Vector* in_vp,
    const Vector* white_rgb_vp
);

int convert_vector_rgb_to_xyz(Vector** out_vpp, const Vector* in_vp);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif 
}
#endif

#endif


