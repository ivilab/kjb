
/* $Id: i_stat.c 4727 2009-11-16 20:53:54Z kobus $ */

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
#include "l/l_sys_io.h"
#include "i/i_seq.h"
#include "i/i_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              get_target_image_sequence
 *
 * Gets a target image sequence
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of image sequences. If *target_ispp is NULL, then this
 * routine creates the image sequence. If it is not NULL, and it is the right
 * size, then this routine does nothing. If it is the wrong size, then it is 
 * resized.
 *
 * The routine free_image_sequence should be used to dispose of the storage once
 * it is no longer needed
 *
 * Returns:
 *     On error, this routine returns NULL, with an error message being set.
 *     On success it returns a pointer to the sequence.
 *
 * Index: images, memory allocation
 *
 * Author: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int get_target_image_sequence(KJB_image_sequence** target_ispp, int length)
{
    KJB_image_sequence* isp;
    int i;

    if(target_ispp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if(*target_ispp != NULL)
    {
        if((*target_ispp)->length == length)
        {
            return NO_ERROR;
        }
    }
    free_image_sequence(*target_ispp);

    NRE(isp = TYPE_MALLOC(KJB_image_sequence));

    if(length > 0)
    {
        NRE(isp->elements = N_TYPE_MALLOC(KJB_image*, length));
        isp->length = length;
    }
    else
    {
        isp->elements = NULL;
        isp->length = 0;
    }

    for(i = 0; i < length; i++)
    {
        isp->elements[i] = NULL;
    }

    *target_ispp = isp;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  read_image_sequence
 *
 * Reads in a target image sequence
 *
 * This routine reads the images of an image sequence from the file names in 
 * filelist and stores them in target_ispp. The resulting image sequence will
 * have filenames->num_words images.
 *
 * The routine free_image_sequence should be used to dispose of the storage once
 * it is no longer needed
 *
 * Returns:
 *     On error, this routine returns ERROR, with an error message being set.
 *     On success it returns NO_ERROR.
 *
 * Index: images, memory allocation
 *
 * Related: Word_list
 *
 * Author: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int read_image_sequence(KJB_image_sequence** target_ispp, const Word_list* filenames)
{
    int i;

    if(filenames == NULL || target_ispp == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_image_sequence(target_ispp, filenames->num_words));

    for(i = 0; i < filenames->num_words; i++)
    {
        if(!is_file(filenames->words[i]))
        {
            set_error("%s not a valid file.", filenames->words[i]);
            return ERROR;
        }
        ERE(kjb_read_image(&(*target_ispp)->elements[i], filenames->words[i]));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              free_image_sequence
 *
 * Free the storace in an image sequence
 *
 * This routine frees the storage in an image sequence
 *
 * Index: images, memory allocation
 *
 * Author: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

void free_image_sequence(KJB_image_sequence* isp)
{
    int i, count;
    KJB_image** ip_pos;

    if(isp == NULL) return;

    ip_pos = isp->elements;
    count = isp->length;

    for(i = 0; i < count; i++)
    {
        kjb_free_image(*ip_pos);
        ip_pos++;
    }

    kjb_free(isp->elements);
    kjb_free(isp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  average_bw_images
 *
 * Averages a set of images
 *
 * This routine averages all the images in images and puts the result in avg_img.
 *
 * The images in images are assumed to be black and white, although the
 * image_to_matrix_routine is used in case it is not.
 *
 * Returns:
 *     On error, this routine returns ERROR, with an error message being set.
 *     On success it returns NO_ERROR.
 *
 * Index: images
 *
 * Related: image_to_matrix
 *
 * Author: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int average_bw_images
(
    KJB_image**               avg_img,
    const KJB_image_sequence* images
)
{
    Matrix* avg_img_mat = NULL;

    ERE(average_bw_images_2(&avg_img_mat, images));
    ERE(matrix_to_bw_image(avg_img_mat, avg_img));

    free_matrix(avg_img_mat);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  std_dev_bw_images
 *
 * Computes the standard deviation of a set of images
 *
 * This routine finds the standard deviation all the images in images and puts
 * the result in std_dev_img. One can save the routine some time by providing
 * the average of the images in avg_img. If it is not available, avg_img should
 * be NULL.
 *
 * The images in images are assumed to be black and white, although the
 * image_to_matrix_routine is used in case it is not.
 *
 * Returns:
 *     On error, this routine returns ERROR, with an error message being set.
 *     On success it returns NO_ERROR.
 *
 * Index: images
 *
 * Related: image_to_matrix
 *
 * Author: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int std_dev_bw_images
(
    KJB_image**               std_dev_img,
    const KJB_image_sequence* images,
    const KJB_image*          avg_img
)
{
    Matrix* sdv_img_mat = NULL;
    Matrix* avg_img_mat = NULL;

    if(avg_img != NULL)
    {
        image_to_matrix(avg_img, &avg_img_mat);
    }

    ERE(std_dev_bw_images_2(&sdv_img_mat, images, avg_img_mat));
    ERE(matrix_to_bw_image(sdv_img_mat, std_dev_img));

    free_matrix(sdv_img_mat);
    if(avg_img != NULL)
    {
        free_matrix(avg_img_mat);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  average_bw_images_2
 *
 * Averages a set of images
 *
 * This routine averages all the images in images and puts the result in
 * avg_img_mat, in matrix form.
 *
 * The images in images are assumed to be black and white, although the
 * image_to_matrix_routine is used in case it is not.
 *
 * Returns:
 *     On error, this routine returns ERROR, with an error message being set.
 *     On success it returns NO_ERROR.
 *
 * Index: images
 *
 * Related: image_to_matrix
 *
 * Author: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int average_bw_images_2
(
    Matrix**                  avg_img_mat,
    const KJB_image_sequence* images
)
{
    int i;
    Matrix* temp_img_mat = NULL;

    if(images == NULL || avg_img_mat == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 1; i < images->length; i++)
    {
        if(images->elements[i]->num_rows != images->elements[0]->num_rows || images->elements[i]->num_cols != images->elements[0]->num_cols)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    get_initialized_matrix(avg_img_mat, images->elements[0]->num_rows, images->elements[0]->num_cols, 0.0);

    for(i = 0; i < images->length; i++)
    {
        image_to_matrix(images->elements[i], &temp_img_mat);
        ow_add_matrices(*avg_img_mat, temp_img_mat);
    }

    ow_divide_matrix_by_scalar(*avg_img_mat, images->length);

    free_matrix(temp_img_mat);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                  std_dev_bw_images_2
 *
 * Computes the standard deviation of a set of images
 *
 * This routine finds the standard deviation all the images in images and puts
 * the result in std_dev_img_mat, in matrix form. If the matrix representing the
 * average of the images is available, it can be passed in avg_img_mat to save
 * some time; otherwise, pass NULL.
 *
 * The images in images are assumed to be black and white, although the
 * image_to_matrix_routine is used in case it is not.
 *
 * Returns:
 *     On error, this routine returns ERROR, with an error message being set.
 *     On success it returns NO_ERROR.
 *
 * Index: images
 *
 * Related: image_to_matrix
 *
 * Author: Ernesto Brau
 *
 * -----------------------------------------------------------------------------
*/

int std_dev_bw_images_2
(
    Matrix**                  std_dev_img_mat,
    const KJB_image_sequence* images,
    const Matrix*             avg_img_mat
)
{
    Matrix* avg_img_mat_loc = NULL;
    Matrix* temp_img_mat = NULL;

    int i;

    if(images == NULL || std_dev_img_mat == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for(i = 1; i < images->length; i++)
    {
        if(images->elements[i]->num_rows != images->elements[0]->num_rows || images->elements[i]->num_cols != images->elements[0]->num_cols)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if(avg_img_mat == NULL)
    {
        ERE(average_bw_images_2(&avg_img_mat_loc, images));
    }
    else
    {
        copy_matrix(&avg_img_mat_loc, avg_img_mat);
    }

    get_initialized_matrix(std_dev_img_mat, avg_img_mat_loc->num_rows, avg_img_mat_loc->num_cols, 0.0);

    for(i = 0; i < images->length; i++)
    {
        image_to_matrix(images->elements[i], &temp_img_mat);
        ow_subtract_matrices(temp_img_mat, avg_img_mat_loc);
        ow_square_matrix_elements(temp_img_mat);
        ow_add_matrices(*std_dev_img_mat, temp_img_mat);
    }

    ow_divide_matrix_by_scalar(*std_dev_img_mat, images->length - 1);
    ow_sqrt_matrix_elements(*std_dev_img_mat);

    free_matrix(avg_img_mat_loc);
    free_matrix(temp_img_mat);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

