
/* $Id: seg_spots.c 17796 2014-10-21 04:17:21Z predoehl $ */

/* ========================================================================== *
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
* ========================================================================== */

#include <l/l_error.h>
#include <l/l_sort.h>
#include <i/i_float_io.h>
#include <seg/seg_spots.h>
#include <seg/seg_connected_components.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */

static int get_connected_components
(
    const Int_matrix*   in_mp,
    int                 min_size,
    int                 max_size,
    Int_matrix_vector** components,
    Vector_vector**     centroids
);

/* ------------------------------------------------------------------------- */

/* ============================================================================
 *                      find_bright_spots_in_image
 *
 * Finds bright spots in single image
 *
 * This accepts a single image and finds local neighborhoods of pixels that are
 * all "bright" according to the complicated definition below -- basically,
 * exceeding a relative threshold determined from inputs 'background,'
 * 'thresholds,' 'min_brightness' and 'similarity.'  The only shape constraint
 * on the neighborhoods is that they must be 8-connected regions of pixels,
 * within a given size range, i.e., a feasible pixel count.
 *
 * The image is converted to a matrix by uniform (!) weighting of RGB channels.
 * Then the background matrix is subtracted, element-wise.  You, the caller,
 * might prepare this matrix as the elementwise arithmetic average of pixel
 * intensities in a series.  After subtracting, we search the image and locate
 * the pixel B (for bright) with the highest remaining intensity, i.e., B has
 * the biggest intensity shift above the background value.  Pixel B is used to
 * judge whether the input image is "bright enough" to process.  There are two
 * requirements to be "bright enough."  First, the intensity of 'img' at
 * location B must equal or exceed 'min_brightness.'  Second, the magnitude of
 * the intensity shift above background at location B must equal or exceed the
 * value of 'thresholds' at location B.  If both are true, the image is
 * bright enough that it might generate bright spots.  Otherwise, 'img' is not
 * bright enough:  the function immediately returns NO_ERROR, and the output
 * containers are empty (although they still must be freed by you, the caller).
 *
 * The 'thresholds' matrix contains local threshold values at each pixel
 * location.  You, the caller, might compute this matrix as the element-wise
 * standard deviations of the pixel intensities in a series, or a scaled
 * version thereof, or something similar.  It does not represent a threshold
 * value for all bright pixels, just a threshold value to be compared with
 * relatively bright pixel B.
 *
 * An image pixel is intense enough to be a candidate for a bright spot if its
 * intensity shift above background exceeds the product of 'similarity' times
 * the intensity shift above background at B.  Thus 'similarity' should be a
 * positive value typically less than 1.  (A 'similarity' value above 1 is
 * silently clipped back to 1.)  Such pixels are grouped into eight-connected
 * regions.  If the region's total size (i.e., its pixel count) is at least
 * 'min_size' and at most 'max_size' then it is formally considered a (bright)
 * spot.
 *
 * The function produces two kinds of output.  The first output, *spots, is
 * optional, listing the exact positions of all spot pixels.  The second
 * output, *centroids, is required, listing the position of each spot's center
 * of mass.  If the pixel coordinates are not required, you can call the
 * routine with 'spots' equal to NULL.  You, the caller, of course have
 * ownership of the output container object(s) and must free them, whether
 * or not the image has any bright spots of acceptable size.
 *
 * To elaborate:  at successful exit,
 * (*centroids)->length contains the number of spots that were detected.
 * The centroid of the pixels of the i-th spot can then be found in
 * (*centroids)->elements[i], which points to a 2-element Vector.  Its index-0
 * and index-1 elements contain, respectively, the centroid X and Y coords.
 * If parameter 'spots' does not equal NULL, then at successful
 * exit, *spots points to an array of Int_matrix objects.  Then,
 * (*spots)->elements[i] points to an M-by-2 Int_matrix, where M is the number
 * of pixels in the i-th spot.  Row k, column 0 contains the k-th pixel's X
 * coordinate, and row k column 1 contains its Y coordinate.
 *
 * Returns:
 *    On error this routine returns ERROR with an error message being set.
 *    On success it returns NO_ERROR.  Absence of bright spots is not an error.
 *
 * Related:
 *    find_bright_spots_in_image_sequence
 *
 * Authors:
 *    Ernesto Brau and Andrew Predoehl
 *
 * Index: image processing
 * ----------------------------------------------------------------------------
*/

int find_bright_spots_in_image
(
    const KJB_image*    img,            /* image to be scanned               */
    const Matrix*       background,     /* pixelwise non-bright baseline     */
    const Matrix*       thresholds,     /* used to qualify brightest pixel B */
    int                 min_brightness, /* also qualifies brightest pixel B  */
    int                 min_size,       /* num. pixels in smallest spot      */
    int                 max_size,       /* num. pixels in largest spot       */
    double              similarity,     /* factor of spot-pixel threshold    */
    Int_matrix_vector** spots,          /* Optional output matrices          */
    Vector_vector**     centroids       /* Mandatory output vectors          */
)
{
    Matrix *img_mat = NULL;
    Matrix* fg_img_mat = NULL;
    Int_matrix* bin_img_mat = NULL;
    int return_val = ERROR;

    double max_val;
    int max_row, max_col;

    NRE(img);
    NRE(background);
    NRE(thresholds);
    NRE(centroids);

    if (similarity <= 0.0)
    {
        set_error("similarity must be positive (but, received value was %e)\n",
                similarity);
        NOTE_ERROR();
        return ERROR;
    }

    EGC(image_to_matrix(img, &img_mat));
    EGC(subtract_matrices(&fg_img_mat, img_mat, background));
    EGC(get_max_matrix_element(fg_img_mat, &max_val, &max_row, &max_col));

    if (    max_val < thresholds -> elements[max_row][max_col]
        ||  img_mat -> elements[max_row][max_col] < min_brightness
       )
    {
        /* No bright spots found:  all pixels were too dim, at least
         * when compared to the matrix of threshold values.
         * This outcome is possibly a disappointment, but not an error.
         */
        EGC(get_target_vector_vector(centroids, 0));
        if (spots != NULL)
        {
            EGC(get_target_int_matrix_vector(spots, 0));
        }
    }
    else
    {
        /* Create a boolean matrix of pixels exceeding the thresholds.
         * Search the matrix for connected components, and make a list.
         * If 'similarity' exceeds 1, it is as if it were clamped at 1.
         */
        if (similarity < 1.0) max_val *= similarity;
        EGC(find_in_matrix(&bin_img_mat, fg_img_mat,
                           is_matrix_element_greater_than, (void*)&max_val));

        EGC(get_connected_components(bin_img_mat, min_size, max_size,
                                     spots, centroids));
    }

    return_val = NO_ERROR;

cleanup:
    if (ERROR == return_val && *centroids)
    {
        free_vector_vector(*centroids);
        *centroids = NULL;
    }
    free_matrix(img_mat);
    free_matrix(fg_img_mat);
    free_int_matrix(bin_img_mat);

    return return_val;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* ============================================================================
 *                      find_bright_spots_in_image_sequence
 *
 * Finds bright spots in sequence of images
 *
 * Please see the documentation for find_bright_spots_in_image, which is
 * similar.
 * This routine calls that function on an image sequence, and uses a
 * background matrix as the pixel-wise average of the input image intensities,
 * (assuming uniform channel weighting).  The threshold matrix is set to the
 * pixel-wise standard devation of the input image intensities.
 *
 * Returns:
 *    On error this routine returns ERROR with an error message being set.
 *    On success it returns NO_ERROR.
 *
 * Author: Ernesto Brau
 *
 * Documenter: Ernesto Brau
 *
 * Related:
 *     find_bright_spots_in_image
 *
 * Index: image processing
 * ----------------------------------------------------------------------------
*/

int find_bright_spots_in_image_sequence
(
    V_v_v**                    points,
    Int_matrix_vector_vector** blobs,
    const KJB_image_sequence*  images,
    int                        min_brightness,
    int                        min_blob_size,
    int                        max_blob_size,
    double                     similarity
)
{
    Matrix* average_img_mat = NULL;
    Matrix* std_dev_img_mat = NULL;
    int return_val = ERROR;

    V_v_v* centroids;
    int i, T;

    NRE(points);
    NRE(images);

    ERE(average_bw_images_2(&average_img_mat, images));
    EGC(std_dev_bw_images_2(&std_dev_img_mat, images, average_img_mat));

    T = images -> length;
    get_target_v3(points, T);
    centroids = *points;
    if (blobs != NULL)
    {
        EGC(get_target_int_matrix_vector_vector(blobs, T));
    }

    for (i = 0; i < T; i++)
    {
        EGC(find_bright_spots_in_image(
                images->elements[i],
                average_img_mat,
                std_dev_img_mat,
                min_brightness,
                min_blob_size,
                max_blob_size,
                similarity,
                blobs ? (*blobs) -> elements + i : NULL,
                centroids -> elements + i
            ));

        if (0 == centroids -> elements[i] -> length)
        {
            ERE(pso("Warning: brightest pixel is not bright enough to be "
                    "a blob.\n"));
        }
    }

    return_val = NO_ERROR;

cleanup:
    free_matrix(average_img_mat);
    free_matrix(std_dev_img_mat);
    return return_val;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

static int get_connected_components
(
    const Int_matrix*   in_mp,
    int                 min_size,
    int                 max_size,
    Int_matrix_vector** components,
    Vector_vector**     centroids
)
{
    Int_matrix* temp_int_mat = NULL;
    KJB_region_list* region_list = NULL;
    int return_val = ERROR;
    int cur_comp_lbl = 0;
    int cur_comp_idx = 0;
    int num_valid_regions = 0;

    Vector **cca;      /* base of centroid coordinates array   */
    Int_matrix **cpra; /* base of component pixel roster array */

    int i, num_regions, max_label;

    NRE(centroids);
    NRE(in_mp);

    ERE(label_eight_connected_regions(&temp_int_mat, &region_list, NULL,
                             &num_regions, &max_label, NULL, in_mp, 1));

    /* Count the valid regions (not background, and size within constraints) */
    for (i = 0; i <= max_label; ++i)
    {
        const KJB_region *r = region_list -> regions + i;
        if (    r->label == r->root_label  &&  r->root_label != 0
            &&  min_size <= r->moment_00  &&  r->moment_00 <= max_size)
        {
            ++num_valid_regions;
        }
    }

    /* Allocate output storage (or exit with cleanup, if we cannot). */
    EGC(get_target_vector_vector(centroids, num_valid_regions));
    cca = (*centroids) -> elements;
    if (components != NULL)
    {
        EGC(get_target_int_matrix_vector(components, num_valid_regions));
        cpra = (*components) -> elements;
    }

    /* If no regions are valid, depart gracefully (with cleanup). */
    if (0 == num_valid_regions)
    {
        return_val = NO_ERROR;
        goto cleanup;
    }

    /* Scan all connected components meeting size constraints. */
    for (i = 0; i <= max_label; ++i)
    {
        const KJB_region *r = region_list -> regions + i;
        if (r->label == r->root_label  &&  r->root_label != 0)
        {
            double rm00 = r -> moment_00; /* num. pixels in component */
            if (min_size <= rm00 && rm00 <= max_size)
            {
                Vector *cc; /* this centroid's coordinates */
                EGC(get_target_vector(cca + cur_comp_idx, 2));
                cc = cca[cur_comp_idx];
                cc -> elements[0] = region_list -> regions[i].moment_10 / rm00;
                cc -> elements[1] = region_list -> regions[i].moment_01 / rm00;

                if (components != NULL)
                {
                    int j, cur_px = 0;
                    Int_matrix* cur_comp; /* pixel roster of this component */
                    EGC(get_target_int_matrix(cpra + cur_comp_idx, rm00, 2));
                    cur_comp = cpra[cur_comp_idx];

                    /* scan bounding box of region around current component */
                    for (j = r -> min_row; j <= r -> max_row; ++j)
                    {
                        int k;
                        for (k = r -> min_col; k <= r -> max_col; ++k)
                        {
                            if (temp_int_mat->elements[j][k] == cur_comp_lbl+1)
                            {
                                cur_comp->elements[cur_px][0] = k; /*x coord*/
                                cur_comp->elements[cur_px][1] = j; /*y coord*/
                                ++cur_px;
                            }
                        }
                    }
                    ASSERT(cur_px == rm00);
                }
                ++cur_comp_idx;
            }
            ++cur_comp_lbl;
        }
    }

    return_val = NO_ERROR;

cleanup:
    if (ERROR == return_val)
    {
        if (components && *components)
        {
            free_int_matrix_vector(*components);
            *components = NULL;
        }
        if (*centroids)
        {
            free_vector_vector(*centroids);
            *centroids = NULL;
        }
    }
    free_int_matrix(temp_int_mat);
    free_region_list(region_list);
    return return_val;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */


#ifdef __cplusplus
}
#endif

