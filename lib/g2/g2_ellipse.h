
/* $Id: g2_ellipse.h 4727 2009-11-16 20:53:54Z kobus $ */

/*
    Copyright (c) 1994-2008 by Kobus Barnard (author).

    Personal and educational use of this code is granted, provided
    that this header is kept intact, and that the authorship is not
    misrepresented. Commercial use is not permitted.
*/

#ifndef G2_ELLIPSE_INCLUDED
#define G2_ELLIPSE_INCLUDED


#include "m/m_gen.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */

typedef struct Ellipse
{
    Vector* offset_vp;
    Matrix* rotation_mp;
    double a;
    double b;
    double max_x;
    double max_y;
    double error_1;
    double error_2;
    double error_3;
    double r_chrom_ave;
    double g_chrom_ave;
    double R_ave;
    double G_ave;
    double B_ave;
    int    seg_index;
}
Ellipse;


typedef struct Ellipse_list
{
    int num_ellipses;
    Ellipse** ellipses;
}
Ellipse_list;


#ifdef TRACK_MEMORY_ALLOCATION

#   define create_ellipse_list(x) \
                   debug_create_ellipse_list(x, __FILE__, __LINE__)

#   define get_target_ellipse_list(x,y)   \
                   debug_get_target_ellipse_list(x,y, __FILE__, __LINE__)


    Ellipse_list* debug_create_ellipse_list
    (
        int         len,
        const char* file_name,
        int         line_number
    );

    int debug_get_target_ellipse_list
    (
        Ellipse_list** target_list_ptr_ptr,
        int            num_ellipses,
        const char*    file_name,
        int            line_number
    );

#else
    Ellipse_list* create_ellipse_list(int len);

    int get_target_ellipse_list
    (
        Ellipse_list** out_arg_list_ptr,
        int            num_ellipses
    );

#endif

int ow_multiply_ellipse_by_scalar(Ellipse* ellipse_ptr, double factor);

void free_ellipse_list(Ellipse_list* list_ptr);
void free_ellipse(Ellipse* ellipse_ptr);


#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif


