
/* $Id: i_byte.c 5724 2010-03-18 01:29:35Z ksimek $ */

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

#include "i/i_gen.h"     /* Only safe as first include in a ".c" file. */
#include "i/i_byte.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              create_byte_image
 *
 *
 * Index: images, Byte images
 *
 * -----------------------------------------------------------------------------
*/

Byte_image* create_byte_image(int num_rows, int num_cols)
{
    Byte_image* ip;
    int         i;


#ifdef TEST
    /*CONSTCOND*/
    ASSERT(sizeof(Byte_pixel) == 3);
#endif

    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    /* prevent nx0 or 0xn image */
    if(num_rows*num_cols == 0 && num_rows + num_cols != 0)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(ip = TYPE_MALLOC(Byte_image));

    ip->num_rows = num_rows;
    ip->num_cols = num_cols;

    ip->read_only = FALSE;

    if(num_rows == 0 && num_cols == 0)
    {
        ip->pixels = NULL;
    }
    else
    {
        if ((ip->pixels = N_TYPE_MALLOC(Byte_pixel*, num_rows)) == NULL)
        {
            kjb_free(ip);
            return NULL;
        }

        (ip->pixels)[ 0 ] = N_TYPE_MALLOC(Byte_pixel, num_rows*num_cols);

        if ((ip->pixels)[ 0 ] == NULL)
        {
            kjb_free(ip->pixels);
            kjb_free(ip);
            return NULL;
        }
    }

    for (i=1; i<num_rows; i++)
    {
        (ip->pixels)[ i ] = (ip->pixels)[ 0 ] + i*num_cols;
    }

    return ip;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              free_byte_image
 *
 *
 * Index: images, Byte images
 *
 * -----------------------------------------------------------------------------
*/

void free_byte_image(Byte_image* ip)
{

    if (ip != NULL)
    {
        if (ip->pixels != NULL)
        {
            if (ip->read_only)
            {
#ifdef TEST
                set_bug("Attempt to free read only Byte image.");
#endif
                return;
            }

            kjb_free(ip->pixels[ 0 ]);
            kjb_free(ip->pixels);
        }

        kjb_free(ip);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_target_byte_image
 *
 * Gets target Byte image for "building block" routines
 *
 * This routine implements the image creation/over-writing symantics used in
 * the KJB library in the case of Byte images. If *ipp is NULL, then this
 * routine creates the images. If it is not null, and it is the right size,
 * then this routine does nothing. If it is the wrong size, then it is resized.
 *
 * If an actual resize is needed, then a new image of the required size is
 * first created. If the creation is successful, then the old image is free'd.
 * The reason is that if the new allocation fails, a calling application should
 * have use of the old image. The alternate is to free the old image first.
 * This is more memory efficient. A more sophisticated alternative is to free
 * the old image if it can be deterimined that the subsequent allocation will
 * succeed. For example, on systems where it can be guaranteed that free'd
 * memory is not released to the system, this can be done based on image size.
 * More sophisticated methods may also be available. However, any such approach
 * will be system dependent. Although such approaches have merit, it is
 * expected that resizing will occur infrequently enought that it is not worth
 * implementing them. Thus the simplest method with good semantics under most
 * conditions has been used.
 *
 * The sizes (num_rows and num_cols) must both be nonnegative.
 * Conventionally, num_rows and num_cols are both positive;
 * however, it is also acceptable to have num_rows and num_cols both zero.
 * Anything else (such as a mix of positive and zero sizes) is regarded as a
 * bug in the calling program -- set_bug(3) is called and (if it returns) 
 * ERROR is returned.
 *
 * Index: images, Byte images
 *
 * -----------------------------------------------------------------------------
*/

int get_target_byte_image
(
    Byte_image** target_ipp,
    int          num_rows,
    int          num_cols
)
{


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* prevent nx0 or 0xn image */
    if(num_rows*num_cols == 0 && num_rows + num_cols != 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (*target_ipp == NULL)
    {
        NRE(*target_ipp = create_byte_image(num_rows, num_cols));
    }
    else
    {
        if ((*target_ipp)->read_only)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }

        if (    ((*target_ipp)->num_rows != num_rows)
             || ((*target_ipp)->num_cols != num_cols))
        {
            Byte_image* new_ip = create_byte_image(num_rows, num_cols);

            NRE(new_ip);
            free_byte_image(*target_ipp);
            *target_ipp = new_ip;
        }
    }

    return NO_ERROR;
}


#ifdef __cplusplus
}
#endif

