
/* $Id: i_vector.c 13028 2012-09-26 00:43:03Z qtung $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2009 by Kobus Barnard.
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
#include "l/l_sys_io.h"
#include "i/i_gen.h"
#include "i/i_vector.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifdef TRACK_MEMORY_ALLOCATION

/* =============================================================================
 *                             read_image_vector
 *
 * Creates an image vector by reading in images
 *
 * Reads in all images with regular expression file_pattern.
 *
 * Returns:
 *    On error, this routine returns ERROR, with an error message being set.
 *    On success,  it returns a NO_ERROR and the target image vector will be allcoated
 *
 * Author:
 *    Qiyam Tung
 *
 * Index: image_vectors
 *
 * -----------------------------------------------------------------------------
*/

int debug_read_image_vector(KJB_image_vector **target_ivpp,
                            const char *file_pattern,
                            int         num_rows,
                            int         num_cols,
                            const char* filename,
                            int         line_number )
{
  int i;
  Word_list *dir_files_p;
  
  if ((num_rows <= 0) || (num_cols <= 0))
  {
    SET_ARGUMENT_BUG();
    return ERROR;
  }
  
  NRE(*target_ivpp = DEBUG_TYPE_MALLOC(KJB_image_vector, filename, line_number));
  
  dir_files_p = NULL;
  ERE(kjb_glob(&dir_files_p, file_pattern, NULL));
  
  (*target_ivpp)->length = dir_files_p->num_words;
  NRE((*target_ivpp)->images = (KJB_image**)debug_kjb_calloc(sizeof(KJB_image **),
                                                 (dir_files_p->num_words), 
                                                 file_pattern,
                                                 line_number));
  
  for (i = 0; i < dir_files_p->num_words; i++)
  {
    ERE(kjb_read_image_2(&(*target_ivpp)->images[i], dir_files_p->words[i]));
  }

  return NO_ERROR;
}
  

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int read_image_vector(KJB_image_vector **target_ivpp, const char *file_pattern,
                            int num_rows, int num_cols)
{
  int i;
  Word_list *dir_files_p;

  if ((num_rows <= 0) || (num_cols <= 0))
  {
    SET_ARGUMENT_BUG();
    return ERROR;
  }

  NRE((*target_ivpp) = TYPE_MALLOC(KJB_image_vector));
 
  dir_files_p = NULL;
  ERE(kjb_glob(&dir_files_p, file_pattern, NULL));

  (*target_ivpp)->length = dir_files_p->num_words;
  NRE((*target_ivpp)->images = (KJB_image **)kjb_calloc(sizeof(KJB_image **), (*target_ivpp)->length));

  for (i = 0; i < dir_files_p->num_words; i++)
  {
    ERE(kjb_read_image_2(&(*target_ivpp)->images[i], dir_files_p->words[i]));
  }

  return NO_ERROR;
}
#endif

#ifdef TRACK_MEMORY_ALLOCATION

/* =============================================================================
 *                             copy_image_vector
 *
 * Copies an image vector
 *
 * This routine copies an image vector.
 *
 *
 * The routine free_image_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns ERROR, with an error message being set.
 *    On success,  it returns a NO_ERROR and the target image vector will be allcoated
 *
 * Author: 
 *    Qiyam Tung
 *
 * Index: image_vectors
 *
 * -----------------------------------------------------------------------------
*/
int debug_copy_image_vector
(
    KJB_image_vector **out_ivpp,
    const KJB_image_vector *in_ivp,
    const char* filename,
    int         line_number)
{
  KJB_image **out_ip2;
  KJB_image **in_ip2;
  int num_images;
  int i;
  
  if (in_ivp == NULL)
  {
    free_image_vector(*out_ivpp);
    *out_ivpp = NULL;
    return NO_ERROR;
  }

  num_images = in_ivp->length;
  
  
  if (    (*out_ivpp == NULL)
       && ((*out_ivpp)->length != num_images)
     )
  {
    free_image_vector(*out_ivpp);
    *out_ivpp = NULL;

  }
  
  ERE(debug_get_target_image_vector(out_ivpp, num_images, 
                                    in_ivp->images[0]->num_rows, 
                                    in_ivp->images[0]->num_cols,
                                    filename, line_number));

  out_ip2 = (*out_ivpp)->images;
  in_ip2  = in_ivp->images;

  for (i = 0; i < num_images; i++)
  {
    ERE(kjb_copy_image(&(out_ip2[ i ]), in_ip2[ i ]));
  }

  return NO_ERROR;
}
  

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int copy_image_vector(KJB_image_vector **out_ivpp, const KJB_image_vector *in_ivp)
{
  int i;
  int num_images;
  KJB_image **out_ip2;
  KJB_image **in_ip2;
  
  if (in_ivp == NULL)
  {
    free_image_vector(*out_ivpp);
    *out_ivpp = NULL;
    return NO_ERROR;
  }

  num_images = in_ivp->length;
  
  if (    (*out_ivpp == NULL)
       && ((*out_ivpp)->length != num_images)
     )
  {
    free_image_vector(*out_ivpp);
    *out_ivpp = NULL;
  }
  
  ERE(get_target_image_vector(out_ivpp, num_images, in_ivp->images[0]->num_rows, 
                              in_ivp->images[0]->num_cols));

  out_ip2 = (*out_ivpp)->images;
  in_ip2  = in_ivp->images;

  for (i = 0; i < num_images; i++)
  {
    ERE(kjb_copy_image(&(out_ip2[ i ]), in_ip2[ i ]));
  }

  return NO_ERROR;
}
#endif

#ifdef TRACK_MEMORY_ALLOCATION

/* =============================================================================
 *                             get_target_image_vector
 *
 * Creates an image vector
 *
 * Creates an image vector of a given length and dimension.
 *
 * The routine free_image_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns ERROR, with an error message being set.
 *    On success,  it returns a NO_ERROR and the target image vector will be allcoated
 *
 * Author: 
 *    Qiyam Tung
 *
 * Index: image_vectors
 *
 * -----------------------------------------------------------------------------
*/
int debug_get_target_image_vector
(
   KJB_image_vector **ivpp,
   int         num_images,
   int         num_rows,
   int         num_cols,
   const char* filename,
   int         line_number
)
{
  int i;


  if (   (*ivpp != NULL)
      && ((*ivpp)->length == num_images)
     )
  {
    return NO_ERROR;
  }

  free_image_vector(*ivpp);

  *ivpp = DEBUG_TYPE_MALLOC(KJB_image_vector, filename, line_number);
  NRE((*ivpp)->images = (KJB_image **)debug_kjb_calloc(sizeof(KJB_image **), num_images,
                                         filename, line_number));
  (*ivpp)->length = num_images;

  for (i = 0; i < num_images; i++)
  {
    ERE(get_target_image(&(*ivpp)->images[i], num_rows, num_cols));
  }

  return NO_ERROR;
}
  

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */
int get_target_image_vector(KJB_image_vector **ivpp,
                                  int         num_images,
                                  int         num_rows,
                                  int         num_cols)
{
  int i;


  if (   (*ivpp != NULL)
      && ((*ivpp)->length == num_images)
     )
  {
    return NO_ERROR;
  }

  free_image_vector(*ivpp);

  *ivpp = TYPE_MALLOC(KJB_image_vector);
  NRE((*ivpp)->images = (KJB_image **)kjb_calloc(sizeof(KJB_image **), num_images));
                                       
  (*ivpp)->length = num_images;

  for (i = 0; i < num_images; i++)
  {
    ERE(get_target_image(&(*ivpp)->images[i], num_rows, num_cols));
  }

  return NO_ERROR;
}

#endif


/* =============================================================================
 *                             free_image_vector
 *
 * Frees an image vector
 *
 * Frees an image vector
 *
 * Returns:
 *    Nothing.
 *
 * Author: 
 *    Qiyam Tung
 *
 * Index: image_vectors
 *
 * -----------------------------------------------------------------------------
*/
void free_image_vector(KJB_image_vector *ivp)
{
  int count, i;
  KJB_image **ip_array_pos;


  if (ivp == NULL)
  {
    return;
  }

  ip_array_pos = ivp->images;
  count = ivp->length;

  for (i = 0; i < count; i++)
  {
    kjb_free_image(*ip_array_pos);
    ip_array_pos++;
  }

  kjb_free(ivp->images);
  kjb_free(ivp);
}

#ifdef __cplusplus
}

#endif
