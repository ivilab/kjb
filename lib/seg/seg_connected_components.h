
/* $Id: seg_connected_components.h 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               ||
|        Kobus Barnard.                                                        |
|        Prasad Gabbur.                                                        |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */


#ifndef CONNECTED_COMPONENTS_MODULE_INCLUDED
#define CONNECTED_COMPONENTS_MODULE_INCLUDED


/*
// Exported interfaces go here.
*/

#include "m/m_incl.h"
#include "n/n_incl.h"
#include "t3/t3_incl.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif

    
typedef struct KJB_region
{
    long               label;
    long               root_label;
    long               moment_00;
    long               moment_10;
    long               moment_01;
    long               min_row;
    long               max_row;
    long               min_col;
    long               max_col;
    struct KJB_region* next_region_ptr;
}
KJB_region;

typedef struct KJB_region_list
{
    int         num_regions;
    KJB_region* regions;
}
KJB_region_list;

int  get_target_region_list(KJB_region_list** rlpp, int num_regions);
void free_region_list      (KJB_region_list* rlp);

int label_four_connected_regions 
(
    Int_matrix**      out_mpp,
    KJB_region_list** region_list_rlpp,
    Segmentation_t3** segmentation_ptr_ptr,
    int*              num_regions_intp,
    int*              max_label_intp,
    const KJB_image*  in_ip,
    const Int_matrix* in_mp,
    const int         merge_eqvlnt_labels 
);

int label_eight_connected_regions
(
    Int_matrix**      out_mpp,
    KJB_region_list** region_list_rlpp,
    Segmentation_t3** segmentation_ptr_ptr,
    int*              num_regions_intp,
    int*              max_label_intp,
    const KJB_image*  in_ip,
    const Int_matrix* in_mp,
    const int         merge_eqvlnt_labels 
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif    /*   #include this file            */


