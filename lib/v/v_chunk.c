
/* $Id: v_chunk.c 5835 2010-05-02 21:55:15Z ksimek $ */

/*
     Copyright (c) 1994-2008 by Kobus Barnard (author).

     Personal and educational use of this code is granted, provided
     that this header is kept intact, and that the authorship is not
     misrepresented. Commercial use is not permitted.
*/

#include "v/v_gen.h"     /* Only safe as first include in a ".c" file. */
#include "v/v_chunk.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */

static int              fs_chunk_max_step             = 1;
static int              fs_chunk_rechunk_level        = 1;
static int              fs_chunk_connect_corners      = TRUE;
static int              fs_chunk_min_chunk_size       = 50;
static int              fs_chunk_min_rechunk_size     = 10;
static int              fs_cached_i_sum;
static int              fs_cached_j_sum;
static int              fs_cached_num_pixels;
static int              fs_cached_num_boundary_pixels;
static int              fs_marked_pixels_num_rows     = 0;
static int              fs_marked_pixels_num_cols     = 0;
static int**            fs_marked_pixels              = NULL;
static Image_chunk*     fs_cached_chunk_ptr           = NULL;
/* We play it a bit fast and loose to reduce stack use during recursion. */
static const KJB_image* fs_cached_const_ip;  /* Do NOT free this! */

/* -------------------------------------------------------------------------- */

static void grow_chunk(int i, int j);

static int get_target_image_chunk_info
(
    Image_chunk_info** image_chunk_info_ptr_ptr,
    int                num_chunks
);

static Image_chunk_info* create_image_chunk_info(int num_chunks);

static Image_chunk* create_image_chunk
(
    int num_pixels,
    int num_boundary_pixels
);

static void free_image_chunk_storage(void* chunk_ptr);

static void free_image_chunk(Image_chunk* chunk_ptr);



#ifdef TRACK_MEMORY_ALLOCATION
    static void free_allocated_static_data(void);
    static void prepare_memory_cleanup(void);
#endif

/* -------------------------------------------------------------------------- */

int set_chunk_options(const char* option, const char* value)
{
    char  lc_option[ 100 ];
    int   temp_int;
    int   temp_boolean;
    int   result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "chunk-min-chunk-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("chunk-min-chunk-size = %d\n",
                    fs_chunk_min_chunk_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Chunking min chunk size is %d.\n",
                    fs_chunk_min_chunk_size));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) fs_chunk_min_chunk_size = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "chunk-rechunk-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("chunk-rechunk-size = %d\n",
                    fs_chunk_min_rechunk_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Chunking rechunk size is %d.\n",
                    fs_chunk_min_rechunk_size));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) fs_chunk_min_rechunk_size = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "chunk-max-step"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("chunk-max-step = %d\n", fs_chunk_max_step));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Chunking max step is %d.\n", fs_chunk_max_step));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) fs_chunk_max_step = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "chunk-rechunk-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("chunk-rechunk-level = %d\n",
                    fs_chunk_rechunk_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Chunking rechunk level is %d.\n",
                    fs_chunk_rechunk_level));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "chunk-connect-corners"))
         || (match_pattern(lc_option, "chunk-connect-corners"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("chunk-connect-corners = %d\n",
                    fs_chunk_connect_corners));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Chunking connects corners is %d.\n",
                    fs_chunk_connect_corners));
        }
        else
        {
            ERE(temp_boolean = get_boolean_value(value));
            fs_chunk_connect_corners = temp_boolean;
        }

        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            segment_from_background
 *
 * Segments image chunks from the background
 *
 * This routine segments the image chunks from the background, given a function
 * which computes whether or not a pixel is a background pixel. The routine
 * compute a list of chunks which are a pixel count and a list of pixels. The
 * pixels themselves are the pixel values and their coordinates.
 *
 * Returns:
 *     The number of image chunks greater than a requested size on sucess and
 *     ERROR on failure.
 *
 * Index: images, image segmentation, segmentation
 *
 * -----------------------------------------------------------------------------
*/

/*
// Note: We abuse global variables a bit for this routine to keep performance
// within reason. Thus the code is a little sketchy. Basically, the image and
// the marked pixels are treated as globals, and the marked pixels are updated
// in the region growing routine which also used them. The region growing
// routine is called recursively.
*/

int segment_from_background
(
    const KJB_image*   ip,
    int                (*is_background_pixel_fn) (Pixel),
    Image_chunk_info** image_chunk_info_ptr_ptr
)
{
    int            num_rows;
    int            num_cols;
    Pixel*         cur_pix;
    int            i;
    int            j;
    Queue_element* chunk_list_head  = NULL;
    int            num_chunks       = 0;
#ifdef TEST
    int            background_count = 0;
#endif


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    /* If image size has changed, reset recycled storage. */
    if (    (num_rows != fs_marked_pixels_num_rows)
         || (num_cols != fs_marked_pixels_num_cols)
       )
    {
        free_2D_int_array(fs_marked_pixels);
        NRE(fs_marked_pixels = allocate_2D_int_array(num_rows, num_cols));
        fs_marked_pixels_num_rows = num_rows;
        fs_marked_pixels_num_cols = num_cols;

        free_image_chunk(fs_cached_chunk_ptr);

        NRE(fs_cached_chunk_ptr = create_image_chunk(num_rows * num_cols,
                                                  num_rows * num_cols));
    }

    /* Normally bad practice, but we want to improve performace! */
    fs_cached_const_ip = ip;

    /* If pixel is backround, then mark true, otherwise init to false. */
    for (i=0; i<num_rows; i++)
    {
        cur_pix = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            if (fs_cached_const_ip->pixels[ i ][ j ].extra.invalid.pixel == VALID_PIXEL)
            {
                fs_marked_pixels[ i ][ j ] = is_background_pixel_fn(*cur_pix);
            }
            else
            {
                fs_marked_pixels[ i ][ j ] = TRUE;
            }

            cur_pix++;

#ifdef TEST
            if (fs_marked_pixels[ i ][ j ]) background_count++;
#endif
        }
    }

#ifdef TEST
    dbi(background_count);
#endif

    /*
    // Visit each pixel. For each one that is not marked, create a chunk, and
    // segment the chunk by growing the region. (Note that as the region grows,
    // those pixels get marked).
    */
    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            if (! fs_marked_pixels[ i ][ j ])
            {
                Image_chunk* out_chunk_ptr;

                fs_cached_num_pixels = 0;
                fs_cached_i_sum = 0;
                fs_cached_j_sum = 0;
                fs_cached_num_boundary_pixels = 0;

                verbose_pso(20, "Growing chunk (%d,%d) ... ", i, j);

                grow_chunk(i, j);

                verbose_pso(20, "%d pixels.\n", fs_cached_num_pixels);

                out_chunk_ptr = create_image_chunk(fs_cached_num_pixels,
                                                   fs_cached_num_boundary_pixels);

                if (out_chunk_ptr == NULL)
                {
                    num_chunks = ERROR;
                    break;
                }

                num_chunks++;
                out_chunk_ptr->chunk_number = num_chunks;
                out_chunk_ptr->num_pixels = fs_cached_num_pixels;
                out_chunk_ptr->num_boundary_pixels = fs_cached_num_boundary_pixels;
                out_chunk_ptr->i_CM = fs_cached_i_sum / fs_cached_num_pixels;
                out_chunk_ptr->j_CM = fs_cached_j_sum / fs_cached_num_pixels;

                for (j=0; j<fs_cached_num_pixels; j++)
                {
                    out_chunk_ptr->pixels[ j ] = fs_cached_chunk_ptr->pixels[ j ];
                }

                for (j=0; j<fs_cached_num_boundary_pixels; j++)
                {
                    out_chunk_ptr->boundary_pixels[ j ] =
                                        fs_cached_chunk_ptr->boundary_pixels[ j ];
                }

                if (insert_into_queue(&chunk_list_head, (Queue_element**)NULL,
                                      out_chunk_ptr)
                    == ERROR)
                {
                    free_image_chunk(out_chunk_ptr);
                    num_chunks = ERROR;
                    break;
                }
            }

            if (num_chunks == ERROR) break;
        }
    }

    /* Free old chunk info, and create a new one at the same location. */
    if (num_chunks != ERROR)
    {
        if (get_target_image_chunk_info(image_chunk_info_ptr_ptr, num_chunks)
            == ERROR
           )
        {
            num_chunks = ERROR;
        }
    }

    /* Put list of chunks into chunk info structure. */
    if (num_chunks != ERROR)
    {
        Queue_element* cur_elem = chunk_list_head;

        for (i=0; i<num_chunks; i++)
        {
            Queue_element* save_cur_elem = cur_elem;
            Image_chunk* out_chunk_ptr = (Image_chunk*)(cur_elem->contents);


            (*image_chunk_info_ptr_ptr)->chunks[ i ] = out_chunk_ptr;

            cur_elem = cur_elem->next;

            kjb_free(save_cur_elem);
        }
    }
    else
    {
        free_queue(&chunk_list_head, (Queue_element**)NULL,
                   free_image_chunk_storage);
    }

    return num_chunks;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_chunk(int i, int j)
{
    Pixel_info* cur_chunk_pixel_ptr;
    Pixel*      cur_image_pixel_ptr;
    int         boundary            = TRUE;
    int         di;
    int         dj;
    int         i_offset;
    int         j_offset;


    cur_chunk_pixel_ptr = &(fs_cached_chunk_ptr->pixels[ fs_cached_num_pixels ]);
    cur_image_pixel_ptr = &(fs_cached_const_ip->pixels[ i ][ j ]);

    ASSERT(!fs_marked_pixels[ i ][ j ]);
    ASSERT(cur_image_pixel_ptr->extra.invalid.pixel == VALID_PIXEL);

    cur_chunk_pixel_ptr->i = i;
    cur_chunk_pixel_ptr->j = j;

    fs_cached_num_pixels++;
    fs_cached_i_sum += i;
    fs_cached_j_sum += j;

    fs_marked_pixels[ i ][ j ] = TRUE;

    for (di = -fs_chunk_max_step; di <= fs_chunk_max_step; di++)
    {
        for (dj = -fs_chunk_max_step; dj <= fs_chunk_max_step; dj++)
        {
            if ((di != 0) || (dj != 0))
            {
                i_offset = i + di;
                j_offset = j + dj;

                if (    (i_offset > 0)
                     && (j_offset > 0)
                     && (i_offset < fs_marked_pixels_num_rows)
                     && (j_offset < fs_marked_pixels_num_cols)
                     && (fs_chunk_connect_corners || (i_offset != j_offset))
                     && ( ! fs_marked_pixels[ i_offset ][ j_offset ])
                   )
                {
                    grow_chunk(i_offset, j_offset);
                    boundary = FALSE;
                }
            }
        }
    }

    if (boundary)
    {
        fs_cached_chunk_ptr->boundary_pixels[ fs_cached_num_boundary_pixels ] =
                                                        *cur_chunk_pixel_ptr;
        fs_cached_num_boundary_pixels++;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_target_image_chunk_info
(
    Image_chunk_info** image_chunk_info_ptr_ptr,
    int                num_chunks
)
{

    free_image_chunk_info(*image_chunk_info_ptr_ptr);

    *image_chunk_info_ptr_ptr = create_image_chunk_info(num_chunks);

    if (*image_chunk_info_ptr_ptr == NULL)
    {
        return ERROR;
    }
    else
    {
        return NO_ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Image_chunk_info* create_image_chunk_info(int num_chunks)
{
    Image_chunk_info* image_chunk_info_ptr;
    int               i;

    NRN(image_chunk_info_ptr = TYPE_MALLOC(Image_chunk_info));

    if (num_chunks == 0)
    {
        image_chunk_info_ptr->chunks = NULL;
    }
    else
    {
        image_chunk_info_ptr->chunks = N_TYPE_MALLOC(Image_chunk*, num_chunks);

        if (image_chunk_info_ptr->chunks == NULL)
        {
            kjb_free(image_chunk_info_ptr);
            return NULL;
        }
    }

    image_chunk_info_ptr->num_chunks = num_chunks;

    for (i=0; i<num_chunks; i++)
    {
        image_chunk_info_ptr->chunks[ i ] = NULL;
    }

    return image_chunk_info_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void free_image_chunk_info(Image_chunk_info* image_chunk_info_ptr)
{

    if (image_chunk_info_ptr != NULL)
    {
        int num_chunks = image_chunk_info_ptr->num_chunks;
        int i;

        for (i=0; i<num_chunks; i++)
        {
            free_image_chunk(image_chunk_info_ptr->chunks[ i ]);
        }
        kjb_free(image_chunk_info_ptr->chunks);
        kjb_free(image_chunk_info_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Image_chunk* create_image_chunk
(
    int num_pixels,
    int num_boundary_pixels
)
{
    Image_chunk* chunk_ptr;


    NRN(chunk_ptr = TYPE_MALLOC(Image_chunk));

    chunk_ptr->pixels = N_TYPE_MALLOC(Pixel_info, num_pixels);
    chunk_ptr->boundary_pixels = N_TYPE_MALLOC(Pixel_info, num_boundary_pixels);

    if ((chunk_ptr->pixels == NULL) || (chunk_ptr->boundary_pixels == NULL))
    {
        kjb_free(chunk_ptr->pixels);
        kjb_free(chunk_ptr->boundary_pixels);
        kjb_free(chunk_ptr);
        return NULL;
    }

    chunk_ptr->num_pixels = num_pixels;
    chunk_ptr->num_boundary_pixels = num_boundary_pixels;

    return chunk_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void free_image_chunk_storage(void* chunk_ptr)
{

    free_image_chunk((Image_chunk*)chunk_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void free_image_chunk(Image_chunk* chunk_ptr)
{

    if (chunk_ptr != NULL)
    {
        kjb_free(chunk_ptr->pixels);
        kjb_free(chunk_ptr->boundary_pixels);
    }

    kjb_free(chunk_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void prepare_memory_cleanup(void)
{
    static int first_time = TRUE;


    if (first_time)
    {
        add_cleanup_function(free_allocated_static_data);
        first_time = FALSE;
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void free_allocated_static_data(void)
{
    free_2D_int_array(fs_marked_pixels);
    free_image_chunk(fs_cached_chunk_ptr);

}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

