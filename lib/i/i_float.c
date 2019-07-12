
/* $Id: i_float.c 20918 2016-10-31 22:08:27Z kobus $ */

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
#include "i/i_float.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION

#   define allocate_2D_float_pixel_array(x, y) \
            debug_allocate_2D_float_pixel_array(x, y, __FILE__, __LINE__)

    static Pixel** debug_allocate_2D_float_pixel_array
    (
        int         num_rows,
        int         num_cols,
        const char* file_name,
        int         line_number
    );
#else
    static Pixel** allocate_2D_float_pixel_array
    (
        int num_rows,
        int num_cols
    );
#endif

static void free_2D_float_pixel_array(Pixel** pixel_array);

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                              kjb_create_image
 *
 * Creates a float image
 *
 * This routine creates a float image of the size specified by the arguments
 * "num_rows" and "num_cols". Images are disposed of with the routine
 * "kjb_free_image"
 *
 * Note:
 *     Usually the routine get_target_image is used to create images
 *     instead of this routine.
 *
 * Returns:
 *     A pointer to the new image is returned on success. If the creation fails,
 *     NULL is returned with an appropriate error message being set.
 *
 * Index: images, float images
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

KJB_image* debug_create_image
(
    int         num_rows,
    int         num_cols,
    const char* file_name,
    int         line_number
)
{
    KJB_image* ip;
    int i; 


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    /* Prevent nx0 or 0xn image */
    if(num_rows*num_cols == 0 && num_rows + num_cols != 0)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(ip = DEBUG_TYPE_MALLOC(KJB_image, file_name, line_number));

    if(num_rows == 0 && num_cols == 0)
    {
        ip->pixels = NULL;
    }
    else
    {
        ip->pixels = debug_allocate_2D_float_pixel_array(num_rows, num_cols,
                                                         file_name, line_number);

        if (ip->pixels == NULL)
        {
            kjb_free(ip);
            return NULL;
        }
    }

    ip->num_rows = num_rows;
    ip->num_cols = num_cols;

    for (i=0; i<num_rows; i++)
    {
        Pixel* row_pos = ip->pixels[ i ];
        int    j; 

        for (j=0; j<num_cols; j++)
        {
            row_pos->extra.alpha = 0.0;
            row_pos->extra.invalid.pixel = VALID_PIXEL;
            row_pos->extra.invalid.r = VALID_PIXEL;
            row_pos->extra.invalid.b = VALID_PIXEL;
            row_pos->extra.invalid.b = VALID_PIXEL;

            row_pos++;
        }
    }

    ip->read_only = FALSE;
    ip->flags = 0;

    return ip;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

KJB_image* kjb_create_image(int num_rows, int num_cols)
{
    KJB_image* ip;
    int i;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    /* Prevent nx0 or 0xn image */
    if(num_rows*num_cols == 0 && num_rows + num_cols != 0)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    NRN(ip = TYPE_MALLOC(KJB_image));

    if(num_rows == 0 && num_cols == 0)
    {
        ip->pixels = NULL;
    }
    else
    {
        ip->pixels = allocate_2D_float_pixel_array(num_rows, num_cols);

        if (ip->pixels == NULL)
        {
            kjb_free(ip);
            return NULL;
        }
    }

    for (i=0; i<num_rows; i++)
    {
        Pixel* row_pos = ip->pixels[ i ];
        int j; 

        for (j=0; j<num_cols; j++)
        {
            row_pos->extra.alpha = 0.0;
            row_pos->extra.invalid.pixel = VALID_PIXEL;
            row_pos->extra.invalid.r = VALID_PIXEL;
            row_pos->extra.invalid.b = VALID_PIXEL;
            row_pos->extra.invalid.b = VALID_PIXEL;

            row_pos++;
        }
    }

    ip->num_rows = num_rows;
    ip->num_cols = num_cols;

    ip->read_only = FALSE;
    ip->flags = 0;

    return ip;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         check_image_initialization
 *
 * Checks tha all pixels in an image have been set
 *
 * This routine checks tha all pixels in an image have been set. If this is not
 * the case, this is treated as bug, typically leading to an abort (depending on
 * the bug handler). 
 *
 * This routine is only available when we are checking the heap. In "production"
 * code, is is #define'd as a NOP. 
 *
 * Returns:
 *    This routine does not return any value. 
 *
 * Index:
 *    debugging, memory checking, images, float images  
 *
 * -----------------------------------------------------------------------------
*/

#ifdef TRACK_MEMORY_ALLOCATION

void check_image_initialization(const KJB_image* ip)
{

    if (ip != NULL)
    {
        size_t blocks_per_pixel = sizeof(Pixel) / sizeof(float);

        /*
         * We must have a valid pointer to check the initialization,
         * otherwise problems such as double free can look like unitialized
         * memory.
        */
        kjb_check_free(ip);

        check_initialization(ip->pixels[ 0 ],
                             ip->num_cols * ip->num_rows * blocks_per_pixel,
                             sizeof(float));
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
void debug_verify_image
(
    const KJB_image* ip,
    const char*   file_name,
    int           line_number
)
{
    int i,j, num_rows, num_cols;

    if (ip == NULL) return;

    check_image_initialization(ip);

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (    (isnanf(ip->pixels[ i ][ j ].r))
                 || (isnanf(ip->pixels[ i ][ j ].g))
                 || (isnanf(ip->pixels[ i ][ j ].b))
               )
            {
                p_stderr("Image verification failed.\n");
                p_stderr("Element (%d, %d) has a NaN value for one of (R,G,B)\n", i, j);
                p_stderr("Verification called from line %d of %s.\n",
                         line_number, file_name);

                SET_NAN_FLOAT_BUG();
                kjb_exit(EXIT_FAILURE);
            }
            else if (    ( ! isfinitef(ip->pixels[ i ][ j ].r))
                      || ( ! isfinitef(ip->pixels[ i ][ j ].g))
                      || ( ! isfinitef(ip->pixels[ i ][ j ].b))
                    )
            {
                p_stderr("Image verification failed.\n");
                p_stderr("Element (%d, %d) has infinite value for one of (R,G,B)\n", i, j);
                p_stderr("Verification called from line %d of %s.\n",
                         line_number, file_name);

                SET_INFINITE_FLOAT_BUG();
                kjb_exit(EXIT_FAILURE);
            }
        }
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                              kjb_free_image
 *
 * Disposes of images
 *
 * This routine disposes of images such as those created by get_target_image or
 * create image (often indirectly).
 *
 * Index: images, float images
 *
 * -----------------------------------------------------------------------------
*/

void kjb_free_image(KJB_image* ip)
{

    if (ip != NULL)
    {
        if (ip->pixels != NULL)
        {
#ifdef TRACK_MEMORY_ALLOCATION
            size_t blocks_per_pixel = sizeof(Pixel) / sizeof(float);
#endif

            if (ip->read_only)
            {
#ifdef TEST
                set_bug("Attempt to free read only float image.");
#endif
                return;
            }

#ifdef TRACK_MEMORY_ALLOCATION
            /*
             * We must have a valid pointer to check the initialization,
             * otherwise problems such as double free can look like unitialized
             * memory.
            */
            kjb_check_free(ip);

            check_initialization(ip->pixels[ 0 ],
                                 ip->num_cols * ip->num_rows * blocks_per_pixel,
                                 sizeof(float));
#endif

            free_2D_float_pixel_array(ip->pixels);
        }

        kjb_free(ip);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             kjb_copy_image
 *
 * Copies and image
 *
 * This routine copies the float image pointed to by the input parameter
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

int kjb_copy_image(KJB_image** target_ipp, const KJB_image* source_ip)
{
    IMPORT int kjb_use_memcpy;
    KJB_image*  target_ip;
    int           num_rows;
    int           num_cols;
    int           i;
    int           j;
    Pixel*  source_image_row_pos;
    Pixel*  target_image_row_pos;


    if (source_ip == NULL)
    {
        kjb_free_image(*target_ipp);
        *target_ipp = NULL;
        return NO_ERROR;
    }

    num_rows = source_ip->num_rows;
    num_cols = source_ip->num_cols;

    ERE(get_target_image(target_ipp, num_rows, num_cols));

    if(num_rows == 0 || num_cols == 0) 
    {
        return NO_ERROR;
    }

    if (kjb_use_memcpy)
    {
        /* If we add resize capability, see the code for Matrix */

        (void)memcpy((*target_ipp)->pixels[ 0 ],
                     source_ip->pixels[ 0 ],
                     ((size_t)num_rows * num_cols) * sizeof(Pixel));
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
 *                             get_image_window
 *
 * Extracts a window from an image
 *
 * This routine extracts a window from the float image pointed to by the input
 * parameter "source_ip" into *target_ipp which is created are reszied as
 * needed. The window is specified by its minimum col (arg "row_offset") and
 * minimum row (arg "col_offset") and its dimensions (args "num_target_rows" and
 * "num_target_cols").
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, basic image routines
 *
 * -----------------------------------------------------------------------------
*/

int get_image_window
(
    KJB_image**      target_ipp,
    const KJB_image* source_ip,
    int              row_offset,
    int              col_offset,
    int              num_target_rows,
    int              num_target_cols
)
{
    KJB_image* target_ip;
    int    num_source_rows;
    int    num_source_cols;
    int    i, j;
    Pixel* source_image_row_pos;
    Pixel* target_image_row_pos;


    num_source_rows = source_ip->num_rows;
    num_source_cols = source_ip->num_cols;

    if (    (num_source_rows < row_offset + num_target_rows)
         || (num_source_cols < col_offset + num_target_cols)
         || (row_offset < 0)
         || (col_offset < 0)
       )
    {
        ERE(get_initialized_image_2(target_ipp,
                                    num_target_rows, num_target_cols,
                                    128, 128, 128));
    }
    else
    {
        ERE(get_target_image(target_ipp, num_target_rows, num_target_cols));
    }

    target_ip = *target_ipp;

    verbose_pso(20, "Image window top left corner is %d, %d.\n",
                col_offset, row_offset);
    verbose_pso(20, "Image window bottom right corner is %d, %d.\n",
                col_offset + num_target_cols - 1,
                row_offset + num_target_rows - 1);

    for (i=0; i<num_target_rows; i++)
    {
        if ((row_offset + i) < 0) continue;
        if ((row_offset + i) >= num_source_rows) break;

        source_image_row_pos = source_ip->pixels[ row_offset + i ];
        source_image_row_pos += col_offset;

        target_image_row_pos = target_ip->pixels[ i ];

        for (j=0; j<num_target_cols; j++)
        {
            if ((col_offset + j) < 0) continue;
            if ((col_offset + j) >= num_source_cols) break;

            *target_image_row_pos = *source_image_row_pos;

            target_image_row_pos++;
            source_image_row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_zero_image
 *
 * Gets target float image initialized to zero
 *
 * This routine gets a target float image initialized to zero.  If
 * *target_ipp is NULL, then this routine creates the images. If it is not
 * null, and it is the right size, then this routine sets it to zero in place.
 * If it is the wrong size, then it is resized.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, float images
 *
 * -----------------------------------------------------------------------------
*/

int get_zero_image(KJB_image** target_ipp, int num_rows, int num_cols)
{
    KJB_image* target_ip;
    int    i, j;
    Pixel* row_pos;


    ERE(get_target_image(target_ipp, num_rows, num_cols));
    target_ip = *target_ipp;

    for (i=0; i<num_rows; i++)
    {
        row_pos = target_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            row_pos->extra.alpha = 0.0;
            row_pos->extra.invalid.pixel = VALID_PIXEL;
            row_pos->extra.invalid.r = VALID_PIXEL;
            row_pos->extra.invalid.b = VALID_PIXEL;
            row_pos->extra.invalid.b = VALID_PIXEL;
            row_pos->r = 0.0;
            row_pos->g = 0.0;
            row_pos->b = 0.0;

            row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_invalid_zero_image
 *
 * Gets target float image initialized to invalid zero pixels
 *
 * This routine gets a target float image initialized to invalid zero pixels.
 * If *target_ipp is NULL, then this routine creates the images. If it is not
 * null, and it is the right size, then this routine sets it to zero in place.
 * If it is the wrong size, then it is resized.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, float images
 *
 * -----------------------------------------------------------------------------
*/

int get_invalid_zero_image
(
    KJB_image** target_ipp,
    int         num_rows,
    int         num_cols
)
{
    KJB_image* target_ip;
    int    i, j;
    Pixel* row_pos;


    ERE(get_target_image(target_ipp, num_rows, num_cols));
    target_ip = *target_ipp;

    for (i=0; i<num_rows; i++)
    {
        row_pos = target_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            row_pos->extra.alpha = 0.0;
            row_pos->extra.invalid.pixel = INVALID_PIXEL;
            row_pos->extra.invalid.r = INVALID_PIXEL;
            row_pos->extra.invalid.b = INVALID_PIXEL;
            row_pos->extra.invalid.b = INVALID_PIXEL;
            row_pos->r = 0.0;
            row_pos->g = 0.0;
            row_pos->b = 0.0;

            row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_initialized_image
 *
 * Gets target float image initialized to specified value
 *
 * This routine gets a target float image initialized to the pixel value
 * provided.  If *target_ipp is NULL, then this routine creates the images.
 * If it is not null, and it is the right size, then this routine does nothing.
 * If it is the wrong size, then it is resized.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, float images
 *
 * -----------------------------------------------------------------------------
*/

int get_initialized_image
(
    KJB_image** target_ipp,
    int         num_rows,
    int         num_cols,
    Pixel*      initial_value_ptr
)
{
    KJB_image* target_ip;
    int    i, j;
    Pixel* row_pos;


    ERE(get_target_image(target_ipp, num_rows, num_cols));
    target_ip = *target_ipp;

    for (i=0; i<num_rows; i++)
    {
        row_pos = target_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            *row_pos = *initial_value_ptr;
            row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_initialized_image_2
 *
 * Gets target float image initialized to specified value
 *
 * This routine gets a target float image initialized to the R,G,B value
 * provided.  If *target_ipp is NULL, then this routine creates the images.
 * If it is not null, and it is the right size, then this routine does nothing.
 * If it is the wrong size, then it is resized.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, float images
 *
 * -----------------------------------------------------------------------------
*/

int get_initialized_image_2
(
    KJB_image** target_ipp,
    int         num_rows,
    int         num_cols,
    int         r,
    int         g,
    int         b
)
{
    KJB_image* target_ip;
    int    i, j;
    Pixel* row_pos;


    ERE(get_target_image(target_ipp, num_rows, num_cols));
    target_ip = *target_ipp;

    for (i=0; i<num_rows; i++)
    {
        row_pos = target_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            row_pos->extra.alpha = 0.0;
            row_pos->extra.invalid.pixel = VALID_PIXEL;
            row_pos->extra.invalid.r = VALID_PIXEL;
            row_pos->extra.invalid.b = VALID_PIXEL;
            row_pos->extra.invalid.b = VALID_PIXEL;
            row_pos->r = r;
            row_pos->g = g;
            row_pos->b = b;

            row_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          get_target_image
 *
 * Gets target float image for "building block" routines
 *
 * This routine implements the image creation/over-writing semantics used in
 * the KJB library in the case of Byte images. If *target_ipp is NULL, then
 * this routine creates the images. If it is not null, and it is the right
 * size, then this routine does nothing. If it is the wrong size, then it is
 * resized.
 *
 * In order to be memory efficient, we free before resizing. If the resizing
 * fails, then the original contents of the *target_ipp will be lost.
 * However, *target_ipp will be set to NULL, so it can be safely sent to
 * free_float_image(). Note that this is in fact the convention throughout the
 * KJB library--if destruction on failure is a problem (usually when
 * *target_ipp is global)--then work on a copy!
 *
 * The sizes (num_rows and num_cols) must both be nonnegative.
 * Conventionally, num_rows and num_cols are both positive;
 * however, it is also acceptable to have num_rows and num_cols both zero.
 * Anything else (such as a mix of positive and zero sizes) is regarded as a
 * bug in the calling program -- set_bug(3) is called and (if it returns)
 * ERROR is returned.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, float images
 *
 * -----------------------------------------------------------------------------
*/

/*
// FIX
//
// We need to upgrade this routine so that the storage can be recycled, but this
// is tricky because we have to fix both the array of row pointers.
//
// Actually, this only makes sense if we store the amount of storage as an extra
// variable. Then the storage gets bumped up to a max and stays there. If we
// simply downsize, then on a upsize we have to free anyway. Hence this is not
// as useful.
*/

#ifdef TRACK_MEMORY_ALLOCATION
int debug_get_target_image
(
    KJB_image** target_ipp,
    int         num_rows,
    int         num_cols,
    const char* file_name,
    int         line_number
)
{
    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    /* Prevent nx0 or 0xn image */
    if(num_rows*num_cols == 0 && num_rows + num_cols != 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (*target_ipp == NULL)
    {
        NRE(*target_ipp = debug_create_image(num_rows, num_cols,
                                                      file_name, line_number));
    }
    else
    {
        if ((*target_ipp)->read_only)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }

        /*
         * If we add resize capability, see the code for Matrix.
         * Don't forget to change any copy code that uses memcpy.
        */

        if (    ((*target_ipp)->num_rows != num_rows)
             || ((*target_ipp)->num_cols != num_cols))
        {
            kjb_free_image(*target_ipp);
            NRE(*target_ipp = debug_create_image(num_rows, num_cols,
                                                 file_name, line_number));
        }
    }
    return NO_ERROR;
}

        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

int get_target_image(KJB_image** target_ipp, int num_rows, int num_cols)
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
        NRE(*target_ipp = kjb_create_image(num_rows, num_cols));
    }
    else
    {
        if ((*target_ipp)->read_only)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }

        /*
         * If we add resize capability, see the code for Matrix.
         * Don't forget to change any copy code that uses memcpy. 
        */

        if (    ((*target_ipp)->num_rows != num_rows)
             || ((*target_ipp)->num_cols != num_cols))
        {
            kjb_free_image(*target_ipp);
            NRE(*target_ipp = kjb_create_image(num_rows, num_cols));
        }
    }

    return NO_ERROR;
}

#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static Pixel** debug_allocate_2D_float_pixel_array
(
    int         num_rows,
    int         num_cols,
    const char* file_name,
    int         line_number
)
{
    Pixel **pixel_array;
    int i;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }


    NRN(pixel_array = DEBUG_N_TYPE_MALLOC(Pixel*, num_rows, file_name,
                                          line_number));

    (pixel_array)[ 0 ] = DEBUG_N_TYPE_MALLOC(Pixel, num_rows*num_cols,
                                             file_name, line_number);

    if ((pixel_array)[ 0 ] == NULL)
    {
        kjb_free(pixel_array);
        return NULL;
    }

    for (i=0; i<num_rows; i++)
    {
        (pixel_array)[ i ] = (pixel_array)[ 0 ] + i*num_cols;
    }

    return pixel_array;
}


        /*  ==>                               /\                              */
        /*  ==>  Development code above       ||                              */
#else   /* ------------------------------------------------------------------ */
        /*  ==>  Production code below        ||                              */
        /*  ==>                               \/                              */

static Pixel** allocate_2D_float_pixel_array(int num_rows, int num_cols)
{
    Pixel **pixel_array;
    int i;


    if ((num_rows < 0) || (num_cols < 0))
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }
    else if (num_rows == 0)
    {
        return NULL;
    }

    NRN(pixel_array = N_TYPE_MALLOC(Pixel*, num_rows));

    (pixel_array)[ 0 ] = N_TYPE_MALLOC(Pixel, num_rows*num_cols);

    if ((pixel_array)[ 0 ] == NULL)
    {
        kjb_free(pixel_array);
        return NULL;
    }

    for (i=0; i<num_rows; i++)
    {
        (pixel_array)[ i ] = (pixel_array)[ 0 ] + i*num_cols;
    }

    return pixel_array;
}


#endif    /*   #ifdef TRACK_MEMORY_ALLOCATION ... #else ...   */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void free_2D_float_pixel_array(Pixel** pixel_array)
{

    if (pixel_array != NULL)
    {
        kjb_free(pixel_array[ 0 ]);
        kjb_free(pixel_array);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int is_black_and_white( const KJB_image * ip)
{
    int i = 0;
    int j = 0;
    double avg_diff = 0.0;
    for(i = 0; i < ip->num_rows; i++)
    {
        for(j = 0; j < ip->num_cols; j++)
        {
            avg_diff += fabs(ip->pixels[i][j].r - ip->pixels[i][j].g) + fabs(ip->pixels[i][j].r - ip->pixels[i][j].b);
        }
    }
    avg_diff /= (ip->num_cols * ip->num_rows);
    if(avg_diff <= FLT_EPSILON)
    {
        return 1;
    }
    return 0;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

