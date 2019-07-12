
/* $Id: wrap_fftw.h 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision|
|  group (the authors) including                                               |
|        Kobus Barnard.                                                        |
|                                                                              |
|  (Copyright only applies to the wrapping code, not the wrapped code).        |
|                                                                              |
|  Personal and educational use of this code is granted, provided that this    |
|  header is kept intact, and that the authorship is not misrepresented, that  |
|  its use is acknowledged in publications, and relevant papers are cited.      |
|                                                                              |
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).         |
|                                                                              |
|  Please note that the code in this file has not necessarily been adequately  |
|  tested. Naturally, there is no guarantee of performance, support, or fitness|
|  for any particular task. Nonetheless, I am interested in hearing about      |
|  problems that you encounter.                                                |
|                                                                              |
* =========================================================================== */

#ifndef WRAP_FFTW_INCLUDED
#define WRAP_FFTW_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


#ifdef BACKWARDS_COMPATABLE
#    define WRAP_FFTW_DEFAULT_STYLE FFTW_DEFAULT_STYLE
#    define WRAP_FFTW_NORMALIZE_STYLE FFTW_NORMALIZE_STYLE
#    define WRAP_FFTW_MATLAB_STYLE FFTW_MATLAB_STYLE

#    define set_wrap_fftw_style set_fftw_style
#endif 

typedef enum Wrap_fftw_style
{
    FFTW_DEFAULT_STYLE,
    FFTW_NORMALIZE_STYLE,
    FFTW_MATLAB_STYLE
}
Wrap_fftw_style;


void set_fftw_style(Wrap_fftw_style fftw_style);

/* Backwards compatiblity. To be removed after spring term, 2006. */
void set_wrap_fftw_style(Wrap_fftw_style wrap_fftw_style);

int get_matrix_dct        (Matrix** output_mpp, const Matrix* input_mp);
int get_matrix_inverse_dct(Matrix** output_mpp, const Matrix* input_mp);

int get_matrix_dft
(
    Matrix**      output_re_mpp,
    Matrix**      output_im_mpp,
    const Matrix* input_re_mp,
    const Matrix* input_im_mp
);

int get_matrix_inverse_dft
(
    Matrix**      output_re_mpp,
    Matrix**      output_im_mpp,
    const Matrix* input_re_mp,
    const Matrix* input_im_mp
);

int get_vector_dft
(
    Vector**      output_re_vpp,
    Vector**      output_im_vpp,
    const Vector* input_re_vp,
    const Vector* input_im_vp
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif   /* Kobus */



