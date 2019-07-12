
/* $Id: i_transform.c 7578 2010-11-21 19:53:39Z ksimek $ */

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
#include "i/i_gamma.h"
#include "i/i_transform.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */


/* =============================================================================
 *                             ow_invert_image
 *
 *
 * Index: images, image inversion, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int ow_invert_image(KJB_image* ip)
{
    int   i, j, num_rows, num_cols;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            ip->pixels[i][j].r = 255.0 - ip->pixels[i][j].r;
            ip->pixels[i][j].g = 255.0 - ip->pixels[i][j].g;
            ip->pixels[i][j].b = 255.0 - ip->pixels[i][j].b;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             ow_invert_gamma_image
 *
 * Inverts a gamma corrected image in linear space
 *
 * This routine inverts a gamma corrected image assuming that that inversion
 * should occur in linear space. The linear inverted image is re-gamma'd.
 * Currently, default gamma correction is used.
 *
 * So far, this has not given results that are palalable, and this code is
 * currently flagged as UNUSED!
 *
 * Index: images, image inversion, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int ow_invert_gamma_image(KJB_image* ip)
{

    UNTESTED_CODE();


    ERE(ow_invert_image_gamma(ip, NULL));
    ERE(ow_invert_image(ip));
    ERE(ow_gamma_correct_image(ip, NULL));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             rotate_image_right
 *
 * Rotates an image to the right 90 degrees.
 *
 * This routine rotates an input image to the right 90 degrees. The result image
 * is created or resized as needed in conformance with KJB library semantics.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure with an error message being set.
 *
 * Index: images, image rotation, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int rotate_image_right(KJB_image** target_ipp, const KJB_image* ip)
{
    KJB_image* target_ip;
    int   i, j, num_rows, num_cols;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    ERE(get_target_image(target_ipp, ip->num_cols, ip->num_rows));
    target_ip = *target_ipp;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            target_ip->pixels[j][num_rows - 1 - i].r = ip->pixels[i][j].r;
            target_ip->pixels[j][num_rows - 1 - i].g = ip->pixels[i][j].g;
            target_ip->pixels[j][num_rows - 1 - i].b = ip->pixels[i][j].b;
            target_ip->pixels[j][num_rows - 1 - i].extra.invalid = ip->pixels[i][j].extra.invalid;
        }
    }

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             rotate_image_left
 *
 * Rotates an image to the left 90 degrees.
 *
 * This routine rotates an input image to the left 90 degrees. The result image
 * is created or resized as needed in conformance with KJB library semantics.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure with an error message being set.
 *
 * Index: images, image rotation, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int rotate_image_left(KJB_image** target_ipp, const KJB_image* ip)
{
    KJB_image* target_ip;
    int   i, j, num_rows, num_cols;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    ERE(get_target_image(target_ipp, ip->num_cols, ip->num_rows));
    target_ip = *target_ipp;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            target_ip->pixels[num_cols - 1 - j][i].r = ip->pixels[i][j].r;
            target_ip->pixels[num_cols - 1 - j][i].g = ip->pixels[i][j].g;
            target_ip->pixels[num_cols - 1 - j][i].b = ip->pixels[i][j].b;
            target_ip->pixels[num_cols - 1 - j][i].extra.invalid = ip->pixels[i][j].extra.invalid;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       ow_horizontal_flip_image
 *
 * Flips an image across the horizontal axis.
 *
 * This routine flips an image across the horizontal axis.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure with an error message being set.
 *
 * Index: images, image rotation, image transformation
 *
 * Author:
 *     Abin Shahab
 *
 * Documentor:
 *     Kobus Barnard
 *
 * -----------------------------------------------------------------------------
*/

int ow_horizontal_flip_image(KJB_image* ip)
{
    int   i, j, num_rows, num_cols;
    float r, g, b;
    Invalid_pixel invalid;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    TEST_PSO(("\n\nflipping 0 through %d rows\n\n", num_rows));
    for (i=0; i<(num_rows/2); i++)
    {
        for (j=0; j<num_cols; j++)
        {
            r = ip->pixels[num_rows - 1 - i][j].r;
            g = ip->pixels[num_rows - 1 - i][j].g;
            b = ip->pixels[num_rows - 1 - i][j].b;
            invalid = ip->pixels[num_rows - 1 - i][j].extra.invalid;

            ip->pixels[num_rows - 1 - i][j].r = ip->pixels[i][j].r;
            ip->pixels[num_rows - 1 - i][j].g = ip->pixels[i][j].g;
            ip->pixels[num_rows - 1 - i][j].b = ip->pixels[i][j].b;
            ip->pixels[num_rows - 1 - i][j].extra.invalid = ip->pixels[i][j].extra.invalid;

            ip->pixels[i][j].r = r;
            ip->pixels[i][j].g = g;
            ip->pixels[i][j].b = b;
            ip->pixels[i][j].extra.invalid = invalid;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       ow_vertical_flip_image
 *
 * Flips an image across the vertical axis.
 *
 * This routine flips an image across the vertical axis.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure with an error message being set.
 *
 * Index: images, image rotation, image transformation
 *
 * Author:
 *     Abin Shahab
 *
 * Documentor:
 *     Kobus Barnard
 *
 * -----------------------------------------------------------------------------
*/

int ow_vertical_flip_image(KJB_image* ip)
{
    int   i, j, num_rows, num_cols;
    float r, g, b;
    Invalid_pixel invalid;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    TEST_PSO(("\n\nflipping 0 through %d cols\n\n", num_cols));
    for (i=0; i<(num_cols/2); i++)
    {
        for (j=0; j<num_rows; j++)
        {
            r = ip->pixels[j][num_cols - 1 - i].r;
            g = ip->pixels[j][num_cols - 1 - i].g;
            b = ip->pixels[j][num_cols - 1 - i].b;
            invalid = ip->pixels[j][num_cols - 1 - i].extra.invalid;

            ip->pixels[j][num_cols - 1 - i].r = ip->pixels[j][i].r;
            ip->pixels[j][num_cols - 1 - i].g = ip->pixels[j][i].g;
            ip->pixels[j][num_cols - 1 - i].b = ip->pixels[j][i].b;
            ip->pixels[j][num_cols - 1 - i].extra.invalid = ip->pixels[j][i].extra.invalid;

            ip->pixels[j][i].r = r;
            ip->pixels[j][i].g = g;
            ip->pixels[j][i].b = b;
            ip->pixels[j][i].extra.invalid = invalid;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              scale_image_size
 *
 * Scale an image
 *
 * This routine scales (in size) the image pointed to by ip and puts it in 
 * *target_ipp. This routine follows the KJB memory management conventions.
 * This routine calls imagemagick to perform the scaling.
 *
 * Note: This should not be confused with scale_image, which scales the 
 * pixel values (the color) of an image. In fact, scale_image should probably
 * be renamed to something else and this routine should be called scale_image.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure with an error message being set.
 *
 * Index: images, image rotation, image transformation
 *
 * Author:
 *     Ernesto Brau
 *
 * Documentor:
 *     Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int scale_image_size
(
    KJB_image**      target_ipp,
    const KJB_image* ip,
    double           scale_factor
)
{
    char command[1000];
    static const char* convert_programs[] = {"convert +compress -type truecolor -resize", "convert -type truecolor -resize", "convert -resize"};
    int num_convert_programs = sizeof(convert_programs) / sizeof(char*);
    char orig_image_filename[MAX_FILE_NAME_SIZE];
    char new_image_filename[MAX_FILE_NAME_SIZE];
    int result;
    int i;

    if(scale_factor <= 0.0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(target_ipp == NULL || ip == NULL)
    {
        return NO_ERROR;
    }

    if(scale_factor == 1.0)
    {
        kjb_copy_image(target_ipp, ip);
        return NO_ERROR;
    }

    BUFF_GET_TEMP_FILE_NAME(orig_image_filename);
    BUFF_CAT(orig_image_filename, ".tif");
    BUFF_GET_TEMP_FILE_NAME(new_image_filename);
    BUFF_CAT(new_image_filename, ".tif");

    kjb_write_image(ip, orig_image_filename);

    for(i = 0; i < num_convert_programs; i++)
    {
        ERE(kjb_sprintf(command, sizeof(command), "%s %f%% %s %s", convert_programs[i], scale_factor * 100, orig_image_filename, new_image_filename));
        result = system(command);
        if(result == NO_ERROR && get_path_type(new_image_filename) == PATH_IS_REGULAR_FILE)
        {
            ERE(kjb_read_image(target_ipp, new_image_filename));
            remove(orig_image_filename);
            remove(new_image_filename);
            return NO_ERROR;
        }
    }

    remove(orig_image_filename);

    set_error("Error resizing image with all conversion programs.");
    add_error("Tried \"%s\"", convert_programs[0]);
    for(i = 1; i < num_convert_programs; i++)
    {
        cat_error(", \"%s\"", convert_programs[i]);
    }

    cat_error(".");

    return ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

