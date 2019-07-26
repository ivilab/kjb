
/* $Id: i_bw_byte.c 8780 2011-02-27 23:42:02Z predoehl $ */

#include "i/i_gen.h"     /* Only safe as first include in a ".c" file. */
#include "i/i_float.h"
#include "i/i_bw_byte.h"

#ifdef __cplusplus
extern "C" {
#endif

/*int write_ircamera_image_jpg(unsigned char *pixels, const char *filename)*/
static Bw_byte_image* create_bw_byte_image(int num_rows, int num_cols);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             bw_byte_image_to_kjb_image
 *
 * Converts a bw_byte_image to a KJB_image
 *
 * This routine implements the image creation/over-writing semantics used in
 * the KJB library in the case of Black and White Byte images. If *target_ipp 
 * is NULL, then
 * this routine creates the image. If it is not null, and it is the right
 * size, then this routine does nothing. If it is the wrong size, then it is
 * resized.
 *
 * In order to be memory efficient, we free before resizing. If the resizing
 * fails, then the original contents of the *target_ipp will be lost.
 * However, *target_ipp will be set to NULL, so it can be safely sent to
 * free_bw_byte_image(). Note that this is in fact the convention throughout the
 * KJB library--if destruction on failure is a problem (usually when
 * *target_ipp is global)--then work on a copy!
 *
 * Related:
 *    Bw_byte_image, kjb_free_bw_byte_image
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure with an error message being set.
 *
 * Author:
 *     Kate Spriggs
 *
 * Documentor:
 *     Kate Spriggs
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/

int get_target_bw_byte_image(Bw_byte_image **target_ipp, int num_rows, int num_cols)
{
  if ((num_rows <= 0) || (num_cols <= 0))
    {
      SET_ARGUMENT_BUG();
      return ERROR;
    }
  else if (*target_ipp == NULL)
    {
      NRE(*target_ipp = create_bw_byte_image(num_rows, num_cols));
    }
  else
    {
      /*
      if ((*target_ipp)->read_only)
        {
      SET_ARGUMENT_BUG();
      return ERROR;
        }
      */
      if (    ((*target_ipp)->num_rows != num_rows)
          || ((*target_ipp)->num_cols != num_cols))
        {
      kjb_free_bw_byte_image(*target_ipp);
      NRE(*target_ipp = create_bw_byte_image(num_rows, num_cols));
        }
    }

  return NO_ERROR;

}

void kjb_free_bw_byte_image(Bw_byte_image* ip)
{
  free_2D_byte_array(ip->pixels);
  kjb_free(ip);
}

static Bw_byte_image* create_bw_byte_image(int num_rows, int num_cols)
{
  Bw_byte_image* ip;
  if ((num_rows <= 0) || (num_cols <= 0))
    {
      SET_ARGUMENT_BUG();
      return NULL;
    }

  NRN(ip = TYPE_MALLOC(Bw_byte_image));

  ip->pixels = allocate_2D_byte_array(num_rows, num_cols);

  if (ip->pixels == NULL)
    {
      kjb_free(ip);
      return NULL;
    }

  ip->num_rows = num_rows;
  ip->num_cols = num_cols;
  ip->read_only = FALSE;

  return ip;

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             bw_byte_image_to_kjb_image
 *
 * Converts a bw_byte_image to a KJB_image
 *
 * This routine converts a bw_byte_image to a KJB_image.
 *
 * Related:
 *    Bw_byte_image, KJB_image
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure with an error message being set.
 *
 * Author:
 *     Kate Spriggs
 *
 * Documentor:
 *     Kate Spriggs
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/


int bw_byte_image_to_kjb_image(KJB_image ** target_ipp, const Bw_byte_image* source_ip)
{
  int i,j;
  KJB_image *target_ip = NULL;

  if(source_ip == NULL)
    {
      set_error("bw_byte_image_to_kjb_image: source_ip is NULL.\n");
      return ERROR;
    }
  if(* target_ipp != NULL)
    {
      kjb_free_image(* target_ipp);
      * target_ipp = NULL;
    }
  ERE(get_target_image(target_ipp, source_ip->num_rows, source_ip->num_cols));
  
  target_ip = *target_ipp;

  for(i= 0; i < source_ip->num_rows; i++)
    {
      for(j= 0; j < source_ip->num_cols; j++)
        {
          target_ip->pixels[i][j].r = source_ip->pixels[i][j];
          target_ip->pixels[i][j].g = source_ip->pixels[i][j];
          target_ip->pixels[i][j].b = source_ip->pixels[i][j];
        }
    }

  

  return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             rotate_bw_byte_image
 *
 * Rotates a bw_byte_image to the right 180 degrees.
 *
 * This routine rotates an input image to the right 180 degrees. The result image
 * is created or resized as needed in conformance with KJB library semantics.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure with an error message being set.
 *
 * Index: images, image rotation, image transformation
 *
 * -----------------------------------------------------------------------------
*/

int rotate_bw_byte_image(Bw_byte_image** target_ipp, const Bw_byte_image* ip)
{
  Bw_byte_image* target_ip = NULL;
  int   i, j, num_rows, num_cols;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    ERE(get_target_bw_byte_image(target_ipp, ip->num_rows, ip->num_cols));
    target_ip = *target_ipp;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
      /*target_ip->pixels[j][num_rows - 1 - i] = ip->pixels[i][j];*/
      target_ip->pixels[num_rows - 1 - i][num_cols - 1 - j] = ip->pixels[i][j];
        }
    }

    /*kjb_free_bw_byte_image(target_ip);*/
    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             kjb_copy_bw_byte_image
 *
 * Copies a bw_byte image
 *
 * This routine copies the bw_byte image pointed to by the input parameter
 * "source_ip" into *target_ipp which is created are reszied as needed. If
 * source_ip is NULL, then *target_ipp becomes NULL also.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, basic image routines
 *
 * -----------------------------------------------------------------------------
*/

int kjb_copy_bw_byte_image(Bw_byte_image** target_ipp, const Bw_byte_image* source_ip)
{
    IMPORT int kjb_use_memcpy;
    Bw_byte_image*  target_ip;
    int           num_rows;
    int           num_cols;
    int           i;
    int           j;
    unsigned char*  source_image_row_pos;
    unsigned char*  target_image_row_pos;


    if (source_ip == NULL)
    {
        kjb_free_bw_byte_image(*target_ipp);
        *target_ipp = NULL;
        return NO_ERROR;
    }

    num_rows = source_ip->num_rows;
    num_cols = source_ip->num_cols;

    ERE(get_target_bw_byte_image(target_ipp, num_rows, num_cols));

    if (kjb_use_memcpy)
    {
        /* If we add resize capability, see the code for Matrix */

        (void)memcpy( (*target_ipp)->pixels[ 0 ],
                     source_ip->pixels[ 0 ],
                     ((size_t)num_rows * num_cols) * sizeof(unsigned char));
    }
    else
    {
        target_ip = *target_ipp;

        for (i=0; i<num_rows; i++)
        {
            source_image_row_pos = source_ip->pixels[ i ];
            target_image_row_pos = target_ip->pixels[ i ];

            for (j=0; j<num_cols; j++)
            {
                *target_image_row_pos = *source_image_row_pos;

                target_image_row_pos++;
                source_image_row_pos++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          ow_get_bw_byte_image_face_region
 *
 * Extracts an image region
 *
 *
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, basic image routines
 *
 * -----------------------------------------------------------------------------
*/

int get_bw_byte_image_face_region(Bw_byte_image **target_ipp, Bw_byte_image * source_ip, 
                  int x, int y, int num_cols, int num_rows)
{
  int i, j;
  if (source_ip == NULL)
    {
      set_error("get_bw_byte_image_face_region: source_ip is NULL.\n");
      return ERROR;
    }

  if(*target_ipp != NULL)
    {
      kjb_free_bw_byte_image(*target_ipp);
      *target_ipp = NULL;
    }

  ERE(get_target_bw_byte_image(target_ipp, num_rows, num_cols));
  
  /*rows = width*/
  for(j = 0; j < num_cols; j++)    
    {
      for(i = 0; i < num_rows; i ++)
    {
      (*target_ipp)->pixels[i][j] = source_ip->pixels[i+y][j+x];
      /*(*target_ipp)->pixels[i][j] = j;*/
    }
    }
    /*
  for( i = 0; i < num_rows; i++)
    {
      kjb_memcpy((*target_ipp)->pixels[i], source_ip->pixels[i+y], num_cols);
    }
  */
  return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             kjb_byte_image_to_bw_image
 *
 * Converts a KJB_image to a bw_byte_image
 *
 * This routine converts a KJB_image to a bw_byte_image by averaging the 
 * pixel values of kjb_image.
 *
 * Related:
 *    Bw_byte_image, KJB_image
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure with an error message being set.
 *
 * Author:
 *     Kate Spriggs
 *
 * Documentor:
 *     Ernesto Brau
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/

int kjb_image_to_bw_byte_image ( Bw_byte_image ** bw_image, const KJB_image * kjb_image )
{
    Bw_byte_image *image;
    int i, j;

    ERE(get_target_bw_byte_image(bw_image, kjb_image->num_rows, kjb_image->num_cols));
    image = *bw_image;

    for (i = 0; i < kjb_image->num_rows; i++)
    {
        for (j = 0; j < kjb_image->num_cols; j++)
        {
            image->pixels[i][j] = (unsigned char)((1.0/3.0) * (kjb_image->pixels[i][j].r +
                                   kjb_image->pixels[i][j].g + kjb_image->pixels[i][j].b));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif
