
/* $Id: v_illum.h 4727 2009-11-16 20:53:54Z kobus $ */

/*
    Copyright (c) 1994-2008 by Kobus Barnard (author).

    Personal and educational use of this code is granted, provided
    that this header is kept intact, and that the authorship is not
    misrepresented. Commercial use is not permitted.
*/

#ifndef V_ILLUM_INCLUDED
#define V_ILLUM_INCLUDED


#include "l/l_def.h"

#include "h/h_hull.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


int is_possible_illum_change
(
    double R_illum,
    double G_illum,
    double B_illum,
    double R_non_illum,
    double G_non_illum,
    double B_non_illum,
    double relative_error
);

int is_possible_illum_ratio(double r_ratio, double g_ratio);

int is_possible_shadow_change
(
    double R_shadow,
    double G_shadow,
    double B_shadow,
    double R_non_shadow,
    double G_non_shadow,
    double B_non_shadow,
    double relative_error
);

int is_possible_shadow_ratio
(
    double R_ratio,
    double G_ratio,
    double B_ratio
);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


