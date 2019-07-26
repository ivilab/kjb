/* $Id: keypoint.c 22174 2018-07-01 21:49:18Z kobus $ */
/* =========================================================================== *
 * |
 * |  Copyright (c) 1994-2012 by Kobus Barnard. 
 * |
 * |  Personal and educational use of this code is granted, provided that this
 * |  header is kept intact, and that the authorship is not misrepresented, that
 * |  its use is acknowledged in publications, and relevant papers are cited.
 * |
 * |  Please note that the code in this file has not necessarily been adequately
 * |  tested. Naturally, there is no guarantee of performance, support, or fitness
 * |  for any particular task. Nonetheless, I am interested in hearing about
 * |  problems that you encounter. Contact me (Kate) at ykk+SLIC (at) cs.arizona.edu
 * |
 * * =========================================================================== */


#include "kpt/keypoint.h"

#ifdef __cplusplus
extern "C" {
#endif


/* To avoid cluttered look, skip over some kpts when drawing */ 
static int fs_kpt_draw_skip_step = 0; 
/* When drawing two images, how much space to leave between them. */
static int fs_image_gap = 0;  
/* Draw the matched images side-by-side or one on top of another. */
static int fs_draw_side_by_side = FALSE;
static int fs_different_color_lines = TRUE;
static int fs_kpt_red_val = 75; /* part of the RGB value of the keypoint visualization */
static int fs_kpt_green_val = 0; /* part of the RGB value of the keypoint visualization */
static int fs_kpt_blue_val = 135; /* part of the RGB value of the keypoint visualization */

static int LINE_WIDTH = 0; 

int set_keypoint_options(const char* option, const char* value)
{
    int result = NOT_FOUND;
    char lc_option[ 100 ];
    int temp_int_value;
    int temp_boolean_value;

    EXTENDED_LC_BUFF_CPY( lc_option, option );

    if ( match_pattern( lc_option, "kpt-draw-skip-step" ) )
    {
        if ( value == NULL )
        {
            return NO_ERROR;
        }
        if ( value[0] == '\0' )
        {
            pso( "The keypoint drawing skip step is '%d'\n", fs_kpt_draw_skip_step );
        }
        else if (value[0] == '?')
        {
            pso( "kpt-draw-skip-step = %d\n", fs_kpt_draw_skip_step );
        }
        else
        {
            ERE( ss1pi(value, &temp_int_value) );

            if ( temp_int_value < 0 )
            {
                set_error("kpt-draw-skip-step should be positive.");
                return ERROR;
            }
            fs_kpt_draw_skip_step = temp_int_value;
        }
        result = NO_ERROR;
    }
/* --------------------------------------------------------------------------------------------- */
    if ( match_pattern( lc_option, "image-gap" ) )
    {
        if ( value == NULL )
        {
            return NO_ERROR;
        }
        if ( value[0] == '\0' )
        {
            pso( "The gap between the images is '%d' pixels.\n", fs_image_gap );
        }
        else if (value[0] == '?')
        {
            pso( "image-gap = %d\n", fs_image_gap );
        }
        else
        {
            ERE( ss1pi(value, &temp_int_value) );

            if ( temp_int_value < 0 )
            {
                set_error("image-gap should be positive.");
                return ERROR;
            }
            fs_image_gap = temp_int_value;
        }
        result = NO_ERROR;
    }
/* --------------------------------------------------------------------------------------------- */
    if ( match_pattern( lc_option, "draw-side-by-side" ) )
    {
        if ( value == NULL )
        {
            return NO_ERROR;
        }
        if ( value[0] == '\0' )
        {
            if ( fs_draw_side_by_side == TRUE)
            {
                pso( "The matched images will be drawn side by side.\n");
            }
            else
            {
                pso( "The matched images will be drawn one on top of another.\n");
            }
        }
        else if (value[0] == '?')
        {
            pso( "draw-side-by-side = %s\n", fs_draw_side_by_side ? "t" : "f" );
        }
        else
        {
            ERE( temp_boolean_value = get_boolean_value(value));
            fs_draw_side_by_side = temp_boolean_value;
        }
        result = NO_ERROR;
    }
/* --------------------------------------------------------------------------------------------- */
    if ( match_pattern( lc_option, "draw-colorful-lines" ) )
    {
        if ( value == NULL )
        {
            return NO_ERROR;
        }
        if ( value[0] == '\0' )
        {
            if ( fs_different_color_lines == TRUE)
            {
                pso( "The matched lines will be drawn in different colors.\n");
            }
            else
            {
                pso( "The matched lines will not be drawn in different colors.\n");
            }
        }
        else if (value[0] == '?')
        {
            pso( "draw-colorful-lines = %s\n", fs_different_color_lines ? "t" : "f" );
        }
        else
        {
            ERE( temp_boolean_value = get_boolean_value(value));
            fs_different_color_lines = temp_boolean_value;
        }
        result = NO_ERROR;
    }
/* --------------------------------------------------------------------------------------------- */
    if ( match_pattern( lc_option, "kpt-red-val" ) )
    {
        if ( value == NULL )
        {
            return NO_ERROR;
        }
        if ( value[0] == '\0' )
        {
            pso( "The red channel for drawing keypoints is set to %d.\n", fs_kpt_red_val );
        }
        else if (value[0] == '?')
        {
            pso( "kpt-red-val = %d\n", fs_kpt_red_val );
        }
        else
        {
            ERE( ss1pi(value, &temp_int_value) );

            if ( temp_int_value < 0 )
            {
                set_error("kpt-red-val should be positive.");
                return ERROR;
            }
            fs_kpt_red_val = temp_int_value;
        }
        result = NO_ERROR;
    }
/* --------------------------------------------------------------------------------------------- */
    if ( match_pattern( lc_option, "kpt-green-val" ) )
    {
        if ( value == NULL )
        {
            return NO_ERROR;
        }
        if ( value[0] == '\0' )
        {
            pso( "The green channel for drawing keypoints is set to %d.\n", fs_kpt_green_val );
        }
        else if (value[0] == '?')
        {
            pso( "kpt-green-val = %d\n", fs_kpt_green_val );
        }
        else
        {
            ERE( ss1pi(value, &temp_int_value) );

            if ( temp_int_value < 0 )
            {
                set_error("kpt-green-val should be positive.");
                return ERROR;
            }
            fs_kpt_green_val = temp_int_value;
        }
        result = NO_ERROR;
    }
/* --------------------------------------------------------------------------------------------- */
    if ( match_pattern( lc_option, "kpt-blue-val" ) )
    {
        if ( value == NULL )
        {
            return NO_ERROR;
        }
        if ( value[0] == '\0' )
        {
            pso( "The blue channel for drawing keypoints is set to %d.\n", fs_kpt_blue_val );
        }
        else if (value[0] == '?')
        {
            pso( "kpt-blue-val = %d\n", fs_kpt_blue_val );
        }
        else
        {
            ERE( ss1pi(value, &temp_int_value) );

            if ( temp_int_value < 0 )
            {
                set_error("kpt-blue-val should be positive.");
                return ERROR;
            }
            fs_kpt_blue_val = temp_int_value;
        }
        result = NO_ERROR;
    }
/* --------------------------------------------------------------------------------------------- */

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 create_keypoint
 * Create a keypoint
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    One success it returns a pointer to the Keypoint.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
 */
Keypoint*   create_keypoint( void )
{
   Keypoint *kp ;
   int      result = NO_ERROR;
   
   NRN( kp = TYPE_MALLOC( Keypoint ) );

   kp->descrip = NULL;
   result = get_target_vector( &(kp->descrip), KEYPOINT_DESCRIP_LENGTH ); 
   if (kp->descrip == NULL || result == ERROR)
   {
       add_error("ERROR (%s +%d): Unable to allocate memory for the keypoint descriptor.", __FILE__, __LINE__);
       free_keypoint( kp );
       return NULL;
   }

   kp->row     = NOT_SET;
   kp->col     = NOT_SET;
   kp->scale   = NOT_SET;
   kp->ori     = NOT_SET;
   
   return kp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                get_target_keypoint 
 *
 * Gets target keypoint 
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of keypoints. If *target_kpp is NULL, then this routine
 * creates the keypoint . If it is not null, and it is the right size (for now
 * we only use 128-dimensional descriptor), then this
 * routine does nothing. 
 *
 * Note that the routine creates a standard 128-dimensional keypoint vector,
 * hence why there's no need to specify the length of the vector.
 *
 * Index: target, keypoints 
 *
 * TODO: add support for setting the descriptor length
 *
 * -----------------------------------------------------------------------------
*/

int get_target_keypoint (Keypoint ** target_kpp)
{
    Keypoint * kp = *target_kpp;

    if (kp == NULL)
    {
        NRE(kp = create_keypoint( ));
        *target_kpp = kp;
    }
    else /*if (length == kp->length)*/
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    return NO_ERROR;
}

#if 0
/*int set_keypoint
( 
    Keypoint** kpt_ptr_ptr,
    const float   row,
    const float   col,
    const float   scale,
    const float   ori,
    const Vector  *descr
)
{
    Keypoint *kpt_ptr = *kpt_ptr_ptr;

    if (kpt_ptr != NULL)
    {
        kpt_ptr->row = row;
        kpt_ptr->col = col;
        kpt_ptr->scale = scale;
        kpt_ptr->ori = ori;

        kpt_ptr->descrip = descr;
    }

    return NO_ERROR;
}
*/
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            copy_keypoint
 *
 * Copies a keypoint 
 *
 * This routine copies a Keypoint pointed to by the source_kp to the 
 * Keypoint pointed to by the *target_kp. If *target_kp is NULL, then it
 * is created.
 *
 * Returns:
 *  NO_ERROR on success and ERROR on failure with an appropriate error message set.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/

int copy_keypoint
(
    Keypoint** target_kpp,
    const Keypoint* source_kp
)
{
    if (source_kp == NULL)
    {
        free_keypoint( *target_kpp );
        *target_kpp = NULL;
        return NO_ERROR;
    }

    if ( *target_kpp == source_kp )
    {
        return NO_ERROR;
    }

    ERE( get_target_keypoint( target_kpp ));

    (*target_kpp)->row = source_kp->row;
    (*target_kpp)->col = source_kp->col;
    (*target_kpp)->ori = source_kp->ori;
    (*target_kpp)->scale = source_kp->scale;

    ERE( copy_vector( &((*target_kpp)->descrip), 
                      source_kp->descrip ) );
    
    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                set_keypoint_from_vector
 *                            
 *  Sets the values of a keypoint
 *
 *  Sets the values of a keypoint to those provided in a 132 element vector
 *
 *  Vector elements 0-3 store keypoint location (x,y), scale and orientation;
 *  Vector elements 4-131 are for the keypoint descriptor
 *
 *  Returns:
 *  NO_ERROR on success or ERROR with an error message set.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
 */
int set_keypoint_from_vector
( 
    Keypoint**      kpp,
    const Vector    *kpt_val_vp
)
{
    Keypoint*      kpt_ptr = NULL;

    /* If the vector is empty */
    if (kpt_val_vp == NULL)
    {
        free_keypoint( kpt_ptr );
        free_keypoint( *kpp );
        *kpp = NULL;
        return NO_ERROR;
    }

    ERE( get_target_keypoint( kpp )); 

    if (kpp != NULL)
    {
        Vector *descr = NULL;
        kpt_ptr = *kpp;

        ERE( copy_vector_segment( &descr, kpt_val_vp, 4,  KEYPOINT_DESCRIP_LENGTH ) );
        /* TODO: Verify that the descriptor values are not < 0 or > 255) */
        ERE( copy_vector( &(kpt_ptr->descrip), descr ));
        if (kpt_ptr->descrip == NULL)
        {
            add_error("ERROR (%s +%d): An error has occurred while copying keypoint descriptors.", __FILE__, __LINE__);
            free_vector( descr );
            free_keypoint( kpt_ptr );
            free_keypoint( *kpp );
            *kpp = NULL;
            return ERROR;
        }

        kpt_ptr->row =   kpt_val_vp->elements[0];
        kpt_ptr->col =   kpt_val_vp->elements[1];
        kpt_ptr->scale = kpt_val_vp->elements[2];
        kpt_ptr->ori =   kpt_val_vp->elements[3]; 

        free_vector( descr );
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 free_keypoint
 *                               
 * Frees the space associated with a Keypoint
 * 
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
 */
void free_keypoint( Keypoint *kp )
{
    if (kp != NULL)
    {
        if (kp->descrip != NULL)
        {
/*#ifdef CHECK_KEYPOINT_INITIALZATION_ON_FREE*/
            /*
             * We must have a valid pointer to check the initialization,
             * otherwise problems such as double free can look like unitialized
             * memory.
            */
/*            if (kjb_debug_level >= 10)
            {
                check_initialization(kp->descrip, kp->descrip->length, sizeof(Vector));
            }
*/
/* #endif */
            /* kjb_free( kp->descrip ); */
            free_vector( kp->descrip );
        }
        kjb_free( kp );
    }

}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *               read_vl_keypoints_into_matrix
 *
 * Reads in the VLFeat keypoints from a file.
 *
 * Uses KJB fp_read_formatted_matrix to read in the keypoints from the
 * specified file into a matrix.
 *
 * Input parameters: 
 * |    const char      *fname - a filename from which to read the kpts
 * |    Matrix**        kpt_mpp - a matrix where to store the keypoints
 *
 * Returns: an error code.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int read_vl_keypoints_into_matrix
(
    const char      *fname,
    Matrix**        kpt_mpp
)
{
    FILE *fp = NULL;
    int result = NOT_FOUND;

    verbose_pso(9, "Opening '%s'...\n", fname);
    fp = kjb_fopen( fname, "r" );
    if (fp == NULL)
    {
        warn_pso("(%s +%d): A keypoint file '%s' does not exist.", __FILE__, __LINE__, fname);
        add_error("ERROR (%s +%d): A keypoint file '%s' does not exist.", __FILE__, __LINE__, fname);
        ERE(kjb_fclose(fp));
        return ERROR;
    }

    verbose_pso(9, "Reading the formatted matrix...\n");
    result = fp_read_formatted_matrix(kpt_mpp, fp);
    if (result == ERROR)
    {
        add_error("ERROR (%s +%d): Unable to read VL keypoints into a matrix.", __FILE__, __LINE__);
    }
    else if (result == EOF)
    {
        /*warn_pso("(%s +%d) There are no VL keypoints to read.\n", __FILE__, __LINE__);*/
        verbose_pso(8, "(%s +%d) There are no VL keypoints to read.\n", __FILE__, __LINE__);
        free_matrix(*kpt_mpp);
        *kpt_mpp = NULL;
        result = NO_ERROR; /* If there were no keypoints read, no need to continue? */
    }

    kjb_fclose(fp);
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *               read_vl_keypoint_vector_from_file    
 *
 * Read a vector of keypoints from a named file
 *
 * The function reads-in the keypoints from the provided
 * filename into a Keypoint_vector.
 *
 * The function originally reads the contents of the file into a 
 * matrix and then creates a Keypoint_vector and populates it with
 * the values from the matrix.
 *
 * The vector is set to NULL, if there are no keypoints in the file.
 *
 * Input parameters:
 *  const char * fname - the name of the file with the keypoints
 *  Keypoint_vector** output_kvpp - a vector where keypoints are stored
 *
 * Returns: NO_ERROR or an error code (or EOF) 
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
 */
int read_vl_keypoint_vector_from_file
(
    const char *                fname,
    Keypoint_vector**           output_kvpp
)
{
    int             result = NO_ERROR;
    Matrix          *image_kpt_mp = NULL;

    /* Read in the keypoints and store them into a matrix */
    verbose_pso(9, " Reading VL keypoints into matrix.\n");
    result = read_vl_keypoints_into_matrix( fname, &image_kpt_mp ); 
    if (result == ERROR)
    {
        add_error("ERROR (%s +%d): Unable to read keypoints from '%s'.", __FILE__, __LINE__, fname);
        free_matrix( image_kpt_mp );
        return result;
    }
    /*
    else if (result == EOF)
    {
        free_matrix( image_kpt_mp );
    }
    */


    verbose_pso(9, " Getting VL keypoints vector from matrix.\n");
    result = get_keypoint_vector_from_matrix( output_kvpp, image_kpt_mp);
    if (result == ERROR)
    {
        add_error("ERROR (%s +%d): Unable to get keypoint vector from matrix", __FILE__, __LINE__ );
        free_matrix( image_kpt_mp );
        return result;
    }

    free_matrix( image_kpt_mp );
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *               extract_selected_keypoints_positions
 *
 * Extract the (x,y) coordinates of the VLFeat keypoints.
 *
 * Input parameters: 
 *  Matrix**   pos_mpp - where to store the position of the keypoints
 *  Keypoint_vector *kvp - contains image keypoints and descriptors
 *  Int_vector *mask_ivp - if an element is negative, then the corresponding
 *                         element in the Keypoint_vector is not extracted.
 *
 * Returns: an error code.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int extract_selected_keypoints_positions
(
    Matrix**                  pos_mpp,
    const Keypoint_vector     *kvp,
    const Int_vector          *mask_ivp
)
{
    int result = NO_ERROR;
    int i;
    int selected ;
    int length;
    int num_to_extract;

    if( kvp == NULL)
    {
        return NO_ERROR;
    }
    if (mask_ivp == NULL)
    {
        result = extract_keypoints_positions(pos_mpp, kvp);
        return result;
    }

    length = kvp->length;
    if (length != mask_ivp->length)
    {
        add_error("Mask vector is of a different dimension than the keypoint vector.");
        add_error("%d != %d", length, mask_ivp->length);
        return ERROR;
    }

    num_to_extract = 0;
    for (i = 0; i < length; i++)
    {
        if (mask_ivp->elements[i] >= 0)
        {
            num_to_extract++;
        }
    }

    /* 2 columns: for x and for y coordinates */
    ERE( get_target_matrix( pos_mpp, num_to_extract, 2 ));

    selected = 0;
    for (i=0; i < length; i++)
    {
        if (mask_ivp->elements[i] >= 0)
        {
            (*pos_mpp)->elements[selected][0] = kvp->elements[i]->row;
            (*pos_mpp)->elements[selected][1] = kvp->elements[i]->col;
            selected++;
        }
    }

    return result;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ============================================================================
 *               extract_keypoints_positions
 *
 * Extract the (x,y) coordinates of the VLFeat keypoints.
 *
 * Input parameters: 
 *  Matrix**   pos_mpp - where to store the position of the keypoints
 *  Keypoint_vector *kvp - contains image keypoints and descriptors
 *
 * Returns: an error code.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int extract_keypoints_positions
(
    Matrix**                  pos_mpp,
    const Keypoint_vector     *kvp
)
{
    int result = NO_ERROR;
    int i;
    int length;

    if( kvp == NULL)
    {
        return NO_ERROR;
    }

    length = kvp->length;

    /* 2 columns: for x and for y coordinates */
    ERE( get_target_matrix( pos_mpp, length, 2 ));

    for (i=0; i < length; i++)
    {
        (*pos_mpp)->elements[i][0] = kvp->elements[i]->row;
        (*pos_mpp)->elements[i][1] = kvp->elements[i]->col;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             create_keypoint_vector
 *
 * Creates a keypoint vector
 *
 * This routine creates a Keypoint_vector vector of size "length" which is a type for a
 * vector of keypoints.  All keypoint pointers are set to NULL.
 *
 * The routine free_keypoint_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL
 *    On success it returns a pointer to the Keypoint_vector.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/

Keypoint_vector* create_keypoint_vector(int length)
{
    Keypoint_vector* kvp;
    int              i;

    NRN( kvp = TYPE_MALLOC( Keypoint_vector ) );
    kvp->length = length;

    if (length > 0)
    {
        NRN(kvp->elements = N_TYPE_MALLOC(Keypoint*, length));
    }
    else
    {
        kvp->elements = NULL;
    }

    for (i=0; i<length; i++)
    {
        kvp->elements[ i ] = NULL; 
    }
    return kvp;
}

#if 0
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_matrix_from_keypoint_vector 
 *
 * Constructs a matrix from a keypoint vector  
 *
 * This routine creates a matrix and puts each Keypoint in the vector as a row.
 * It uses the conventions from set_keypoint_from_vector.
 * 
 * The first argument is the Keypoint vector. If the
 * keypoint vector is NULL, then the target matrix becomes NULL as well.
 * If the target matrix is the wrong size, it is resized.
 * Finally, if it is the right size, then the storage is recycled, as is.
 * Returns:
 *     NO_ERROR on success and ERROR on failure 
 *    On error, this routine returns NULL
 *    On success it returns a pointer to the matrix.
 *
 * -----------------------------------------------------------------------------
*/
/*
int get_matrix_from_keypoint_vector
(
    Matrix** mp,
    const Keypoint_vector* kvp
)
{
    return NO_ERROR;
}
*/
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            get_keypoint_vector_from_matrix
 *
 * Constructs a keypoint vector from a matrix
 *
 * This routine creates a Keypoint_vector vector and puts each row as a Keypoint
 * entry in the vector.
 *
 * The first argument is the address of the target Keypoint vector. If the target
 * keypoint vector is null, then a keypoint vector of the appropriate size is
 * created. 
 * 
 * If the matrix mp is NULL, then the target keypoint vector becomes NULL as well.
 * 
 * Returns:
 *     NO_ERROR on success and ERROR on failure.
 *     This routine will only fail if storage allocation fails.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/

int get_keypoint_vector_from_matrix 
(
    Keypoint_vector** kvpp,
    const Matrix* mp
)
{
    int i;
    int length;
    Keypoint_vector* kvp = NULL;
    Vector* tmp_vp = NULL;

    if (kvpp == NULL)
    {
        add_error("ERROR (%s +%d): About to get a seg fault. Terminating.\n",__FILE__, __LINE__);
        return ERROR;
    }


    if (mp == NULL)
    {
        verbose_pso(7, "(%s +%d): The matrix is empty.\n", __FILE__, __LINE__);
        free_vector( tmp_vp );
        free_keypoint_vector(*kvpp);
        *kvpp = NULL;
        return NO_ERROR;
    }

    length = mp->num_rows;

    ERE( get_target_keypoint_vector( kvpp, length ));
    kvp = *kvpp;

    for (i=0; i < length; i++)
    {
        ERE( get_matrix_row( &tmp_vp, mp, i ));
        ERE( set_keypoint_from_vector( &kvp->elements[i], tmp_vp ));
    }

    free_vector( tmp_vp );
    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            copy_keypoint_vector
 *
 * Copies a keypoint vector from a matrix
 *
 * This routine copies a Keypoint_vector pointed to by the source_kvp to the 
 * Keypoint_vector pointed to by the *target_kvp. If *target_kvp is NULL, then it
 * is created.
 *
 * Returns:
 *  NO_ERROR on success and ERROR on failure with an appropriate error message set.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/

int copy_keypoint_vector
(
    Keypoint_vector** target_kvpp,
    const Keypoint_vector* source_kvp
)
{
    int i;
    int length;

    /* TODO: Make sure we are not overwriting the original vector! */
    if (source_kvp == NULL)
    {
        free_keypoint_vector( *target_kvpp );
        *target_kvpp = NULL;
        return NO_ERROR;
    }

    length = source_kvp->length;

    if (    (*target_kvpp != NULL) 
         && ((*target_kvpp)->length != length) )
    {
        free_keypoint_vector( *target_kvpp );
        *target_kvpp = NULL;
    }

    ERE( get_target_keypoint_vector( target_kvpp, length ));

    for (i = 0; i < length; i++)
    {
        ERE( get_target_keypoint( &(*target_kvpp)->elements[i] ));
        ERE( copy_keypoint( &((*target_kvpp)->elements[i]), source_kvp->elements[i]) );
    }
    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            copy_keypoint_vector_selected_rows
 *
 * Copies a keypoint vector from a matrix
 *
 * Copies a keypoint vector from a matrix, ignoring the keypoints whose index
 * is set to 0 in the mask_ivp.
 *
 * This routine copies a Keypoint_vector pointed to by the source_kvp to the 
 * Keypoint_vector pointed to by the *target_kvp. 
 * If the mask_ivp element for the keypoint is 0, then the keypoint is not copied.
 * If *target_kvp is NULL, then it is created.
 *
 * Returns:
 *  NO_ERROR on success and ERROR on failure with an appropriate error message set.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int copy_keypoint_vector_selected_rows
(
    Keypoint_vector** target_kvpp,
    const Keypoint_vector* source_kvp,
    const Int_vector* mask_ivp
)
{
    int i;
    int j;
    int length = 0;
    int target_length = 0;
    /* TODO: Make sure we are not overwriting the original vector! */

    if (source_kvp == NULL)
    {
        free_keypoint_vector( *target_kvpp );
        *target_kvpp = NULL;
        return NO_ERROR;
    }

    length = source_kvp->length;
    verbose_pso(9, " Source kvp length = %d\n", length);
    if (mask_ivp->length != length)
    {
        warn_pso( " The mask vector doesn't match the number of elements in the source keypoint vector.");
        insert_error( " The mask vector doesn't match the number of elements in the source keypoint vector.");
        free_keypoint_vector( *target_kvpp );
        *target_kvpp = NULL;
        return ERROR;
    }

    /* Get the number of elements that need to be copied */
    for (i=0; i < length; i++)
    {
        if (mask_ivp->elements[i] > 0)
        {
            target_length++;
        }
    }

    verbose_pso(9, " Copying %d keypoints.\n", target_length);

    if (    (*target_kvpp != NULL) 
         && ((*target_kvpp)->length != target_length) )
    {
        free_keypoint_vector( *target_kvpp );
        *target_kvpp = NULL;
    }

    ERE( get_target_keypoint_vector( target_kvpp, target_length ));

    j = 0;
    for (i = 0; i < length; i++)
    {
        if (mask_ivp->elements[i] > 0)
        {
            ERE( get_target_keypoint( &(*target_kvpp)->elements[j] ));
            ERE( copy_keypoint( &((*target_kvpp)->elements[j]), source_kvp->elements[i]) );
            j++;

            /*
            db_rv( source_kvp->elements[i]->descrip );
            */
        }
    }
    return NO_ERROR;
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           write_vl_keypoint_vector
 *
 * Writes a Keypoint_vector in a VL format to a file
 *
 * This routine outputs a Keypoint_vector in a VL format to a file specified by 
 * the input file name.
 *
 * "file_name" is a pointer to a character array containing the name
 * of the file to write the Keypoint_vector contents to.
 * "kvp" is a pointer to the Keypoint_vector whose contents need to be written.
 * If the vector is NULL, then this routine is a NOP.
 *
 * Returns:
 *     NO_ERROR on success, or ERROR on a file write or close error.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int write_vl_keypoint_vector
(
    const Keypoint_vector* kvp,
    const char* file_name
)
{
    FILE* fp;
    int i;
    int length;
    Vector* kpt_vp = NULL;
    int   result = NO_ERROR;

    if( kvp == NULL )
    {
        verbose_pso(8, " Keypoint vector is empty; skip writing it.\n");
        return NO_ERROR;
    }

    if (file_name != NULL) 
    {
        if (skip_because_no_overwrite(file_name)) return NO_ERROR;

        verbose_pso(9, " Writing vl keypoint vector to %s\n", file_name);
        NRE(fp = kjb_fopen(file_name, "w"));
    }
    else
    {
        return ERROR;
    }
    
    length = kvp->length;

    for (i = 0; i < length; i++)
    {
        result = get_vector_from_keypoint( &kpt_vp, kvp->elements[i] );
        if (result == ERROR)
        {
            kjb_fclose(fp);
            free_vector( kpt_vp );
            return ERROR;
        }
        /*db_rv(kpt_vp);*/

        result = fp_write_row_vector( kpt_vp, fp );
        if (result == ERROR)
        {
            kjb_fclose(fp);
            free_vector( kpt_vp );
            return ERROR;
        } 
    }
   
    result = kjb_fclose(fp);

    free_vector( kpt_vp );

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       get_vector_from_keypoint
 *
 * Turn the keypoint values into a vector
 *
 * Turn the keypoint values into a vector of length KEYPOINT_DESCRIP_LENGTH + 4
 * (for x,y, scale and orientation parameters).
 *
 * Returns:
 *  NO_ERROR on success or ERROR with an error message set.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int get_vector_from_keypoint
(
    Vector** vpp,           /* output vector */
    const Keypoint* kpt     /* keypoint to turn into a vector */
)
{
    int length = KEYPOINT_DESCRIP_LENGTH + 4; /* (x,y), scale, orient */
    Vector* vp;
    int    i;

    if (kpt == NULL)
    {
        verbose_pso(7, " Keypoint is empty; not getting the vector.\n");
        return NO_ERROR;
    }

    if (vpp == NULL)
    {
        verbose_pso(6, " You are about to get a seg fault. Aborting.\n");
        SET_ARGUMENT_BUG();
    }

    ERE( get_target_vector(vpp, length) );
    vp = *vpp;

    vp->elements[0] = kpt->row;
    vp->elements[1] = kpt->col;
    vp->elements[2] = kpt->scale;
    vp->elements[3] = kpt->ori;

    for (i = 0; i < KEYPOINT_DESCRIP_LENGTH; i++)
    {
        vp->elements[i+4] = (kpt->descrip)->elements[i];
    }

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       free_keypoint_vector
 *
 * Frees the storage in a keypoint vector.
 *
 * This routine frees the storage in a keypoint array.
 *
 * Index: memory allocation, arrays, keypoints 
 *
 * -----------------------------------------------------------------------------
*/
void free_keypoint_vector (Keypoint_vector* kvp)
{
    int count, i;
    Keypoint** kp_array_pos;

    if (kvp != NULL)
    {
        if (kvp->elements != NULL)
        {   
            kp_array_pos = kvp->elements;
            count = kvp->length;

            for (i=0; i<count; i++)
            {
                free_keypoint(*kp_array_pos);
                kp_array_pos++;
            }

            kjb_free(kvp->elements);

        }
    }

    kjb_free(kvp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       get_target_keypoint_vector
 *
 * Gets a target keypoint vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of keypoint vectors. If *target_kvpp is
 * NULL, then this routine creates the keypoint vector. If it is not null, and it
 * is the right size, then this routine does nothing. If it is the wrong size,
 * then it is resized.
 *
 * The routine free_keypoint_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the keypoint vector.
 *
 * Index: memory allocation, arrays, keypoints
 *
 * -----------------------------------------------------------------------------
*/

int get_target_keypoint_vector(Keypoint_vector** kvpp, int length)
{

    if ( (*kvpp != NULL) && ((*kvpp)->length == length) )
    {
        return NO_ERROR;
    }

    free_keypoint_vector(*kvpp);
    NRE(*kvpp = create_keypoint_vector(length));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             create_keypoint_vector_vector
 *
 * Creates a keypoint vector vector
 *
 * This routine creates a vector of Keypoint_vector vector of size "length". 
 * All keypoint pointers are set to NULL.
 *
 * The routine free_keypoint_vector_vector should be used to dispose of the 
 * storage once it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL
 *    On success it returns a pointer to the array.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/

Keypoint_vector_vector* create_keypoint_vector_vector(int length)
{
    Keypoint_vector_vector* kvvp;
    int              i;

    NRN( kvvp = TYPE_MALLOC( Keypoint_vector_vector ) );
    kvvp->length = length;

    if (length > 0)
    {
        NRN(kvvp->elements = N_TYPE_MALLOC(Keypoint_vector*, length));
    }
    else
    {
        kvvp->elements = NULL;
    }

    for (i=0; i<length; i++)
    {
        kvvp->elements[ i ] = NULL;
    }

    return kvvp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                       get_target_keypoint_vector_vector
 *
 * Gets a target keypoint vector vector
 *
 * This routine implements the creation/over-writing semantics used in the KJB
 * library in the case of keypoint vector vectors. If *target_kvvpp is
 * NULL, then this routine creates the object. If it is not null, and it
 * is the right size, then this routine does nothing. If it is the wrong size,
 * then it is resized.
 *
 * The routine free_keypoint_vector_vector should be used to dispose of the storage once
 * it is no longer needed.
 *
 * Returns:
 *    On error, this routine returns NULL, with an error message being set.
 *    On success it returns a pointer to the array.
 *
 * Index: memory allocation, arrays, keypoints, vectors
 *
 * -----------------------------------------------------------------------------
*/

int get_target_keypoint_vector_vector(Keypoint_vector_vector** kvvpp, int length)
{

    if ( (*kvvpp != NULL) && ((*kvvpp)->length == length) )
    {
        return NO_ERROR;
    }

    free_keypoint_vector_vector(*kvvpp);
    NRE(*kvvpp = create_keypoint_vector_vector(length));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                       free_keypoint_vector_vector
 *
 * Frees the storage in a keypoint vector vector
 *
 * This routine frees the storage in a keypoint array.
 *
 * Returns: N/A
 *
 * Index: memory allocation, arrays, keypoints
 *
 * -----------------------------------------------------------------------------
*/

void free_keypoint_vector_vector (Keypoint_vector_vector* kvvp)
{
    int count, i;
    Keypoint_vector** kvpp;


    if (kvvp == NULL) return;

    kvpp = kvvp->elements;
    count = kvvp->length;

    for (i=0; i<count; i++)
    {
        free_keypoint_vector(*kvpp);
        kvpp++;
    }

    kjb_free(kvvp->elements);
    kjb_free(kvvp);
}


/* =============================================================================
 *                       free_keypoint_vector_descriptors
 *
 * Frees the descriptors of a keypoint vector.
 *
 * This routine frees the storage associated with descriptors in a keypoint array.
 *
 * Index: memory allocation, arrays, keypoints 
 *
 * -----------------------------------------------------------------------------
*/
void free_keypoint_vector_descriptors (Keypoint_vector* kvp)
{
    int count, i;

    if (kvp != NULL)
    {
        if (kvp->elements != NULL)
        {   
            count = kvp->length;

            for (i=0; i<count; i++)
            {
                free_vector( kvp->elements[i]->descrip );
                kvp->elements[i]->descrip = NULL; 
            }
        }
    }
}



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ===========================================================================
 *                           keypoint_euclidean_distance
 *
 * Computes the Euclidean distance between two keypoint descriptors.
 *
 * This routine computes the Euclidean distance between two keypoint which are
 * implemented as KJB Vectors. 
 *
 * Returns:
 *     NO_ERROR on success; ERROR on failure, with a descriptive error message
 *     being set.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int keypoint_euclidean_distance
(
    Vector* kpt1_descr_vp,
    Vector* kpt2_descr_vp,
    double* distance_ptr
)
{
    int     i;
    double    diff, sum;

    if ( (kpt1_descr_vp == NULL) || (kpt2_descr_vp == NULL) )
    {
        add_error("ERROR (%s +%d): One of the input vectors is NULL.", __FILE__, __LINE__);
        return ERROR;
    }

    if (kpt1_descr_vp->length != kpt2_descr_vp->length)
    {
        add_error("ERROR (%s +%d): Input vectors have different dimensions:", __FILE__, __LINE__);
        add_error(" (kpt1_descr_vp->length=%d)", kpt1_descr_vp->length);
        add_error(" (kpt2_descr_vp->length=%d)", kpt2_descr_vp->length);
        return ERROR;
    }

    sum = (double)0.0;
    for (i = 0; i < kpt1_descr_vp->length; i++)
    {
        diff = kpt1_descr_vp->elements[i] - kpt2_descr_vp->elements[i];
        sum += (diff * diff);
    }

    *distance_ptr = (double)sqrt((double)sum);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       get_keypoint_match
 *
 * Finds the closest match to the given keypoint in a vector of candidate keypoints.
 *
 * This routine finds the two nearest neighbors of the given keypoint. 
 * The similarity is measured by the Euclidean distance of the keypoint vectors.
 * Based on D. Lowe's "Distinctive image features from scale-invariant keypoints"
 * paper, the cloest match is selected based on the ratio of the distances to the
 * nearest and the second nearest neighbor.
 *
 * Input:
 *    const Keypoint        *target_kpt - the keypoint for which to find a match
 *    const Keypoint_vector *candidate_kvp - a vector of possible matches
 *    const int             dist_ratio - a ratio between 2 nearest keypoint matches
 *
 * Returns:
 *    On error, this routine returns ERROR.
 *    If the keypoint pointers are NULL, the routine returns NOT_SET.
 *    If a match was not found then the routine returns NOT_FOUND, otherwise,
 *    On success it returns the index of the keypoint matched to the target.
 *
 * Index: match, keypoints
 *
 * -----------------------------------------------------------------------------
*/
int get_keypoint_match 
(
    const Keypoint        *target_kpt,
    const Keypoint_vector *candidate_kvp,
    const double          dist_ratio
)
{
    int result       = NOT_FOUND;
    double dist_sq_1 = DBL_MAX;
    double dist_sq_2 = DBL_MAX;
    double dist_sq   = 0;
    int num_candidates     = 0;
    int i;
    Vector *descriptor = NULL; /* temporary holder for the match descriptors */
    int match_index  = NOT_FOUND;

    if ((target_kpt == NULL) || (candidate_kvp == NULL))
    {
        free_vector( descriptor );
        return NOT_SET;
    }

    num_candidates = candidate_kvp->length;

    for (i = 0; i < num_candidates; i++)
    {
        copy_vector( &descriptor, candidate_kvp->elements[i]->descrip);

        result = keypoint_euclidean_distance( target_kpt->descrip,
                                              descriptor, 
                                              &dist_sq );
        if (result != ERROR)
        {
            if (dist_sq < dist_sq_1) /* && (keypoint_dot_product() >= 0 */
            {
                dist_sq_2 = dist_sq_1;
                dist_sq_1 = dist_sq;
                match_index = i; 
            }
            else if (dist_sq < dist_sq_2)
            {
                dist_sq_2 = dist_sq;
            }
        }
        else
        {
            warn_pso(" An error has occurred while computing keypoint Euclidean distance.\n");
            result = ERROR;
            break;
        }
                                              
    } /* end for i < num_candidates */

    if (result != ERROR)
    {
        if ( dist_sq_1 < (dist_ratio * dist_sq_2))
        {
            result = match_index;
        }
        else
        {
            result = NOT_FOUND;
        }
    }

    free_vector( descriptor );
    return result;
}

/* =============================================================================
 *                       get_local_keypoint_match
 *
 * Finds the closest match to the given keypoint in a vector of candidate keypoints.
 * Closeness is measured by both, the location and descriptor similarity.
 *
 * This routine finds the two nearest neighbors of the given keypoint. 
 * The similarity is measured by the Euclidean distance of the keypoint vectors.
 * Based on D. Lowe's "Distinctive image features from scale-invariant keypoints"
 * paper. However, the cloest match is selected based on the ratio of the distances 
 * to the nearest and the second nearest neighbor, AND its location proximity.
 *
 * Input:
 *    const Keypoint        *target_kpt - the keypoint for which to find a match
 *    const Keypoint_vector *candidate_kvp - a vector of possible matches
 *    const Matrix          *H_mp - a homography that transforms target into candidate
 *    const int             dist_ratio - a ratio between 2 nearest keypoint matches
 *    const double          dist_thresh - a threshold on how far a match can be
 *
 * Returns:
 *    On error, this routine returns ERROR.
 *    If the keypoint pointers are NULL, the routine returns NOT_SET.
 *    If a match was not found then the routine returns NOT_FOUND, otherwise,
 *    On success it returns the index of the keypoint matched to the target.
 *
 * Index: match, keypoints
 *
 * Related: get_keypoint_match, find_putative_matches 
 *
 * -----------------------------------------------------------------------------
*/
int get_local_keypoint_match 
(
    const Keypoint        *target_kpt,
    const Keypoint_vector *candidate_kvp,
    /*const Matrix          *H_mp,*/
    const double          dist_ratio,
    const double          dist_thresh
)
{
    int result       = NOT_FOUND;
    double dist_sq_1 = DBL_MAX;
    double dist_sq_2 = DBL_MAX;
    double dist_sq   = 0;
    int num_candidates     = 0;
    int i;
    Vector *descriptor = NULL; /* temporary holder for the match descriptors */
    int match_index  = NOT_FOUND;
    Matrix *x_pos_mp = NULL;
    Matrix *y_pos_mp = NULL;
    Matrix *projected_pos_mp = NULL;
    /*
    double targ_x = 0;
    double targ_y = 0;
    double cand_x = 0;
    double cand_y = 0;
    */

    if ((target_kpt == NULL) || (candidate_kvp == NULL))
    {
        free_vector( descriptor );
        return NOT_SET;
    }

    num_candidates = candidate_kvp->length;

    /* Frame to slide */
    for (i = 0; i < num_candidates; i++)
    {
        if ( ( fabs(target_kpt->row - candidate_kvp->elements[i]->row) <= dist_thresh)
            && (fabs(target_kpt->col - candidate_kvp->elements[i]->col) <= dist_thresh) )
        {
            copy_vector( &descriptor, candidate_kvp->elements[i]->descrip);

            result = keypoint_euclidean_distance( target_kpt->descrip,
                                                  descriptor, 
                                                  &dist_sq );
            if (result != ERROR)
            {
                if (dist_sq < dist_sq_1) 
                {
                    dist_sq_2 = dist_sq_1;
                    dist_sq_1 = dist_sq;
                    match_index = i; 
                }
                else if (dist_sq < dist_sq_2)
                {
                    dist_sq_2 = dist_sq;
                }
            }
            else
            {
                warn_pso(" An error has occurred while computing keypoint Euclidean distance.\n");
                result = ERROR;
                break;
            }
        }                                     
    } /* end for i < num_candidates */

    if (result != ERROR)
    {
        if ( dist_sq_1 < (dist_ratio * dist_sq_2))
        {
            result = match_index;
        }
        else
        {
            result = NOT_FOUND;
        }
    }

    free_matrix( x_pos_mp );
    free_matrix( y_pos_mp );
    free_matrix( projected_pos_mp );
    free_vector( descriptor );
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               draw_oriented_keypoint
 *
 * Draw an oriented keypoint
 *
 * The function draws an oriented keypoint (a circle with a line 
 * indicating the keypoint direction) on a given image.
 *
 * Input parameters: 
 *  KJB_image* ip - a handle to the image
 *  float x, y - position of the keypoint 
 *  float scale - keypoint scale
 *  float rad - keypoint radius
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int draw_oriented_keypoint 
( 
    KJB_image* ip, 
    float x, 
    float y, 
    float scale, 
    float rad 
) 
{
    float x_c = 0;
    float y_c = 0;
     
    /*ERE(image_draw_circle(ip, y, x, scale, 1, 200, 0, 0));*/
    /*ERE(image_draw_circle(ip, y, x, scale, 1, fs_kpt_red_val, fs_kpt_green_val, fs_kpt_blue_val));*/
    ERE(image_draw_circle(ip, y, x, scale, LINE_WIDTH, fs_kpt_red_val, fs_kpt_green_val, fs_kpt_blue_val));

    /* Compute the endpoints of the line (point that lies on the circle */
    x_c = x + scale * cos(rad);
    y_c = y + scale * sin(rad);
    /*ERE(image_draw_segment_2(ip, y, x, y_c, x_c, 1, 200, 0,0));*/
    /*ERE(image_draw_segment_2(ip, y, x, y_c, x_c, 1, fs_kpt_red_val, fs_kpt_green_val, fs_kpt_blue_val));*/
    ERE(image_draw_segment_2(ip, y, x, y_c, x_c, LINE_WIDTH, fs_kpt_red_val, fs_kpt_green_val, fs_kpt_blue_val));
    
    return NO_ERROR;
} 


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               draw_oriented_keypoint_1
 * 
 * Draw keypoint with custom colors.
 *
 * Same as draw_oriented_keypoint, except that it is possible to customize
 * the color of the keypoint.
 * 
 * The function draws an oriented keypoint (a circle with a line 
 * indicating the keypoint direction) on a given image.
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int draw_oriented_keypoint_1 
( 
    KJB_image* ip, /* a handle to the image       */
    float x,       /* x position of the keypoint  */
    float y,       /* y position of the keypoint  */
    float scale,   /* keypoint scale              */
    float rad,     /* keypoint radius             */
    int red,       /* color of the drawn keypoint */
    int green,     /* color of the drawn keypoint */
    int blue       /* color of the drawn keypoint */
) 
{
    float x_c = 0;
    float y_c = 0;
     
/*    ERE(image_draw_point(ip, y, x, 1, red, green, blue));*/

    /*ERE(image_draw_circle(ip, x, y, scale, 1, 200, 0, 0));*/
    /*ERE(image_draw_circle(ip, y, x, scale, 1, red, green, blue));*/
    ERE(image_draw_circle(ip, y, x, scale, LINE_WIDTH, red, green, blue));

    /* Compute the endpoints of the line (point that lies on the circle */
    x_c = x + scale * cos(rad);
    y_c = y + scale * sin(rad);
    /*ERE(image_draw_segment_2(ip, y, x, y_c, x_c, 1, red, green, blue));*/
    ERE(image_draw_segment_2(ip, y, x, y_c, x_c, LINE_WIDTH, red, green, blue));
    
    return NO_ERROR;
} 



/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               draw_ubc_keypoints_from_file
 * 
 * Read in keypoints as vectors
 *
 * The function reads-in the keypoint vector values (location, scale 
 * and orientation) from a file and draws oriented keypoints on 
 * the provided image.
 *
 * Input parameters: 
 *  KJB_image* ip - a handle to the resulting image
 *  char* keypoint_filename - a file with the keypoints in the VLFeat 
 *                            format (each line contains x,y, scale, 
 *                            orientation, followed by the ddescriptor)
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int draw_ubc_keypoints_from_file (KJB_image* ip, char *keypoint_filename)
{
  int items_read = NOT_SET; /* a flag indicating if fscanf was successful */
  float total, length;
  FILE *fp = NULL;
  
  NRE(fp = kjb_fopen(keypoint_filename, "r"));
  
  items_read = fscanf(fp, "%f %f", &total, &length);
  if (items_read <= 0)
  {
      insert_error("Unable to read '%s'", keypoint_filename);
      return ERROR;
  }

  verbose_pso(9, "Total = %f, descriptor length = %f\n", total, length);
  if (length != KEYPOINT_DESCRIP_LENGTH)
  {
      add_error("ERROR (%s +%d): Invalid keypoint length (%f).", __FILE__, __LINE__, length);
      return ERROR;
  }
  
  ERE( fp_draw_vl_keypoints (fp, ip) );
  ERE( kjb_fclose(fp) );
  
  return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               fp_draw_vl_keypoints
 * 
 * Read keypoints, draw image
 *
 * The function reads-in the keypoint vector values (location, scale 
 * and orientation) from a file and draws oriented keypoints on 
 * the provided image.
 *
 * Input 'fp' is a file pointer to the file with the keypoints in the VLFeat 
 * format (each line contains x,y, scale, orientation, followed by the
 * ddescriptor).
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int fp_draw_vl_keypoints(
    FILE* fp,     /* input file                      */
    KJB_image* ip /* a handle to the resulting image */
)
{
    int items_read = NOT_SET; /* a flag indicating if fscanf was successful */
    float row, col, scale, orientation;
    int j;
    int tmp; /* used to read-in the descriptor values */
    
    items_read = fscanf(fp, "%f %f %f %f", &row, &col, &scale, &orientation);
    while (items_read > 0)
    {
        /* Read in all the SIFT keypoints from the file*/
        if (items_read != 4)
        {
            add_error("ERROR (%s +%d): fscanf did not read position, scale, "
                      "and orientation.", __FILE__, __LINE__);
            return ERROR;
        } 
    
        /* Read in all the descriptor values */
        /* TODO: replace with reading a vector */
        for (j = 0; j < KEYPOINT_DESCRIP_LENGTH ; j++)
        {
            items_read = fscanf(fp, "%d", &tmp);
            if (items_read != 1)
            {
                add_error("ERROR (%s +%d): fscanf could not read the "
                          "descriptor value.", __FILE__, __LINE__);
                return ERROR;  
            }
        }
        /* Discard the descriptors */
        
        /*
        draw_oriented_keypoint_1(ip, row, col, scale, orientation, 0, 0, 200);
        */
        /*draw_oriented_keypoint_1(ip, col, row, scale, orientation, 0, 0, 200);*/
        draw_oriented_keypoint_1(ip, col, row, scale, orientation, fs_kpt_red_val, fs_kpt_green_val, fs_kpt_blue_val );
        items_read=fscanf(fp, "%f %f %f %f", &row, &col, &scale, &orientation);
    }
  
    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               draw_vl_keypoints_from_file
 * 
 * Reads in keypoint vectors
 *
 * The function reads-in the keypoint vector values (location, scale 
 * and orientation) from a file and draws oriented keypoints on 
 * the provided image.
 *
 * Input parameters: 
 *  KJB_image* ip - a handle to the resulting image
 *  char* keypoint_filename - a file with the keypoints in the VLFeat 
 *                            format (each line contains x,y, scale, 
 *                            orientation, followed by the ddescriptor)
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int draw_vl_keypoints_from_file (KJB_image* ip, char *keypoint_filename)
{
    int items_read = NOT_SET; /* a flag indicating if fscanf was successful */
    float row, col, scale, orientation;
    int j;
    FILE *fp = NULL;
    int tmp; /* used to read-in the descriptor values */
    
    NRE(fp = kjb_fopen(keypoint_filename, "r"));
  
    /* TODO: call fp_draw_vl_keypoints() */
    
    items_read = fscanf(fp, "%f %f %f %f", &row, &col, &scale, &orientation);
    while (items_read > 0)
    {
        /* Read in all the SIFT keypoints from the file*/
        if (items_read != 4)
        {
            add_error("ERROR (%s +%d): fscanf did not read position, scale, "
                      "and orientation.", __FILE__, __LINE__);
            return ERROR;
        } 
    
        /* Read in all the descriptor values */
        /* TODO: replace with reading a vector */
        for (j = 0; j < KEYPOINT_DESCRIP_LENGTH ; j++)
        {
            items_read = fscanf(fp, "%d", &tmp);
            if (items_read != 1)
            {
                add_error("ERROR (%s +%d): fscanf could not read the "
                          "descriptor value.", __FILE__, __LINE__);
                return ERROR;  
            }
        }
        /* Discard the descriptors */
        
        /*draw_oriented_keypoint_1 ( ip, row, col, scale, orientation, 0, 0, 200 ) ;*/
        draw_oriented_keypoint_1 ( ip, row, col, scale, orientation, fs_kpt_red_val, fs_kpt_green_val, fs_kpt_blue_val ) ;
        items_read = fscanf(fp, "%f %f %f %f", &row, &col, &scale, &orientation);
    }
  
      
  /*    ERE(kjb_display_image(ip, NULL)); */
    ERE(kjb_fclose(fp));
    
    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               draw_vl_keypoints_from_keypoint_vector
 * 
 * Reads in keypoints, draws them.
 *
 * The function reads-in the values from a keypoint vector and draws oriented 
 * keypoints on the provided image.
 *
 * Input parameters: 
 *  KJB_image* ip - a handle to the image
 *  Keypoint_vector* keypoint_kvp - the keypoints vector
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int draw_keypoints_from_keypoint_vector
(
  KJB_image* ip, 
  const Keypoint_vector* keypoint_kvp
)
{
  int j;
  int length;

  if (ip == NULL) return NO_ERROR;
  
  if (keypoint_kvp == NULL) return NO_ERROR;

  length = keypoint_kvp->length;

  for (j=0; j < length; j++)
  {
      ERE( draw_oriented_keypoint_1 ( ip, keypoint_kvp->elements[j]->row, 
                                      keypoint_kvp->elements[j]->col, 
                                      keypoint_kvp->elements[j]->scale, 
                                      keypoint_kvp->elements[j]->ori, 
                                      fs_kpt_red_val, fs_kpt_green_val, fs_kpt_blue_val ) );
                                      /*0, 0, 200 ) );*/
  }

  return NO_ERROR;
}

/* =============================================================================
 *               draw_vl_keypoint_vector_with_mask
 * 
 * Reads in keypoints, draws the keypoints that have non-zero value in the mask.
 *
 * The function reads-in the keypoint and mask values from the corresponding 
 * vectors and draws only the keypoints, whose indices in the mask had non-zero
 * values.
 * The function is based on draw_keypoints_from_keypoint_vector().
 *
 * Input parameters: 
 *  KJB_image* ip - a handle to the image
 *  Keypoint_vector* keypoint_kvp - the keypoints vector
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int draw_vl_keypoint_vector_with_mask
(
  KJB_image*                ip, 
  const Keypoint_vector*    keypoint_kvp,
  const Int_vector*         mask_ivp
)
{
  int j;
  int length;

  if (ip == NULL) return NO_ERROR;
  if (keypoint_kvp == NULL) return NO_ERROR;
  if (mask_ivp == NULL) return NO_ERROR;

  length = keypoint_kvp->length;
  if (length != mask_ivp->length)
  {
      warn_pso("Vector dimensions must match! %d != %d\n", length, mask_ivp->length);
      return ERROR;
  }

  for (j=0; j < length; j++)
  {
      if (mask_ivp->elements[j] >= 0)
      {
          ERE( draw_oriented_keypoint_1 ( ip, keypoint_kvp->elements[j]->row, 
                                          keypoint_kvp->elements[j]->col, 
                                          keypoint_kvp->elements[j]->scale, 
                                          keypoint_kvp->elements[j]->ori, 
                                          fs_kpt_red_val, fs_kpt_green_val, fs_kpt_blue_val ) );
          /*0, 0, 200 ) );*/
      }
  }

  return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               draw_vl_keypoint_vector_with_mask_value
 * 
 * Reads in keypoints, draws the keypoints that match the specified value in 
 * the mask.
 *
 * The function reads-in the keypoint and mask values from the corresponding 
 * vectors and draws only the keypoints, whose indices in the mask match the 
 * specified value.
 * The function is based on draw_keypoints_from_keypoint_vector().
 *
 * Input parameters: 
 *  KJB_image* ip - a handle to the image
 *  Keypoint_vector* keypoint_kvp - the keypoints vector
 *  Int_vector* mask_ivp - mask values
 *  int val - the value, which if matched, results in drawing the keypoint at
 *  the corresponding index
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int draw_vl_keypoint_vector_with_mask_value
(
  KJB_image*                ip, 
  const Keypoint_vector*    keypoint_kvp,
  const Int_vector*         mask_ivp,
  const int                 val
)
{
  int j;
  int length;

  if (ip == NULL) return NO_ERROR;
  if (keypoint_kvp == NULL) return NO_ERROR;
  if (mask_ivp == NULL) return NO_ERROR;

  length = keypoint_kvp->length;
  if (length != mask_ivp->length)
  {
      warn_pso("Vector dimensions must match! %d != %d\n", length, mask_ivp->length);
      return ERROR;
  }

  for (j=0; j < length; j++)
  {
      if (mask_ivp->elements[j] == val)
      {
          ERE( draw_oriented_keypoint_1 ( ip, keypoint_kvp->elements[j]->row, 
                                          keypoint_kvp->elements[j]->col, 
                                          keypoint_kvp->elements[j]->scale, 
                                          keypoint_kvp->elements[j]->ori, 
                                          fs_kpt_red_val, fs_kpt_green_val, fs_kpt_blue_val ) );
          /*0, 0, 200 ) );*/
      }
  }

  return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               draw_keypoint_correspondences
 * 
 * Visualize a correspondence
 *
 * The function draws the two images one underneath the other 
 * and connects via lines the locations of the corresponing keypoints
 * provided in the Keypoint vectors. 
 * img1_kvp->elements[0] is assumed to match img2_kvp->elements[0], etc.
 *
 * Input parameters: 
 *  KJB_image* img1_ip - a handle to the image
 *  KJB_image* img2_ip - a handle to another image
 *  KJB_image** result_ip - a handle to the resulting image
 *  Keypoint_vector* img1_kvp - the keypoints for img1 
 *  Keypoint_vector* img2_kvp - the keypoints for img2 
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *  An ERROR is also returned if the Keypoint vectors differ
 *  in size.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int draw_keypoint_correspondences
(
    const KJB_image           *img1_ip,
    const KJB_image           *img2_ip,
    const Keypoint_vector     *img1_kvp,
    const Keypoint_vector     *img2_kvp,
    KJB_image                 **result_ip
)
{
    int i;
    int num_rows = 0;
    int num_cols = 0;
    int num_matches = 0;

    if (   (img1_ip == NULL || img2_ip == NULL || result_ip == NULL)
        || (img1_kvp == NULL || img2_kvp == NULL)
    )
    {
        return NO_ERROR;
    }

    if (img1_kvp->length != img2_kvp->length)
    {
        add_error("ERROR (%s +%d): Matched keypoint vectors have different dimensions:", __FILE__, __LINE__);
        add_error("img1_kvp->length = %d", img1_kvp->length);
        add_error("img2_kvp->length = %d", img2_kvp->length);

        /* SET_ARGUMENT_BUG(); */
        return ERROR;
    }

    num_rows = img1_ip->num_rows + img2_ip->num_rows;
    num_cols = MAX_OF(img1_ip->num_cols, img2_ip->num_cols);
    ERE( get_zero_image( result_ip, (num_rows+fs_image_gap), num_cols ) ); 
    ERE( image_draw_image( *result_ip, img1_ip, 0, 0, 1) ); 
    ERE( image_draw_image( *result_ip, img2_ip, (img1_ip->num_rows)+fs_image_gap, 0, 1)); 
       
    num_matches = img1_kvp->length;
    verbose_pso(7, " | Drawing %d matches.\n", num_matches);
    /* Draw the connecting lines */
    for (i = 0; i < num_matches; i = (i + fs_kpt_draw_skip_step)) 
    {
        ERE( image_draw_segment_2( *result_ip,
                                   (img1_kvp->elements[i])->col , (img1_kvp->elements[i])->row , 
                                   ((img2_kvp->elements[i])->col + img1_ip->num_rows) + fs_image_gap , (img2_kvp->elements[i])->row, 
                                 /*1, 0, 255, 0  ) );*/
                                 LINE_WIDTH, 0, 255, 0  ) );
    }
    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               draw_keypoint_matches
 *
 * Visualize matching keypoints
 *
 * The function draws the two images one underneath the other or one
 * on top of another, depending on the value of the "draw-side-by-side"
 * option. By default, matched frames are drawn one on top of the other
 * (i.e. the option is FALSE by default).
 * The function connects via lines the locations of the corresponing keypoints
 * provided in the Keypoint vectors. The matches are provided via the
 * match_imp: the first column indexes the keypoints from the first
 * image, the second column indexes the keypoints from the second image.
 * Thus, if match_imp->elements[0][0] is 42 and match_imp->elements[0][1] 
 * is 11, then the keypoint 42 from the first image is matched to the 
 * 11th keypoint from the second image.
 *
 * Input parameters: 
 *  KJB_image* img1_ip - a handle to the image
 *  KJB_image* img2_ip - a handle to another image
 *  Keypoint_vector* img1_kvp - the keypoints for img1 
 *  Keypoint_vector* img2_kvp - the keypoints for img2 
 *  Int_matrix* match_imp - the keypoint matches
 *  KJB_image** result_ip - a handle to the resulting image
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int draw_keypoint_matches
(
    const KJB_image           *img1_ip,
    const KJB_image           *img2_ip,
    const Keypoint_vector     *img1_kvp,
    const Keypoint_vector     *img2_kvp,
    const Int_matrix          *match_imp,
    KJB_image                 **result_ip
)
{
    int i;
    int m1; /* Index of the match from the first image */
    int m2; /* Index of the match from the second image */
    int num_rows = 0;
    int num_cols = 0;
    int num_matches = 0;
    /*int red, green, blue;*/
    Vector *colors = NULL;

    if (   (img1_ip == NULL || img2_ip == NULL || result_ip == NULL)
        || (img1_kvp == NULL || img2_kvp == NULL || match_imp == NULL)
    )
    {
        return NO_ERROR;
    }


/*    num_matches = img1_kvp->length;*/
    num_matches = match_imp->num_rows;
    if (num_matches == 0)
    {
        verbose_pso(8, " There are no matches to draw.\n");
        /*return NO_ERROR;*/
    }

    if (fs_draw_side_by_side == TRUE)
    {
        num_rows = MAX_OF(img1_ip->num_rows, img2_ip->num_rows);
        num_cols = (img1_ip->num_cols + img2_ip->num_cols);
        ERE( get_zero_image( result_ip, num_rows, num_cols + fs_image_gap ) );
        ERE( image_draw_image( *result_ip, img1_ip, 0, 0, 1) ); 
        ERE( image_draw_image( *result_ip, img2_ip, 0, img1_ip->num_cols + fs_image_gap, 1)); 

        /* Draw the connecting lines */
        for (i = 0; i < num_matches; i++ ) 
        {
            m1 = match_imp->elements[i][0];
            m2 = match_imp->elements[i][1];
            if (m1 >= 0 && m2 >= 0) 
            {
                if (m1 < img1_kvp->length && m2 < img2_kvp->length)
                {
                    ERE( get_random_vector( &colors, 3) );
                    /* verbose_pso(7, "Color (%f %f %f)\n",   255*(colors->elements[0]),
                                                255*(colors->elements[1]),
                                                255*(colors->elements[2])); */


                    ERE( image_draw_segment_2( *result_ip,
                                               (img1_kvp->elements[m1])->col, 
                                               (img1_kvp->elements[m1])->row, 
                                               (img2_kvp->elements[m2])->col, 
                                               (img2_kvp->elements[m2])->row + img1_ip->num_cols + fs_image_gap, 
                                               /*1, 0, 255, 0 ) );*/
                                                LINE_WIDTH,
                                                255*(colors->elements[0]),
                                                255*(colors->elements[1]),
                                                255*(colors->elements[2])
                                                ) );
                }
                else
                {
                    add_error("ERROR (%s +%d): Match index is out of bounds.", __FILE__, __LINE__);
                    add_error("Either %d => %d or %d => %d.", m1, img1_kvp->length, m2, img2_kvp->length );
                    return ERROR;
                }
                i = (i + fs_kpt_draw_skip_step) ;
            }
        }    
    }
    else /* draw on top of one another */
    {
        num_rows = (img1_ip->num_rows + img2_ip->num_rows);
        num_cols = MAX_OF(img1_ip->num_cols, img2_ip->num_cols);
        ERE( get_zero_image( result_ip, num_rows, num_cols+fs_image_gap ) );
        ERE( image_draw_image( *result_ip, img1_ip, 0, 0, 1) ); 
        ERE( image_draw_image( *result_ip, img2_ip, img1_ip->num_rows+fs_image_gap, 0, 1)); 

        /* Draw the connecting lines */
        for (i = 0; i < num_matches; i++ ) 
        {
            m1 = match_imp->elements[i][0];
            m2 = match_imp->elements[i][1];
            if (m1 >= 0 && m2 >= 0) 
            {
                if (m1 < img1_kvp->length && m2 < img2_kvp->length)
                {
                    
                    ERE( get_random_vector( &colors, 3) );
                    /*verbose_pso(7, "Color (%f %f %f)\n",   255*(colors->elements[0]),
                                                255*(colors->elements[1]),
                                                255*(colors->elements[2]));
*/

                    ERE( image_draw_segment_2( *result_ip,
                                               (img1_kvp->elements[m1])->col, 
                                               (img1_kvp->elements[m1])->row, 
                                               (img2_kvp->elements[m2])->col + img1_ip->num_rows + fs_image_gap, 
                                               (img2_kvp->elements[m2])->row, 
                                               LINE_WIDTH,
 /*1, 0, 255, 0 ) );*/
                                                255*(colors->elements[0]),
                                                255*(colors->elements[1]),
                                                255*(colors->elements[2])
                                               ) );

                }
                else
                {
                    add_error("ERROR (%s +%d): Match index is out of bounds.", __FILE__, __LINE__);
                    add_error("Either %d => %d or %d => %d.", m1, img1_kvp->length, m2, img2_kvp->length );
                    return ERROR;
                }
                i = (i + fs_kpt_draw_skip_step) ;
            }
        } 
    }

    free_vector( colors );
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *               draw_keypoint_matches_1
 *
 * Visualize matching keypoints using match vector.
 *
 * The function draws the two images one underneath the other or one
 * on top of another, depending on the value of the "draw-side-by-side"
 * option. By default, matched frames are drawn one on top of the other
 * (i.e. the option is FALSE by default).
 * The function connects via lines the locations of the corresponing keypoints
 * provided in the Keypoint vectors. The matches are provided via the
 * match_ivp: the index of the vector indexes the keypoints from the first
 * image, the value at that index indexes the keypoints from the second image.
 * The length of match_ivp is the same as the length of img1_kvp.
 * Thus, if match_ivp->elements[0] is 42, then the keypoint 42 from the second 
 * image (img2_kvp) is matched to the 1st keypoint from the first image (img1_kvp).
 * NOT_SET is a value for the keypoints that weren't matched.
 *
 * Input parameters: 
 *  KJB_image* img1_ip - a handle to the image
 *  KJB_image* img2_ip - a handle to another image
 *  Keypoint_vector* img1_kvp - the keypoints for img1 
 *  Keypoint_vector* img2_kvp - the keypoints for img2 
 *  Int_vector* match_ivp - each element, i, stores indices of img2_kvp that match
 *                          the i-th element of img1_kvp
 *  KJB_image** result_ip - a handle to the resulting image
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int draw_keypoint_matches_1
(
    const KJB_image           *img1_ip,
    const KJB_image           *img2_ip,
    const Keypoint_vector     *img1_kvp,
    const Keypoint_vector     *img2_kvp,
    const Int_vector          *match_ivp,
    KJB_image                 **result_ip
)
{
    int i;
    int m1; /* Index of the match from the first image */
    int m2; /* Index of the match from the second image */
    int num_rows = 0;
    int num_cols = 0;
    int num_matches = 0;

    if (   (img1_ip == NULL || img2_ip == NULL || result_ip == NULL)
        || (img1_kvp == NULL || img2_kvp == NULL || match_ivp == NULL)
    )
    {
        return NO_ERROR;
    }

    num_matches = match_ivp->length;
    if (num_matches == 0)
    {
        warn_pso(" There are no matches to draw.\n");
        return NO_ERROR;
    }

    if (fs_draw_side_by_side == TRUE)
    {
        num_rows = MAX_OF(img1_ip->num_rows, img2_ip->num_rows);
        num_cols = (img1_ip->num_cols + img2_ip->num_cols);
        ERE( get_zero_image( result_ip, num_rows, num_cols + fs_image_gap ) );
        ERE( image_draw_image( *result_ip, img1_ip, 0, 0, 1) ); 
        ERE( image_draw_image( *result_ip, img2_ip, 0, img1_ip->num_cols + fs_image_gap, 1)); 

        /* Draw the connecting lines */
        for (i = 0; i < num_matches; i++ ) 
        {
            m1 = i;
            m2 = match_ivp->elements[i];
            if (m1 >= 0 && m2 >= 0) 
            {
                if (m1 < img1_kvp->length && m2 < img2_kvp->length)
                {
                    ERE( image_draw_segment_2( *result_ip,
                                               (img1_kvp->elements[m1])->col, 
                                               (img1_kvp->elements[m1])->row, 
                                               (img2_kvp->elements[m2])->col, 
                                               (img2_kvp->elements[m2])->row + img1_ip->num_cols + fs_image_gap, 
                                               /*1, 0, 255, 0 ) );*/
                                               LINE_WIDTH, 0, 255, 0 ) );
                }
                else
                {
                    add_error("ERROR (%s +%d): Match index is out of bounds.", __FILE__, __LINE__);
                    add_error("Either %d => %d or %d => %d.", m1, img1_kvp->length, m2, img2_kvp->length );
                    return ERROR;
                }
                i = (i + fs_kpt_draw_skip_step) ;
            }
        }    
    }
    else /* draw on top of one another */
    {
        num_rows = (img1_ip->num_rows + img2_ip->num_rows);
        num_cols = MAX_OF(img1_ip->num_cols, img2_ip->num_cols);
        ERE( get_zero_image( result_ip, num_rows, num_cols+fs_image_gap ) );
        ERE( image_draw_image( *result_ip, img1_ip, 0, 0, 1) ); 
        ERE( image_draw_image( *result_ip, img2_ip, img1_ip->num_rows+fs_image_gap, 0, 1)); 

        /* Draw the connecting lines */
        for (i = 0; i < num_matches; i++ ) 
        {
            m1 = i;
            m2 = match_ivp->elements[i];
            if (m1 >= 0 && m2 >= 0) 
            {
                if (m1 < img1_kvp->length && m2 < img2_kvp->length)
                {
                    if ( fs_different_color_lines == FALSE )
                    {
                        ERE( image_draw_segment_2( *result_ip,
                                                   (img1_kvp->elements[m1])->col, 
                                                   (img1_kvp->elements[m1])->row, 
                                                   (img2_kvp->elements[m2])->col + img1_ip->num_rows + fs_image_gap, 
                                                   (img2_kvp->elements[m2])->row, 
                                                   /*1, 0, 255, 0 ) );*/
                                               LINE_WIDTH, 0, 255, 0 ) );
                    }
                    else
                    {
                        double red = 255.0 * kjb_rand();
                        double green = 255.0 * kjb_rand();
                        double blue = 255.0 * kjb_rand();
                        ERE( image_draw_segment_2( *result_ip,
                                                   (img1_kvp->elements[m1])->col, 
                                                   (img1_kvp->elements[m1])->row, 
                                                   (img2_kvp->elements[m2])->col + img1_ip->num_rows + fs_image_gap, 
                                                   (img2_kvp->elements[m2])->row, 
                                                   LINE_WIDTH, red, green, blue ) );
                    }
                }
                else
                {
                    add_error("ERROR (%s +%d): Match index is out of bounds.", __FILE__, __LINE__);
                    add_error("Either %d => %d or %d => %d.", m1, img1_kvp->length, m2, img2_kvp->length );
                    return ERROR;
                }
                i = (i + fs_kpt_draw_skip_step) ;
            }
        } 
    }

    return NO_ERROR;
}


/* =============================================================================
 *               draw_ransac_matches
 *
 * Visualize RANSAC matches using match vector.
 *
 * The function draws the two images one underneath the other or one
 * on top of another, depending on the value of the "draw-side-by-side"
 * option. By default, matched frames are drawn one on top of the other
 * (i.e. the option is FALSE by default).
 * The function connects via lines the locations of the corresponing keypoints
 * provided in the matrices. The matches are provided via the
 * match_ivp: the values of the vector indicated the index of the location
 * (img1_mp->elements[0] matches img2_mp->elements[0] and so on).
 * The length of match_ivp is the same as the length of img1_mp and img2_mp.
 *
 * Input parameters: 
 *  KJB_image* img1_ip - a handle to the image
 *  KJB_image* img2_ip - a handle to another image
 *  Keypoint_vector* img1_mp - the locations of the keypoints for img1 
 *  Keypoint_vector* img2_mp - the locations of the keypoints for img2
 *  Int_vector* match_ivp - the keypoint matches
 *  KJB_image** result_ip - a handle to the resulting image
 *
 * Returns:
 *  NO_ERROR or an error set by the image_draw routines.
 *
 * Index: keypoints
 *
 * -----------------------------------------------------------------------------
*/
int draw_ransac_matches
(
    const KJB_image  *img1_ip,
    const KJB_image  *img2_ip,
    const Matrix     *img1_mp,
    const Matrix     *img2_mp,
    const Int_vector *match_ivp,
    KJB_image        **result_ip
)
{
    int i;
    int idx; /* Index of the match  */
    int num_rows = 0;
    int num_cols = 0;
    int num_matches = 0;

    if (   (img1_ip == NULL || img2_ip == NULL || result_ip == NULL)
        /*|| (img1_kvp == NULL || img2_kvp == NULL || match_ivp == NULL)*/
        || (img1_mp == NULL || img2_mp == NULL || match_ivp == NULL)
    )
    {
        return NO_ERROR;
    }

    num_matches = match_ivp->length;
    if (num_matches == 0)
    {
        verbose_pso(8, " There are no matches to draw.\n");
        return NO_ERROR;
    }

    if (fs_draw_side_by_side == TRUE)
    {
        num_rows = MAX_OF(img1_ip->num_rows, img2_ip->num_rows);
        num_cols = (img1_ip->num_cols + img2_ip->num_cols);
        ERE( get_zero_image( result_ip, num_rows, num_cols + fs_image_gap ) );
        ERE( image_draw_image( *result_ip, img1_ip, 0, 0, 1) ); 
        ERE( image_draw_image( *result_ip, img2_ip, 0, img1_ip->num_cols + fs_image_gap, 1)); 

        /* Draw the connecting lines */
        for (i = 0; i < num_matches; i++ ) 
        {
            idx = match_ivp->elements[i];
                            ERE( image_draw_segment_2( *result_ip,
                                                       /*
                                               (img1_mp->elements[idx][0]), 
                                               (img1_mp->elements[idx][1]), 
                                               (img2_mp->elements[idx][0]), 
                                               (img2_mp->elements[idx][1]) + img1_ip->num_cols + fs_image_gap, 
                                               */
                                               (img1_mp->elements[idx][1]), 
                                               (img1_mp->elements[idx][0]), 
                                               (img2_mp->elements[idx][1]), 
                                               (img2_mp->elements[idx][0]) + img1_ip->num_cols + fs_image_gap, 
                                               LINE_WIDTH, 0, 255, 0 ) );
                                               /*
                                               (img1_mp->elements[idx])->col, 
                                               (img1_kvp->elements[m1])->row, 
                                               (img2_kvp->elements[m2])->col, 
                                               (img2_kvp->elements[m2])->row + img1_ip->num_cols + fs_image_gap, 
                                               1, 0, 255, 0 ) );
                                               */
                              i = (i + fs_kpt_draw_skip_step) ;
        }    
    }
    else /* draw on top of one another */
    {
        num_rows = (img1_ip->num_rows + img2_ip->num_rows);
        num_cols = MAX_OF(img1_ip->num_cols, img2_ip->num_cols);
        ERE( get_zero_image( result_ip, num_rows, num_cols+fs_image_gap ) );
        ERE( image_draw_image( *result_ip, img1_ip, 0, 0, 1) ); 
        ERE( image_draw_image( *result_ip, img2_ip, img1_ip->num_rows+fs_image_gap, 0, 1)); 

        /* Draw the connecting lines */
        for (i = 0; i < num_matches; i++ ) 
        {
            idx = match_ivp->elements[i];

                ERE( image_draw_segment_2( *result_ip,
                                           /*
                                               (img1_mp->elements[idx][0]), 
                                               (img1_mp->elements[idx][1]), 
                                               (img2_mp->elements[idx][0])+ img1_ip->num_rows + fs_image_gap, 
                                               (img2_mp->elements[idx][1]),
                                               */
                                               (img1_mp->elements[idx][1]), 
                                               (img1_mp->elements[idx][0]), 
                                               (img2_mp->elements[idx][1])+ img1_ip->num_rows + fs_image_gap, 
                                               (img2_mp->elements[idx][0]),
                                               LINE_WIDTH, 0, 255, 0 ) );

                i = (i + fs_kpt_draw_skip_step) ;
        } 
    }

    return NO_ERROR;
}

int set_keypoint_color
(
    const char* red,
    const char* green,
    const char* blue
)
{
    int result = NO_ERROR;

    verbose_pso(11, "Setting keypoint RGB to (%s, %s, %s)\n", red, green, blue);
    ERE(set_keypoint_options("kpt-red-val", (red)));
    ERE(set_keypoint_options("kpt-green-val", (green)));
    ERE(set_keypoint_options("kpt-blue-val", (blue)));

    return result;
}




/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif
