
/* $Id: seg_connected_components.c 15487 2013-10-03 22:04:16Z predoehl $ */

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


#include "seg/seg_connected_components.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                           get_target_region_list
 *
 * Gets target region list.
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of region list.  If *target_rlpp is NULL, then this
 * routine creates the region list.  If it is not NULL, and it is the right
 * size, then this routine does nothing.  If it is the wrong size, then it is
 * resized.
 *
 * In order to be memory efficient, we free before resizing. If the resizing
 * fails, then the original contents of the *target_rlpp will be lost.
 * However,
 * (*target_rlpp)->regions will be set to NULL, so (*target_rlpp) can be safely
 * sent to kjb_free().  Note that this is in fact the convention throughout
 * the KJB library:  if destruction on failure is a problem (usually when
 * *target_rlpp is global) then work on a copy!
 *
 *
 * Index: segmentation
 * -----------------------------------------------------------------------------
*/

int get_target_region_list(KJB_region_list** target_rlpp, int num_regions)
{
    KJB_region_list* temp_rlp;
    KJB_region_list* rlp = *target_rlpp;

    if (num_regions < 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (rlp == NULL)
    {
        NRE(temp_rlp = TYPE_MALLOC(KJB_region_list));
        rlp = temp_rlp;
        
        NRE(rlp->regions = N_TYPE_MALLOC(KJB_region, num_regions));
        rlp->num_regions = num_regions;
        
        *target_rlpp = rlp;
    }
    else if (num_regions == rlp->num_regions)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else if (num_regions < rlp->num_regions)
    {
        rlp->num_regions = num_regions;
    }
    else
    {
        kjb_free(rlp->regions);

        if (num_regions > 0)
        {
            NRE(rlp->regions = N_TYPE_MALLOC(KJB_region, num_regions));
        }
        else
        {
            rlp->regions = NULL;
        }

        rlp->num_regions = num_regions;
    }

    return NO_ERROR; 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             free_region_list
 *
 * Frees the space associated with a KJB_region_list.
 *
 * This routine frees the storage associated with a KJB_region_list obtained from
 * get_target_region_list (perhaps indirectly). If the argument is NULL, then this
 * routine returns safely without doing anything.
 *
 * Index: segmentation
 * -----------------------------------------------------------------------------
*/

void free_region_list(KJB_region_list* rlp)
{

    if (rlp != NULL)
    {
        if (rlp->regions != NULL)
        {
#ifdef CHECK_VECTOR_INITIALZATION_ON_FREE
            /*
             * We must have a valid pointer to check the initialization,
             * otherwise problems such as double free can look like unitialized
             * memory.
            */
            if (kjb_debug_level >= 10)
            {
                check_initialization(rlp->regions, rlp->num_regions, sizeof(KJB_region));
            }
#endif
            kjb_free(rlp->regions);
        }
    }

    kjb_free(rlp);

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           label_four_connected_regions
 *
 * Binary connected component labeling using 4-connectivity definition.
 *
 *    This routine does connected component labeling using 4-connectivity
 *    definition. It employs a custom data structure (KJB_region) to achieve fast
 *    connected component labeling by doing only one pass through the image. The
 *    custom data structure is designed so that all the information required to
 *    uniquely label the 4-connected regions in the image is available after a
 *    single raster scan.
 *
 *    The input binary image contained in in_mp is assumed to be an Int_matrix with
 *    foreground pixels represented by non-zero integers. All the foreground
 *    connected components are labeled by this routine.
 *
 *    The argument region_list_rlpp is a pointer to a pointer to custom data 
 *    type named KJB_region_list. The data structure KJB_region_list encloses an 
 *    array of elements of type KJB_region. If it is not NULL, then it is resized 
 *    by the routine to contain (ceil(num_rows*num_cols/2) + 1) elements. If it is 
 *    NULL, then a local KJB_region_list is allocated and deallocated when done. 
 *    This is to give the user the option of passing in an appropriately pre-allocated 
 *    KJB_region_list and cutting down on the time spent in memory allocation within the 
 *    routine. This might be helpful in cases where the routine is called repeatedly 
 *    from a higher level routine using different input images of the same size
 *    but with the same pre-allocated KJB_region_list to re-use memory.
 *    The allocation size is related to the fact that the maximum number of
 *    4-connected components in an image is ceil(num_rows*num_cols/2) plus one
 *    for the background component.
 *
 *    segmentation_ptr_ptr is used to store information about individual
 *    connected components in the image. It is a pointer to a pointer to a 
 *    Segmentation_t3 data structure. If it is not NULL, then an input image,
 *    which is the source for binary image in_mp, must be passed to this routine
 *    through the in_ip argument. in_ip is used in computing a number of useful
 *    properties for each component. Otherwise the routine returns an ERROR with an 
 *    appropriate error message being set. If segmentation_ptr_ptr is NULL, then 
 *    no information about individual connected components is computed to store into 
 *    that data structure.
 *
 *    The labeled image with a unique positive integer label for each foreground
 *    pixel is returned in out_mpp. It is allocated or resized to the same size
 *    as the input image following the semantics of the KJB library. If
 *    merge_eqvlnt_labels is set to 1, then a unique label is assigned to all
 *    the pixels of a connected component and the unique labels are consecutive
 *    positive integers (1, 2, ....) for the different components. Otherwise a
 *    unique label is not guaranteed for all the pixels of a connected
 *    component. However the distinct labels are equivalent and can be resolved
 *    using the (*region_list_rlpp) array, when region_list_rlpp is not NULL.
 *    
 *    The output parameter (*num_regions_intp) stores the total number of
 *    connected components in the image. max_label_intp points to an integer
 *    that is the maximum label value among all pixels in the output image before
 *    the equivalent labels are resolved.
 *
 *    The fast labeling algorithm is an extension of the region coloring
 *    algorithm proposed by Ballard and Brown:
 *    D.H. Ballard and C.M. Brown, "Computer Vision", Prentice Hall, NJ, 1982.
 *    The work in the following paper served as a starting point for thoughts on
 *    ways to extend the basic region coloring algorithm:
 *    A. Amir, L. Zimet, A. Sangiovanni-Vincentelli,and S. Kao, "An embedded
 *    system for an eye-detection sensor", CVIU, 98(1):104-123, April 2005.
 *    
 * Warning:
 *    This routine results in memory leaks when fails to execute completely.
 *
 * Returns:
 *    On error this routine returns ERROR.
 *    On success it returns NO_ERROR.
 *
 * Index: segmentation
 * -----------------------------------------------------------------------------
*/

int label_four_connected_regions(Int_matrix**      out_mpp,
                                 KJB_region_list** region_list_rlpp,
                                 Segmentation_t3** segmentation_ptr_ptr,
                                 int*              num_regions_intp,
                                 int*              max_label_intp,
                                 const KJB_image*  in_ip,
                                 const Int_matrix* in_mp,
                                 const int         merge_eqvlnt_labels)
{
        int num_rows = in_mp->num_rows;
        int num_cols = in_mp->num_cols;

        int new_label = 1;

        int total_num_regions = 0;
        int max_num_regions  = 1 + ((num_rows*num_cols + 1) / 2); 

        int i, j;

        Int_matrix*      out_mp          = NULL;
        KJB_region_list* region_list_rlp = NULL;

        int top_pixel_value, left_pixel_value;
        int top_pixel_label, left_pixel_label;

        Vector* serial_labels_vp = NULL;

        int min_region_label, max_region_label;
        int next_region_label, new_max_region_label;
        KJB_region* next_region_ptr = NULL;

        int root_label, new_root_label, junction_label;
        int region_label;

        ERE(get_zero_int_matrix(out_mpp, num_rows, num_cols));
        out_mp = *out_mpp;

        /*
         * Initialize the regions_list array.

         * !!!!WARNING!!!! It is assumed that the regions_list array is at least
         *                 of size ((num_rows*num_cols)/2 + 1).
         *                 Approximately, this is the maximum number of regions
         *                 in an image assuming 4-connectivity.
         *                 If this is not guaranteed, then the code will run
         *                 into trouble.
         * !!!!!!TBD!!!!!! Need a better approach.  
        */
        if (region_list_rlpp == NULL)
        {
            ERE(get_target_region_list(&region_list_rlp, max_num_regions)); 
        }
        else
        {
            ERE(get_target_region_list(region_list_rlpp, max_num_regions));
            region_list_rlp = *region_list_rlpp;
        }
        
        for (i = 0; i < max_num_regions; i++)
        {
            region_list_rlp->regions[i].label           = i;
            region_list_rlp->regions[i].root_label      = i;
            region_list_rlp->regions[i].moment_00       = 0;
            region_list_rlp->regions[i].moment_10       = 0;
            region_list_rlp->regions[i].moment_01       = 0;
            region_list_rlp->regions[i].min_row         = num_rows;
            region_list_rlp->regions[i].max_row         = -1;
            region_list_rlp->regions[i].min_col         = num_cols;
            region_list_rlp->regions[i].max_col         = -1;
            region_list_rlp->regions[i].next_region_ptr = NULL;
        }
        
        (*max_label_intp) = 0;

        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {               
                if (in_mp->elements[i][j] != 0)
                {
                    if (i > 0)
                    {
                        top_pixel_value = in_mp->elements[i-1][j];
                        top_pixel_label = out_mp->elements[i-1][j];
                    }
                    else
                    {
                        top_pixel_value = 0;
                        top_pixel_label = 0;
                    }

                    if (j > 0)
                    {
                        left_pixel_value = in_mp->elements[i][j-1];
                        left_pixel_label = out_mp->elements[i][j-1];
                    }
                    else
                    {
                        left_pixel_value = 0;
                        left_pixel_label = 0;
                    }

                    if ( (top_pixel_value == 0) && (left_pixel_value == 0) )
                    {
                        out_mp->elements[i][j] = new_label;

                        region_list_rlp->regions[new_label].moment_00 = region_list_rlp->regions[new_label].moment_00 + 1;
                        region_list_rlp->regions[new_label].moment_10 = region_list_rlp->regions[new_label].moment_10 + j;
                        region_list_rlp->regions[new_label].moment_01 = region_list_rlp->regions[new_label].moment_01 + i;

                        if (i < region_list_rlp->regions[new_label].min_row)
                        {
                            region_list_rlp->regions[new_label].min_row = i;
                        }

                        if (i > region_list_rlp->regions[new_label].max_row)
                        {
                            region_list_rlp->regions[new_label].max_row = i;
                        }

                        if (j < region_list_rlp->regions[new_label].min_col)
                        {
                            region_list_rlp->regions[new_label].min_col = j;
                        }

                        if (j > region_list_rlp->regions[new_label].max_col)
                        {
                            region_list_rlp->regions[new_label].max_col = j;
                        }

                        new_label         = new_label + 1;
                        total_num_regions = total_num_regions + 1;
                    }
                    else if ( (top_pixel_value != 0) && (left_pixel_value == 0) )
                    {
                        out_mp->elements[i][j] = top_pixel_label;

                        region_list_rlp->regions[top_pixel_label].moment_00 = region_list_rlp->regions[top_pixel_label].moment_00 + 1;
                        region_list_rlp->regions[top_pixel_label].moment_10 = region_list_rlp->regions[top_pixel_label].moment_10 + j;
                        region_list_rlp->regions[top_pixel_label].moment_01 = region_list_rlp->regions[top_pixel_label].moment_01 + i;

                        if (i < region_list_rlp->regions[top_pixel_label].min_row)
                        {
                            region_list_rlp->regions[top_pixel_label].min_row = i;
                        }

                        if (i > region_list_rlp->regions[top_pixel_label].max_row)
                        {
                            region_list_rlp->regions[top_pixel_label].max_row = i;
                        }

                        if (j < region_list_rlp->regions[top_pixel_label].min_col)
                        {
                            region_list_rlp->regions[top_pixel_label].min_col = j;
                        }

                        if (j > region_list_rlp->regions[top_pixel_label].max_col)
                        {
                            region_list_rlp->regions[top_pixel_label].max_col = j;
                        }
                    }
                    else if ( (top_pixel_value == 0) && (left_pixel_value != 0) )
                    {
                        out_mp->elements[i][j] = left_pixel_label;

                        region_list_rlp->regions[left_pixel_label].moment_00 = region_list_rlp->regions[left_pixel_label].moment_00 + 1;
                        region_list_rlp->regions[left_pixel_label].moment_10 = region_list_rlp->regions[left_pixel_label].moment_10 + j;
                        region_list_rlp->regions[left_pixel_label].moment_01 = region_list_rlp->regions[left_pixel_label].moment_01 + i;

                        if (i < region_list_rlp->regions[left_pixel_label].min_row)
                        {
                            region_list_rlp->regions[left_pixel_label].min_row = i;
                        }

                        if (i > region_list_rlp->regions[left_pixel_label].max_row)
                        {
                            region_list_rlp->regions[left_pixel_label].max_row = i;
                        }

                        if (j < region_list_rlp->regions[left_pixel_label].min_col)
                        {
                            region_list_rlp->regions[left_pixel_label].min_col = j;
                        }

                        if (j > region_list_rlp->regions[left_pixel_label].max_col)
                        {
                            region_list_rlp->regions[left_pixel_label].max_col = j;
                        }
                    }
                    else /* if ( (top_pixel_value != 0) && (left_pixel_value != 0) ) */
                    {
                        if (left_pixel_label == top_pixel_label)
                        {
                            out_mp->elements[i][j] = left_pixel_label;

                            region_list_rlp->regions[left_pixel_label].moment_00 = region_list_rlp->regions[left_pixel_label].moment_00 + 1;
                            region_list_rlp->regions[left_pixel_label].moment_10 = region_list_rlp->regions[left_pixel_label].moment_10 + j;
                            region_list_rlp->regions[left_pixel_label].moment_01 = region_list_rlp->regions[left_pixel_label].moment_01 + i;

                            if (i < region_list_rlp->regions[left_pixel_label].min_row)
                            {
                                region_list_rlp->regions[left_pixel_label].min_row = i;
                            }

                            if (i > region_list_rlp->regions[left_pixel_label].max_row)
                            {
                                region_list_rlp->regions[left_pixel_label].max_row = i;
                            }

                            if (j < region_list_rlp->regions[left_pixel_label].min_col)
                            {
                                region_list_rlp->regions[left_pixel_label].min_col = j;
                            }

                            if (j > region_list_rlp->regions[left_pixel_label].max_col)
                            {
                                region_list_rlp->regions[left_pixel_label].max_col = j;
                            }
                        }
                        else /* if (left_pixel_label != top_pixel_label) */
                        {
                            if (left_pixel_label < top_pixel_label)
                            {
                                min_region_label = left_pixel_label;
                                max_region_label = top_pixel_label;
                            }
                            else /* if (left_pixel_label > top_pixel_label) */
                            {
                                min_region_label = top_pixel_label;
                                max_region_label = left_pixel_label;
                            }

                            /*
                             * Label this pixel with the minimum among the neighboring labels.
                            */
                            out_mp->elements[i][j] = min_region_label;

                            region_list_rlp->regions[min_region_label].moment_00 = region_list_rlp->regions[min_region_label].moment_00 + 1;
                            region_list_rlp->regions[min_region_label].moment_10 = region_list_rlp->regions[min_region_label].moment_10 + j;
                            region_list_rlp->regions[min_region_label].moment_01 = region_list_rlp->regions[min_region_label].moment_01 + i;

                            if (i < region_list_rlp->regions[min_region_label].min_row)
                            {
                                region_list_rlp->regions[min_region_label].min_row = i;
                            }

                            if (i > region_list_rlp->regions[min_region_label].max_row)
                            {
                                region_list_rlp->regions[min_region_label].max_row = i;
                            }

                            if (j < region_list_rlp->regions[min_region_label].min_col)
                            {
                                region_list_rlp->regions[min_region_label].min_col = j;
                            }

                            if (j > region_list_rlp->regions[min_region_label].max_col)
                            {
                                region_list_rlp->regions[min_region_label].max_col = j;
                            }

                            next_region_label = min_region_label;
                            next_region_ptr   = region_list_rlp->regions[min_region_label].next_region_ptr;

                            while (next_region_ptr != NULL)
                            {
                                if ((*next_region_ptr).label < max_region_label)
                                {
                                    next_region_label = (*next_region_ptr).label;
                                    next_region_ptr   = region_list_rlp->regions[next_region_label].next_region_ptr; 
                                }
                                else if ((*next_region_ptr).label > max_region_label)
                                {
                                    new_max_region_label = (*next_region_ptr).label;

                                    region_list_rlp->regions[next_region_label].next_region_ptr = &region_list_rlp->regions[max_region_label];

                                    next_region_label = max_region_label;
                                    next_region_ptr   = region_list_rlp->regions[max_region_label].next_region_ptr;

                                    max_region_label = new_max_region_label;
                                }
                                else /* if ((*next_region_ptr).label == max_region_label) */
                                {
                                    break;
                                }
                            }
                            
                            region_list_rlp->regions[next_region_label].next_region_ptr = &region_list_rlp->regions[max_region_label];

                            /*
                             *  A region has been merged with another region.
                            */
                            total_num_regions = total_num_regions - 1;
                        }
                    }               
                }
                else
                {
                    /*
                     *  Background region.
                    */
                    region_list_rlp->regions[0].moment_00 = region_list_rlp->regions[0].moment_00 + 1;
                    region_list_rlp->regions[0].moment_10 = region_list_rlp->regions[0].moment_10 + j;
                    region_list_rlp->regions[0].moment_01 = region_list_rlp->regions[0].moment_01 + i;

                    if (i < region_list_rlp->regions[0].min_row)
                    {
                        region_list_rlp->regions[0].min_row = i;
                    }

                    if (i > region_list_rlp->regions[0].max_row)
                    {
                        region_list_rlp->regions[0].max_row = i;
                    }

                    if (j < region_list_rlp->regions[0].min_col)
                    {
                        region_list_rlp->regions[0].min_col = j;
                    }

                    if (j > region_list_rlp->regions[0].max_col)
                    {
                        region_list_rlp->regions[0].max_col = j;
                    }
                }
            }
        }

        (*max_label_intp) = new_label - 1;

        (*num_regions_intp) = 0;

        for (i = 1; i <= (*max_label_intp); i++)
        {
            if (region_list_rlp->regions[i].label == region_list_rlp->regions[i].root_label)
            {
                root_label          = region_list_rlp->regions[i].label;
                (*num_regions_intp) = (*num_regions_intp) + 1;

                next_region_label = root_label;
                next_region_ptr   = region_list_rlp->regions[root_label].next_region_ptr;

                while (next_region_ptr != NULL)
                {
                    /*
                     * While stepping through the regions_list array, check if any region has 
                     * a previously assigned root label:
                     * If not, then the root label of all the regions along this path will 
                     * be set equal to the label of the starting region of this path.
                     * Otherwise the root label of all the regions along this path should 
                     * be forced to have the previously assigned root label. 
                    */
                    if ( (*next_region_ptr).label == (*next_region_ptr).root_label )
                    {
                        next_region_label = (*next_region_ptr).label;
                        region_list_rlp->regions[next_region_label].root_label = root_label;

                        region_list_rlp->regions[root_label].moment_00 = region_list_rlp->regions[root_label].moment_00 + region_list_rlp->regions[next_region_label].moment_00;
                        region_list_rlp->regions[root_label].moment_10 = region_list_rlp->regions[root_label].moment_10 + region_list_rlp->regions[next_region_label].moment_10;
                        region_list_rlp->regions[root_label].moment_01 = region_list_rlp->regions[root_label].moment_01 + region_list_rlp->regions[next_region_label].moment_01;

                        if (region_list_rlp->regions[next_region_label].min_row < region_list_rlp->regions[root_label].min_row)
                        {
                            region_list_rlp->regions[root_label].min_row = region_list_rlp->regions[next_region_label].min_row;
                        }

                        if (region_list_rlp->regions[next_region_label].max_row > region_list_rlp->regions[root_label].max_row)
                        {
                            region_list_rlp->regions[root_label].max_row = region_list_rlp->regions[next_region_label].max_row;
                        }

                        if (region_list_rlp->regions[next_region_label].min_col < region_list_rlp->regions[root_label].min_col)
                        {
                            region_list_rlp->regions[root_label].min_col = region_list_rlp->regions[next_region_label].min_col;
                        }

                        if (region_list_rlp->regions[next_region_label].max_col > region_list_rlp->regions[root_label].max_col)
                        {
                            region_list_rlp->regions[root_label].max_col = region_list_rlp->regions[next_region_label].max_col;
                        }

                        next_region_ptr = region_list_rlp->regions[next_region_label].next_region_ptr;
                    }
                    else
                    {
                        (*num_regions_intp) = (*num_regions_intp) - 1;

                        junction_label    = (*next_region_ptr).label;
                        new_root_label    = (*next_region_ptr).root_label;

                        next_region_ptr = &region_list_rlp->regions[root_label];
                    
                        while ((*next_region_ptr).label < junction_label)
                        {
                            next_region_label = (*next_region_ptr).label;
                            region_list_rlp->regions[next_region_label].root_label = new_root_label;

                            region_list_rlp->regions[new_root_label].moment_00 = region_list_rlp->regions[new_root_label].moment_00 + region_list_rlp->regions[next_region_label].moment_00;
                            region_list_rlp->regions[new_root_label].moment_10 = region_list_rlp->regions[new_root_label].moment_10 + region_list_rlp->regions[next_region_label].moment_10;
                            region_list_rlp->regions[new_root_label].moment_01 = region_list_rlp->regions[new_root_label].moment_01 + region_list_rlp->regions[next_region_label].moment_01;

                            if (region_list_rlp->regions[next_region_label].min_row < region_list_rlp->regions[new_root_label].min_row)
                            {
                                region_list_rlp->regions[new_root_label].min_row = region_list_rlp->regions[next_region_label].min_row;
                            }

                            if (region_list_rlp->regions[next_region_label].max_row > region_list_rlp->regions[new_root_label].max_row)
                            {
                                region_list_rlp->regions[new_root_label].max_row = region_list_rlp->regions[next_region_label].max_row;
                            }

                            if (region_list_rlp->regions[next_region_label].min_col < region_list_rlp->regions[new_root_label].min_col)
                            {
                                region_list_rlp->regions[new_root_label].min_col = region_list_rlp->regions[next_region_label].min_col;
                            }

                            if (region_list_rlp->regions[next_region_label].max_col > region_list_rlp->regions[new_root_label].max_col)
                            {
                                region_list_rlp->regions[new_root_label].max_col = region_list_rlp->regions[next_region_label].max_col;
                            }

                            next_region_ptr = region_list_rlp->regions[next_region_label].next_region_ptr;
                        }

                        break;
                    }
                }
            }
        }

        /*
         * Do a second pass through the region labels image.
         * Using the regions_list array, re-label each pixel in the
         * labeled image with the root label of its region.
        */

        if ( (merge_eqvlnt_labels == 1) || (segmentation_ptr_ptr != NULL) )
        {
            ERE(get_zero_vector(&serial_labels_vp, (*max_label_intp)+1));

            new_label = 1;

            for (i = 1; i <= (*max_label_intp); i++)
            {
                if (region_list_rlp->regions[i].label == region_list_rlp->regions[i].root_label)
                {
                    root_label                             = region_list_rlp->regions[i].root_label;    
                    serial_labels_vp->elements[root_label] = new_label;
                    new_label                              = new_label + 1;
                }
            }

            for (i = 0; i < num_rows; i++)
            {
                for (j = 0; j < num_cols; j++)
                {
                    region_label = out_mp->elements[i][j];
                    root_label   = region_list_rlp->regions[region_label].root_label;

                    /* Kobus. Check that this is what you mean. */
                    out_mp->elements[i][j] = kjb_rint(serial_labels_vp->elements[root_label]);
                }
            }
        }

        if (segmentation_ptr_ptr != NULL)
        {
            if (in_ip == NULL)
            {
                set_error("Input image is NULL");
                return ERROR;
            }

            if (    (in_ip->num_rows != in_mp->num_rows)
                 || (in_ip->num_cols != in_mp->num_cols) )
            {
                set_error("Input image is not the same size as the region map");
                return ERROR;
            }

            ERE(t3_resegment_image(in_ip, out_mp, segmentation_ptr_ptr));
        }

        if (region_list_rlpp == NULL)
        {
            free_region_list(region_list_rlp);
        }

        free_vector(serial_labels_vp);

        return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           label_eight_connected_regions
 *
 * Binary connected component labeling using 8-connectivity definition.
 *
 *    This routine does connected component labeling using 8-connectivity
 *    definition. It employs a custom data structure (KJB_region) to achieve fast
 *    connected component labeling by doing only one pass through the image. The
 *    custom data structure is designed so that all the information required to
 *    uniquely label the 8-connected regions in the image is available after a
 *    single raster scan.
 *
 *    The input binary image contained in in_mp is assumed to be an Int_matrix with
 *    foreground pixels represented by non-zero integers. All the foreground
 *    connected components are labeled by this routine.
 *
 *    The argument region_list_rlpp is a pointer to a pointer to custom data 
 *    type named KJB_region_list. The data structure KJB_region_list encloses an 
 *    array of elements of type KJB_region. If it is not NULL, then it is resized 
 *    by the routine to contain (ceil(num_rows*num_cols/4) + 1) elements. If it is 
 *    NULL, then a local KJB_region_list is allocated and deallocated when done. 
 *    This is to give the user the option of passing in an appropriately pre-allocated 
 *    KJB_region_list and cutting down on the time spent in memory allocation within the 
 *    routine. This might be helpful in cases where the routine is called repeatedly 
 *    from a higher level routine using different input images of the same size
 *    but with the same pre-allocated KJB_region_list to re-use memory.
 *    The allocation size is related to the fact that the maximum number of
 *    8-connected components in an image is ceil(num_rows*num_cols/4) plus one
 *    for the background component.
 *
 *    segmentation_ptr_ptr is used to store information about individual
 *    connected components in the image. It is a pointer to a pointer to a 
 *    Segmentation_t3 data structure. If it is not NULL, then an input image,
 *    which is the source for binary image in_mp, must be passed to this routine
 *    through the in_ip argument. in_ip is used in computing a number of useful
 *    properties for each component. Otherwise the routine returns an ERROR with an 
 *    appropriate error message being set. If segmentation_ptr_ptr is NULL, then 
 *    no information about individual connected components is computed to store into 
 *    that data structure.
 *
 *    The labeled image with a unique positive integer label for each foreground
 *    pixel is returned in out_mpp. It is allocated or resized to the same size
 *    as the input image following the semantics of the KJB library. If
 *    merge_eqvlnt_labels is set to 1, then a unique label is assigned to all
 *    the pixels of a connected component and the unique labels are consecutive
 *    positive integers (1, 2, ....) for the different components. Otherwise a
 *    unique label is not guaranteed for all the pixels of a connected
 *    component. However the distinct labels are equivalent and can be resolved
 *    using the (*region_list_rlpp) array, when region_list_rlpp is not NULL.
 *    
 *    The output parameter (*num_regions_intp) stores the total number of
 *    connected components in the image. max_label_intp points to an integer
 *    that is the maximum label value among all pixels in the output image before
 *    the equivalent labels are resolved.
 *
 *    The fast labeling algorithm is an extension of the region coloring
 *    algorithm proposed by Ballard and Brown:
 *    D.H. Ballard and C.M. Brown, "Computer Vision", Prentice Hall, NJ, 1982.
 *    The work in the following paper served as a starting point for thoughts on
 *    ways to extend the basic region coloring algorithm:
 *    A. Amir, L. Zimet, A. Sangiovanni-Vincentelli,and S. Kao, "An embedded
 *    system for an eye-detection sensor", CVIU, 98(1):104-123, April 2005.
 *    
 * Warning:
 *    This routine results in memory leaks when fails to execute completely.
 *
 * Returns:
 *    On error this routine returns ERROR.
 *    On success it returns NO_ERROR.
 *
 * Index: segmentation
 * -----------------------------------------------------------------------------
*/

int label_eight_connected_regions(Int_matrix**      out_mpp,
                                  KJB_region_list** region_list_rlpp,
                                  Segmentation_t3** segmentation_ptr_ptr,
                                  int*              num_regions_intp,
                                  int*              max_label_intp,
                                  const KJB_image*  in_ip,
                                  const Int_matrix* in_mp,
                                  const int         merge_eqvlnt_labels)
{
        int num_rows = in_mp->num_rows;
        int num_cols = in_mp->num_cols;

        int new_label = 1;

        int total_num_regions = 0;
        int max_num_regions   = 1 + ((num_rows*num_cols + 1) / 4); 

        int i, j, k, l, m, n;

        Int_matrix*      out_mp          = NULL;
        KJB_region_list* region_list_rlp = NULL;

        /*
        int top_pixel_value, left_pixel_value;
        int top_pixel_label, left_pixel_label;
        */
        Int_vector* neighbor_labels_vp = NULL;
        int         max_num_neighbors  = 4;
        int         num_neighbors      = 0; 

        Vector* serial_labels_vp = NULL;

        int min_region_label, max_region_label;
        int next_region_label, new_max_region_label;
        KJB_region* next_region_ptr = NULL;

        int root_label, new_root_label, junction_label;
        int region_label;
        int neighbor_label;

        ERE(get_zero_int_matrix(out_mpp, num_rows, num_cols));
        out_mp = *out_mpp;

        /*
         * Initialize the regions_list array.

         * !!!!WARNING!!!! It is assumed that the regions_list array is at least
         *                 of size ((num_rows*num_cols)/2 + 1).
         *                 Approximately, this is the maximum number of regions
         *                 in an image assuming 4-connectivity.
         *                 If this is not guaranteed, then the code will run
         *                 into trouble.
         * !!!!!!TBD!!!!!! Need a better approach.  
        */
        if (region_list_rlpp == NULL)
        {
            ERE(get_target_region_list(&region_list_rlp, max_num_regions)); 
        }
        else
        {
            ERE(get_target_region_list(region_list_rlpp, max_num_regions));
            region_list_rlp = *region_list_rlpp;
        }
        
        for (i = 0; i < max_num_regions; i++)
        {
            region_list_rlp->regions[i].label           = i;
            region_list_rlp->regions[i].root_label      = i;
            region_list_rlp->regions[i].moment_00       = 0;
            region_list_rlp->regions[i].moment_10       = 0;
            region_list_rlp->regions[i].moment_01       = 0;
            region_list_rlp->regions[i].min_row         = num_rows;
            region_list_rlp->regions[i].max_row         = -1;
            region_list_rlp->regions[i].min_col         = num_cols;
            region_list_rlp->regions[i].max_col         = -1;
            region_list_rlp->regions[i].next_region_ptr = NULL;
        }
        
        (*max_label_intp) = 0;

        for (i = 0; i < num_rows; i++)
        {
            for (j = 0; j < num_cols; j++)
            {               
                if (in_mp->elements[i][j] != 0)
                {
                    /* Collect neighboring pixel labels. */ 
                    num_neighbors = 0;
                    ERE(get_initialized_int_vector(&neighbor_labels_vp, max_num_neighbors, max_num_regions));

                    if ( (i > 0) || (j > 0) )                    
                    {
                        /* Previous row neighbors. */
                        k = i-1;
                        for (l = (j-1); l <= (j+1); l++)
                        {
                            if ( (k >= 0) && (k < num_rows) && (l >= 0) && (l < num_cols) )
                            {
                                if (in_mp->elements[k][l] != 0)
                                {
                                    neighbor_label = out_mp->elements[k][l];

                                    for (m = 0; m < max_num_neighbors; m++)
                                    {
                                        if (neighbor_label < neighbor_labels_vp->elements[m])
                                        {
                                            break;
                                        }                                        
                                    }
                                    for (n = (max_num_neighbors-1); n > m ; n--)
                                    {
                                        neighbor_labels_vp->elements[n] = neighbor_labels_vp->elements[n-1];
                                    }
                                    neighbor_labels_vp->elements[n] = neighbor_label; 

                                    num_neighbors = num_neighbors + 1;
                                }
                            }
                        }

                        /* Current row neighbor. */
                        k = i;
                        for (l = (j-1); l < j; l++)
                        {
                            if ( (k >= 0) && (k < num_rows) && (l >= 0) && (l < num_cols) )
                            {
                                if (in_mp->elements[k][l] != 0)
                                {
                                    neighbor_label = out_mp->elements[k][l];

                                    for (m = 0; m < max_num_neighbors; m++)
                                    {
                                        if (neighbor_label < neighbor_labels_vp->elements[m])
                                        {
                                            break;
                                        }                                        
                                    }
                                    for (n = (max_num_neighbors-1); n > m ; n--)
                                    {
                                        neighbor_labels_vp->elements[n] = neighbor_labels_vp->elements[n-1];
                                    }
                                    neighbor_labels_vp->elements[n] = neighbor_label; 

                                    num_neighbors = num_neighbors + 1;
                                }
                            }
                        }
                    }

                    /*
                    dbi(i);
                    dbi(j);
                    dbi(num_neighbors);
                    */

                    if ( num_neighbors == 0 )
                    {
                        out_mp->elements[i][j] = new_label;

                        region_list_rlp->regions[new_label].moment_00 = region_list_rlp->regions[new_label].moment_00 + 1;
                        region_list_rlp->regions[new_label].moment_10 = region_list_rlp->regions[new_label].moment_10 + j;
                        region_list_rlp->regions[new_label].moment_01 = region_list_rlp->regions[new_label].moment_01 + i;

                        if (i < region_list_rlp->regions[new_label].min_row)
                        {
                            region_list_rlp->regions[new_label].min_row = i;
                        }

                        if (i > region_list_rlp->regions[new_label].max_row)
                        {
                            region_list_rlp->regions[new_label].max_row = i;
                        }

                        if (j < region_list_rlp->regions[new_label].min_col)
                        {
                            region_list_rlp->regions[new_label].min_col = j;
                        }

                        if (j > region_list_rlp->regions[new_label].max_col)
                        {
                            region_list_rlp->regions[new_label].max_col = j;
                        }

                        new_label         = new_label + 1;
                        total_num_regions = total_num_regions + 1;
                    }
                    else
                    {
                        min_region_label = neighbor_labels_vp->elements[0];

                        out_mp->elements[i][j] = min_region_label;

                        region_list_rlp->regions[min_region_label].moment_00 = region_list_rlp->regions[min_region_label].moment_00 + 1;
                        region_list_rlp->regions[min_region_label].moment_10 = region_list_rlp->regions[min_region_label].moment_10 + j;
                        region_list_rlp->regions[min_region_label].moment_01 = region_list_rlp->regions[min_region_label].moment_01 + i;

                        if (i < region_list_rlp->regions[min_region_label].min_row)
                        {
                            region_list_rlp->regions[min_region_label].min_row = i;
                        }

                        if (i > region_list_rlp->regions[min_region_label].max_row)
                        {
                            region_list_rlp->regions[min_region_label].max_row = i;
                        }

                        if (j < region_list_rlp->regions[min_region_label].min_col)
                        {
                            region_list_rlp->regions[min_region_label].min_col = j;
                        }

                        if (j > region_list_rlp->regions[min_region_label].max_col)
                        {
                            region_list_rlp->regions[min_region_label].max_col = j;
                        }

                        /* Pairwise label merging. */
                        for (m = 0; m < (num_neighbors-1); m++)
                        {
                            min_region_label = neighbor_labels_vp->elements[m];
                            max_region_label = neighbor_labels_vp->elements[m+1];

                            if (min_region_label != max_region_label)
                            {
                                next_region_label = min_region_label;
                                next_region_ptr   = region_list_rlp->regions[min_region_label].next_region_ptr;

                                while (next_region_ptr != NULL)
                                {
                                    if ((*next_region_ptr).label < max_region_label)
                                    {
                                        next_region_label = (*next_region_ptr).label;
                                        next_region_ptr   = region_list_rlp->regions[next_region_label].next_region_ptr; 
                                    }
                                    else if ((*next_region_ptr).label > max_region_label)
                                    {
                                        new_max_region_label = (*next_region_ptr).label;

                                        region_list_rlp->regions[next_region_label].next_region_ptr = &region_list_rlp->regions[max_region_label];

                                        next_region_label = max_region_label;
                                        next_region_ptr   = region_list_rlp->regions[max_region_label].next_region_ptr;

                                        max_region_label = new_max_region_label;
                                    }
                                    else /* if ((*next_region_ptr).label == max_region_label) */
                                    {
                                        break;
                                    }
                                }

                                region_list_rlp->regions[next_region_label].next_region_ptr = &region_list_rlp->regions[max_region_label];
                            }

                            /*
                             *  A region has been merged with another region.
                            */
                            total_num_regions = total_num_regions - 1;
                        }
                    }               
                }
                else
                {
                    /*
                     *  Background region.
                    */
                    region_list_rlp->regions[0].moment_00 = region_list_rlp->regions[0].moment_00 + 1;
                    region_list_rlp->regions[0].moment_10 = region_list_rlp->regions[0].moment_10 + j;
                    region_list_rlp->regions[0].moment_01 = region_list_rlp->regions[0].moment_01 + i;

                    if (i < region_list_rlp->regions[0].min_row)
                    {
                        region_list_rlp->regions[0].min_row = i;
                    }

                    if (i > region_list_rlp->regions[0].max_row)
                    {
                        region_list_rlp->regions[0].max_row = i;
                    }

                    if (j < region_list_rlp->regions[0].min_col)
                    {
                        region_list_rlp->regions[0].min_col = j;
                    }

                    if (j > region_list_rlp->regions[0].max_col)
                    {
                        region_list_rlp->regions[0].max_col = j;
                    }
                }
            }
        }

        (*max_label_intp) = new_label - 1;

        (*num_regions_intp) = 0;

        for (i = 1; i <= (*max_label_intp); i++)
        {
            if (region_list_rlp->regions[i].label == region_list_rlp->regions[i].root_label)
            {
                root_label          = region_list_rlp->regions[i].label;
                (*num_regions_intp) = (*num_regions_intp) + 1;

                next_region_label = root_label;
                next_region_ptr   = region_list_rlp->regions[root_label].next_region_ptr;

                while (next_region_ptr != NULL)
                {
                    /*
                     * While stepping through the regions_list array, check if any region has 
                     * a previously assigned root label:
                     * If not, then the root label of all the regions along this path will 
                     * be set equal to the label of the starting region of this path.
                     * Otherwise the root label of all the regions along this path should 
                     * be forced to have the previously assigned root label. 
                    */
                    if ( (*next_region_ptr).label == (*next_region_ptr).root_label )
                    {
                        next_region_label = (*next_region_ptr).label;
                        region_list_rlp->regions[next_region_label].root_label = root_label;

                        region_list_rlp->regions[root_label].moment_00 = region_list_rlp->regions[root_label].moment_00 + region_list_rlp->regions[next_region_label].moment_00;
                        region_list_rlp->regions[root_label].moment_10 = region_list_rlp->regions[root_label].moment_10 + region_list_rlp->regions[next_region_label].moment_10;
                        region_list_rlp->regions[root_label].moment_01 = region_list_rlp->regions[root_label].moment_01 + region_list_rlp->regions[next_region_label].moment_01;

                        if (region_list_rlp->regions[next_region_label].min_row < region_list_rlp->regions[root_label].min_row)
                        {
                            region_list_rlp->regions[root_label].min_row = region_list_rlp->regions[next_region_label].min_row;
                        }

                        if (region_list_rlp->regions[next_region_label].max_row > region_list_rlp->regions[root_label].max_row)
                        {
                            region_list_rlp->regions[root_label].max_row = region_list_rlp->regions[next_region_label].max_row;
                        }

                        if (region_list_rlp->regions[next_region_label].min_col < region_list_rlp->regions[root_label].min_col)
                        {
                            region_list_rlp->regions[root_label].min_col = region_list_rlp->regions[next_region_label].min_col;
                        }

                        if (region_list_rlp->regions[next_region_label].max_col > region_list_rlp->regions[root_label].max_col)
                        {
                            region_list_rlp->regions[root_label].max_col = region_list_rlp->regions[next_region_label].max_col;
                        }

                        next_region_ptr = region_list_rlp->regions[next_region_label].next_region_ptr;
                    }
                    else
                    {
                        (*num_regions_intp) = (*num_regions_intp) - 1;

                        junction_label    = (*next_region_ptr).label;
                        new_root_label    = (*next_region_ptr).root_label;

                        next_region_ptr = &region_list_rlp->regions[root_label];
                    
                        while ((*next_region_ptr).label < junction_label)
                        {
                            next_region_label = (*next_region_ptr).label;
                            region_list_rlp->regions[next_region_label].root_label = new_root_label;

                            region_list_rlp->regions[new_root_label].moment_00 = region_list_rlp->regions[new_root_label].moment_00 + region_list_rlp->regions[next_region_label].moment_00;
                            region_list_rlp->regions[new_root_label].moment_10 = region_list_rlp->regions[new_root_label].moment_10 + region_list_rlp->regions[next_region_label].moment_10;
                            region_list_rlp->regions[new_root_label].moment_01 = region_list_rlp->regions[new_root_label].moment_01 + region_list_rlp->regions[next_region_label].moment_01;

                            if (region_list_rlp->regions[next_region_label].min_row < region_list_rlp->regions[new_root_label].min_row)
                            {
                                region_list_rlp->regions[new_root_label].min_row = region_list_rlp->regions[next_region_label].min_row;
                            }

                            if (region_list_rlp->regions[next_region_label].max_row > region_list_rlp->regions[new_root_label].max_row)
                            {
                                region_list_rlp->regions[new_root_label].max_row = region_list_rlp->regions[next_region_label].max_row;
                            }

                            if (region_list_rlp->regions[next_region_label].min_col < region_list_rlp->regions[new_root_label].min_col)
                            {
                                region_list_rlp->regions[new_root_label].min_col = region_list_rlp->regions[next_region_label].min_col;
                            }

                            if (region_list_rlp->regions[next_region_label].max_col > region_list_rlp->regions[new_root_label].max_col)
                            {
                                region_list_rlp->regions[new_root_label].max_col = region_list_rlp->regions[next_region_label].max_col;
                            }

                            next_region_ptr = region_list_rlp->regions[next_region_label].next_region_ptr;
                        }

                        break;
                    }
                }
            }
        }

        /*
         * Do a second pass through the region labels image.
         * Using the regions_list array, re-label each pixel in the
         * labeled image with the root label of its region.
        */

        if ( (merge_eqvlnt_labels == 1) || (segmentation_ptr_ptr != NULL) )
        {
            ERE(get_zero_vector(&serial_labels_vp, (*max_label_intp)+1));

            new_label = 1;

            for (i = 1; i <= (*max_label_intp); i++)
            {
                if (region_list_rlp->regions[i].label == region_list_rlp->regions[i].root_label)
                {
                    root_label                             = region_list_rlp->regions[i].root_label;    
                    serial_labels_vp->elements[root_label] = new_label;
                    new_label                              = new_label + 1;
                }
            }

            for (i = 0; i < num_rows; i++)
            {
                for (j = 0; j < num_cols; j++)
                {
                    region_label = out_mp->elements[i][j];
                    root_label   = region_list_rlp->regions[region_label].root_label;

                    /* Kobus. Check that this is what you mean. */
                    out_mp->elements[i][j] = kjb_rint(serial_labels_vp->elements[root_label]);
                }
            }
        }

        if (segmentation_ptr_ptr != NULL)
        {
            if (in_ip == NULL)
            {
                set_error("Input image is NULL");
                return ERROR;
            }

            if (    (in_ip->num_rows != in_mp->num_rows)
                 || (in_ip->num_cols != in_mp->num_cols) )
            {
                set_error("Input image is not the same size as the region map");
                return ERROR;
            }

            ERE(t3_resegment_image(in_ip, out_mp, segmentation_ptr_ptr));
        }

        if (region_list_rlpp == NULL)
        {
            free_region_list(region_list_rlp);
        }

        free_int_vector(neighbor_labels_vp);
        free_vector(serial_labels_vp);

        return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

