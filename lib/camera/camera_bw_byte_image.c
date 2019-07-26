
/* $Id: camera_bw_byte_image.c 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) by members of University of Arizona Computer Vision Group     |
|  (the authors) including                                                     |
|        Kobus Barnard.                                                        |
|        Kate Spriggs                                                          |
|        Justin Cappos                                                         |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */


#include "i/i_float.h"
#include "camera/camera_bw_byte_image.h"

#ifdef __cplusplus
extern "C" {
#endif

int get_target_camera_bw_byte_image(Camera_bw_byte_image **target_ipp, int num_rows, int num_cols)
{
    /*Kate - the next code is not checked against the coding requirements*/
    if(target_ipp == NULL)
        return ERROR;
    /*I think we need to free if the structure is not null*/
    if (*target_ipp != NULL)
    {
        kjb_free_camera_bw_byte_image(*target_ipp);      
    }
    if (*target_ipp == NULL)
    {
        *target_ipp = TYPE_MALLOC(Camera_bw_byte_image);
        (*target_ipp)->data = NULL;
        (*target_ipp)->image_time.tv_sec = -1;
        (*target_ipp)->image_time.tv_usec = -1;
        /*(*target_ipp)->camera_id = NULL;*/
        /*  NRN( (*target_ipp) = TYPE_MALLOC(Camera_bw_byte_image));*/

    }

    /*end unchecked coding requirements*/

    if ((num_rows <= 0) || (num_cols <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
    else if ((*target_ipp)->data == NULL)
    {
        get_target_bw_byte_image(&((*target_ipp)->data), num_rows, num_cols);
        if((*target_ipp)->data == NULL)
            return ERROR;
        /*NRE((*target_ipp)->data = create_bw_byte_image(num_rows, num_cols));*/
    }
    else
    {
        /*
        if ((*target_ipp->data)->read_only)
        {
        SET_ARGUMENT_BUG();
        return ERROR;
        }
         */
        if (    (( (*target_ipp)->data)->num_rows != num_rows)
                || (( (*target_ipp)->data)->num_cols != num_cols))
        {
            kjb_free_bw_byte_image( (*target_ipp)->data);
            get_target_bw_byte_image( &((*target_ipp)->data), num_rows, num_cols);
            /*NRE( (*target_ipp)->data = create_bw_byte_image(num_rows, num_cols));*/
        }
    }

    return NO_ERROR;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void kjb_free_camera_bw_byte_image(Camera_bw_byte_image* ip)
{
  kjb_free_bw_byte_image(ip->data);
  kjb_free(ip);
}

#ifdef __cplusplus
}
#endif
