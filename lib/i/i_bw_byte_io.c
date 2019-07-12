
/* $Id: i_bw_byte_io.c 8780 2011-02-27 23:42:02Z predoehl $ */

#include "i/i_float.h"
#include "i/i_bw_byte_io.h"
#include <stdio.h>
/*
#include <stdlib.h>
#include <string.h>
*/

#ifdef __cplusplus
extern "C" {
#endif

int write_bw_byte_image(Bw_byte_image *image, const char * file_name)
{
  int i, j;
  KJB_image *pImage = NULL;

  pImage = kjb_create_image(image->num_rows, image->num_cols);
  if (!pImage)
    {
      printf("Failed to create a KJB image \n");
      return -1;
    }
  for(i= 0; i < image->num_rows; i++)
    {
      for(j= 0; j < image->num_cols; j++)
        {
          pImage->pixels[i][j].r = image->pixels[i][j];
          pImage->pixels[i][j].g = image->pixels[i][j];
          pImage->pixels[i][j].b = image->pixels[i][j];
        }
    }

  kjb_write_image(pImage, file_name);

  kjb_free_image(pImage);
  return 0;
}

int read_bw_byte_image(Bw_byte_image **bw_byte_image_pp, const char * file_name)
{

  KJB_image *image_p = NULL;
  int i,j;

  if(kjb_read_image_2(&image_p, file_name) == ERROR)
    {
      return ERROR;
    }
  
  if(*bw_byte_image_pp == NULL)
    {
      get_target_bw_byte_image(bw_byte_image_pp, image_p->num_rows, image_p->num_cols);
    }
  else
    {
      /*check if the right size*/
      if( (*bw_byte_image_pp)->num_rows != image_p->num_rows || 
      (*bw_byte_image_pp)->num_cols != image_p->num_cols)
    {
      kjb_free_bw_byte_image( *bw_byte_image_pp);
      *bw_byte_image_pp = NULL;
      get_target_bw_byte_image(bw_byte_image_pp, image_p->num_rows, image_p->num_cols);
    }
    }

  for(i= 0; i < image_p->num_rows; i++)
    {
      for(j= 0; j < image_p->num_cols; j++)
        {

      (*bw_byte_image_pp)->pixels[i][j] = image_p->pixels[i][j].r;
    }
    }

  kjb_free_image(image_p);
  return NO_ERROR;
}

#ifdef __cplusplus
}
#endif
