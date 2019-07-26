
/* $Id: i_mode_filter.c 4727 2009-11-16 20:53:54Z kobus $ */

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
#include "i/i_gen.h"
#include "i/i_mode_filter.h"

#ifdef __cplusplus
extern "C" {
#endif

int compare_pixel(const void *_pixel1, const void *_pixel2);

/* =============================================================================
 *                           compare_pixel
 *
 * Compares pixel values as a 3-digit number.
 * 
 * This method establishes a partial-order among pixels.
 * R = 100s, G = 10s, B = 1s.
 * For example, 255 170 100 > 255 173 159 > 254 170 159.
 *
 * Returns: 
 *     1 if pixel1's value > pixel2's value
 *    -1 if pixel1's value < pixel2's value
 *     0 if pixel1's value = pixel2's value
 * Author:
 *     Qiyam Tung
 *
 *
 * -----------------------------------------------------------------------------
*/

int compare_pixel(const void *_pixel1, const void *_pixel2)
{
  
  const Pixel *pixel1, *pixel2;
 
  pixel1 = (const Pixel *)_pixel1;
  pixel2 = (const Pixel *)_pixel2;

  if (pixel1->r > pixel2->r)
  {
    return FIRST_ITEM_GREATER;
  } 
  else if (pixel1->r < pixel2->r)
  {
    return SECOND_ITEM_GREATER;
  }
  else if (pixel1->g > pixel2->g)
  {
    return FIRST_ITEM_GREATER;
  }
  else if (pixel1->g < pixel2->g)
  {
    return SECOND_ITEM_GREATER;
  }
  else if (pixel1->b > pixel2->b)
  {
    return FIRST_ITEM_GREATER;
  }
  else if (pixel1->b < pixel2->b)
  {
    return SECOND_ITEM_GREATER;
  }
  return EQUAL_ITEMS;
}

/* =============================================================================
 *                              mode_filter
 * 
 *  Applies a mode filter on a series of images.
 *
 *  This filter applies the mode filter on all RGB combinations.
 *  In this case, we define the mode as the most common RGB combination
 *  because of the difficulty of classifying a mode for 3 dimensions.
 *  It sorts the pixels first and then walks through the array of images 
 *  looking for the most popular pixel (mode) per row and col of the image.
 *
 *  Returns
 *     On error, it sets an error message and returns ERROR
 *     On success, it returns NO_ERROR
 * 
 *  Author:
 *      Qiyam Tung
 * -----------------------------------------------------------------------------
 */

int mode_filter(KJB_image_vector *ivp, KJB_image **output_image)
{
  int i, j, k;
  int cur_count, max_count;
  Pixel max_pixel, cur_pixel;
  Pixel *array_of_pixels;


  if (ivp == NULL)
  {
    return NO_ERROR;
  }

  ERE(get_target_image(output_image, ivp->images[0]->num_rows, ivp->images[0]->num_cols));
  
  /*  An image vector does not fit into standard sorting formats.
   *  We need to put each pixel into its own array to sort.
   * 
   *  The algorithm goes as follows:
   *  1. Copy pixels (1 of each image) into an array
   *  2. Sort 
   *  3. Find the most popular pixel of that array 
   *  4. Copy it to the corresponding i-j in the mode image.
   *  5. Go back to step 1 until all pixels have been copied. */
  array_of_pixels = (Pixel *)kjb_malloc(sizeof(Pixel)*ivp->length);

  for (i = 0; i < ivp->images[0]->num_rows; i++)
  {
    for (j = 0; j < ivp->images[0]->num_cols; j++)
    {
      for (k = 0; k < ivp->length; k++)
      {
        memcpy(&(array_of_pixels[k]), &(ivp->images[k]->pixels[i][j]), sizeof(Pixel));
      }
      ERE(kjb_sort(array_of_pixels, ivp->length, sizeof(Pixel), compare_pixel,
                   USE_CURRENT_ATN_HANDLING));

      /* Find the most popular pixel */
      memcpy(&max_pixel, array_of_pixels, sizeof(Pixel)); 
      memcpy(&cur_pixel, array_of_pixels, sizeof(Pixel)); 
      max_count = 0;
      cur_count = 0;
      
      for (k = 1; k < ivp->length; k++){
        if (   (cur_pixel.r == array_of_pixels[k].r)
            && (cur_pixel.g == array_of_pixels[k].g)
            && (cur_pixel.b == array_of_pixels[k].b))
        {
          cur_count++;
        } 

        /* If we've found a new pixel or are at the last pixel, we should
         * see whether a more popular pixel was found. */
        if (   (    (cur_pixel.r != array_of_pixels[k].r)
                 || (cur_pixel.g != array_of_pixels[k].g)
                 || (cur_pixel.b != array_of_pixels[k].b)
               )
            || (k + 1 == ivp->length)
           )
        {
          if (max_count < cur_count)
          {
            memcpy(&max_pixel, &(array_of_pixels[k - 1]), sizeof(Pixel)); 
            max_count = cur_count;
          }
          memcpy(&cur_pixel, &(array_of_pixels[k]), sizeof(Pixel));  
          cur_count = 0;
        }
      }
      memcpy(&(*output_image)->pixels[i][j], &max_pixel, sizeof(Pixel));
    }  
  }

  kjb_free(array_of_pixels);

  return NO_ERROR;
}
#ifdef __cplusplus
}
#endif
