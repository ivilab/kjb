
/* $Id: i_hdrc.c 5831 2010-05-02 21:52:24Z ksimek $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Lindsay Martin and Kobus Barnard (authors).
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

#include "i/i_hdrc.h"
#include "m/m_mat_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------ */
#ifndef DATA_DIR
#    define DATA_DIR               "data"
#endif

#ifndef HDRC_DIR
#    define HDRC_DIR               "HDRC"
#endif

#ifndef HDRC_COLOUR_FPC_FILE
#    define HDRC_COLOUR_FPC_FILE   "hdrc-colour-12476.cfg"
#endif

#ifndef HDRC_MONO_FPC_FILE
#    define HDRC_MONO_FPC_FILE     "hdrc-mono-12485.cfg"
#endif

#ifndef HDRC_NUM_NEIGHBOURS
#    define HDRC_NUM_NEIGHBOURS    6
#endif

#ifndef HDRC_NUM_CHANNELS
#    define HDRC_NUM_CHANNELS      3
#endif

#ifndef HDRC_SENSOR_LOG_BASE
#    define HDRC_SENSOR_LOG_BASE   2.0
#endif

/* ------------------------------------------------------------------------ */

static char fs_fixed_pixel_correction_path[ MAX_FILE_NAME_SIZE ] = {'\0'};
static int  fs_enable_fixed_pixel_correction = FALSE;
static Hdrc_camera_type             fs_camera_type    = HDRC_MONOCHROME;
static Pixel_info*                  fs_bad_pixels_ptr = NULL;
static int                          fs_num_bad_pixels = NOT_SET;
static Hdrc_pixel_correction_method fs_fpc_method     = AVERAGE_NEIGHBOURS;
static Hdrc_neighbour_avg_space     fs_avg_space      = LOG_SENSOR_SPACE;

/* ------------------------------------------------------------------------- */

static int initialize_bad_pixel_list(void);
static int set_bad_pixel_list(FILE* bad_pixels_fp);

static int fp_read_bad_pixel_list
(
    Pixel_info** bad_pixels_ptr_ptr,
    int*         num_bad_pixels_ptr,
    FILE*        fp
);

static int get_hdrc_sensor(int i, int j);

static int is_pixel_in_bounds(int i, int j, const KJB_image* ip);

static int get_pixel_data
(
    double*          channel_data_ptr,
    int              channel,
    int              i,
    int              j,
    const KJB_image* ip
);

static int get_hdrc_neighbours
(
    Hdrc_neighbour_vector** target_nv_ptr,
    Hdrc_neighbour_type     neighbour_type,
    int                     i,
    int                     j,
    const KJB_image*        ip
);

static int get_neighbour_channel_average
(
    Vector**                 average_vpp,
    Hdrc_neighbour_avg_space avg_space,
    Hdrc_neighbour_vector*   nv
);

static int get_neighbour_average
(
    double*                  average_ptr,
    Hdrc_neighbour_avg_space avg_space,
    Hdrc_neighbour_vector*   nv
);

static int get_target_hdrc_neighbour_vector
(
    Hdrc_neighbour_vector** target_nv_ptr,
    int                     length
);

static void free_hdrc_neighbour_vector(Hdrc_neighbour_vector* nv);


#ifdef TRACK_MEMORY_ALLOCATION
    static void prepare_memory_cleanup(void);
    static void free_allocated_static_data(void);
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           set_hdrc_options
 *
 *
 *
 * -----------------------------------------------------------------------------
*/
int set_hdrc_options(const char* option, const char* value)
{
    char  lc_option[ 200 ];
    int   result = NOT_FOUND;

    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "hdrc-fpc-file"))
         || (match_pattern(lc_option, "hdrc-fixed-pixel-correction-file"))
       )
    {
        const char* fpc_file_str;

        if (*fs_fixed_pixel_correction_path == '\0')
        {
            fpc_file_str = "<not set>";
        }
        else
        {
            fpc_file_str = fs_fixed_pixel_correction_path;
        }

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hdrc-fixed-pixel-correction-file=%s\n", fpc_file_str));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HDRC fixed pixel correction coordinate file is %s\n",
                    fpc_file_str));
        }
        else
        {
            result = set_hdrc_fixed_pixel_correction_file(value);
        }
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "hdrc-fpc-method"))
         || (match_pattern(lc_option, "hdrc-fixed-pixel-correction-method")))
    {
        const char* fpc_method_str1;
        const char* fpc_method_str2;

        if (fs_fpc_method == NONE)
        {
            fpc_method_str1 = "none";
            fpc_method_str2 = "not corrected";
        }
        else if (fs_fpc_method == SET_PIXEL_TO_ZERO)
        {
            fpc_method_str1 = "zero";
            fpc_method_str2 = "set to zero";
        }
        else if (fs_fpc_method == FLAG_PIXEL_AS_INVALID)
        {
            fpc_method_str1 = "invalidate";
            fpc_method_str2 = "flagged as invalid";
        }
        else if (fs_fpc_method == AVERAGE_NEIGHBOURS)
        {
            fpc_method_str1 = "average-neighbours";
            fpc_method_str2 = "average of neighbours";
        }
        else
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hdrc-fixed-pixel-correction-method = %s\n",
                    fpc_method_str1));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Known HDRC fixed pixel errors are %s.\n",
                    fpc_method_str2));
        }
        else
        {
            if (    (match_pattern(value, "f"))
                 || (match_pattern(value, "off"))
                 || (match_pattern(value, "none")))
            {
                fs_fpc_method = NONE;
                fs_enable_fixed_pixel_correction = FALSE;
                result = NO_ERROR;
            }
            else if (    (match_pattern(value, "0"))
                      || (match_pattern(value, "zero")))
            {
                fs_fpc_method = SET_PIXEL_TO_ZERO;
                fs_enable_fixed_pixel_correction = TRUE;
                result = NO_ERROR;
            }
            else if (    (match_pattern(value, "invalid"))
                      || (match_pattern(value, "invalidate")))
            {
                fs_fpc_method = FLAG_PIXEL_AS_INVALID;
                result = NO_ERROR;
            }
            else if (    (match_pattern(value, "average"))
                      || (match_pattern(value, "average-neighbours")))
            {
                fs_fpc_method = AVERAGE_NEIGHBOURS;
                fs_enable_fixed_pixel_correction = TRUE;
                result = NO_ERROR;
            }
            else
            {
                set_error("%s is an invalid value for option %s",
                          value, option);
                result = ERROR;
            }
        }
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "hdrc-avg-space"))
         || (match_pattern(lc_option, "hdrc-neighbour-averaging-space")))
    {
        const char* avg_space_str1;
        const char* avg_space_str2;

        if (fs_avg_space == LOG_SENSOR_SPACE)
        {
            avg_space_str1 = "sensor";
            avg_space_str2 = "sensor (log)";
        }
        else if (fs_avg_space == LINEAR_INTENSITY_SPACE)
        {
            avg_space_str1 = "linear";
            avg_space_str2 = "linear (intensity)";
        }
        else
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hdrc-neighbour-averaging-space=%s.\n", avg_space_str1));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HDRC neighbouring pixels are averaged in %s space.\n",
                    avg_space_str2));
        }
        else
        {
            if (   (match_pattern(value, "sensor"))
                || (match_pattern(value, "log"))
                || (match_pattern(value, "log-sensor")))
            {
                fs_avg_space = LOG_SENSOR_SPACE;
                result = NO_ERROR;
            }
            else if (   (match_pattern(value, "linear"))
                     || (match_pattern(value, "intensity"))
                     || (match_pattern(value, "linear-intensity")))
            {
                fs_avg_space = LINEAR_INTENSITY_SPACE;
            }
            else
            {
                set_error("%s is an invalid value for option %s",
                          value, option);
                result = ERROR;
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_correct_hdrc_fixed_pixels(KJB_image* ip)
{
    int                    i, j, c;
    int                    result = NO_ERROR;
    Hdrc_neighbour_vector* nv     = NULL;

    if (ip == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(initialize_bad_pixel_list());

    if (fs_enable_fixed_pixel_correction == FALSE || fs_fpc_method == NONE)
    {
        verbose_pso(3,
                    "HDRC fixed pixel correction is disabled and thus NOT done.\n");
    }
    else if (fs_bad_pixels_ptr == NULL || fs_num_bad_pixels == NOT_SET)
    {
        verbose_pso(3,
                    "HDRC fixed pixel correction is skipped - no pixel list.\n");
    }
    else
    {
        verbose_pso(5, "Correcting HDRC fixed pixel errors ...\n");

        verbose_pso(5, "  Pixel coord file  : %s.\n",
                    fs_fixed_pixel_correction_path);

        verbose_pso(5, "  Pixels to correct : %d.\n", fs_num_bad_pixels);

        if (fs_fpc_method == SET_PIXEL_TO_ZERO)
        {
            verbose_pso(5, "  Correction method : Set bad pixels to 0.\n");
        }
        else if (fs_fpc_method == FLAG_PIXEL_AS_INVALID)
        {
            verbose_pso(5, "  Correction method : Invalidate bad pixels.\n");
        }
        else if (fs_fpc_method == AVERAGE_NEIGHBOURS)
        {
            verbose_pso(5, "  Correction method : Average neighbours.\n");
        }
        else
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        ERE(get_target_hdrc_neighbour_vector(&nv, HDRC_NUM_NEIGHBOURS));

        for (c = 0; c < fs_num_bad_pixels; c++)
        {
            i = fs_bad_pixels_ptr[ c ].i;
            j = fs_bad_pixels_ptr[ c ].j;

            if (fs_fpc_method == SET_PIXEL_TO_ZERO)
            {
                (ip->pixels[ i ][ j ]).r = 0.0f;
                (ip->pixels[ i ][ j ]).g = 0.0f;
                (ip->pixels[ i ][ j ]).b = 0.0f;
                (ip->pixels[ i ][ j ]).extra.invalid.pixel = INVALID_PIXEL;
                (ip->pixels[ i ][ j ]).extra.invalid.r     = INVALID_PIXEL;
                (ip->pixels[ i ][ j ]).extra.invalid.g     = INVALID_PIXEL;
                (ip->pixels[ i ][ j ]).extra.invalid.b     = INVALID_PIXEL;
            }
            else if (fs_fpc_method == FLAG_PIXEL_AS_INVALID)
            {
                (ip->pixels[ i ][ j ]).extra.invalid.pixel = INVALID_PIXEL;
                (ip->pixels[ i ][ j ]).extra.invalid.r     = INVALID_PIXEL;
                (ip->pixels[ i ][ j ]).extra.invalid.g     = INVALID_PIXEL;
                (ip->pixels[ i ][ j ]).extra.invalid.b     = INVALID_PIXEL;
            }
            else if (fs_fpc_method == AVERAGE_NEIGHBOURS)
            {
                double sensor_average;

                if (fs_camera_type == HDRC_MONOCHROME)
                {
                    if (get_hdrc_neighbours(&nv, NEAREST_NEIGHBOURS,
                                            i, j, ip) == ERROR)
                    {
                        add_error("Failed to get monochrome nearest neighbours");
                        add_error("for pixel at %d, %d", i, j);
                        result = ERROR;
                        break;
                    }
                }
                else if (fs_camera_type == HDRC_COLOUR)
                {
                    if (get_hdrc_neighbours(&nv, SAME_SENSOR_CHANNEL,
                                            i, j, ip) == ERROR)
                    {
                        add_error("Failed to get colour nearest neighbours");
                        add_error("for pixel at %d, %d", i, j);
                        result = ERROR;
                        break;
                    }

                }

                if (get_neighbour_average(&sensor_average,
                                          fs_avg_space, nv) == ERROR)
                {
                    add_error("Failed to get average of nearest");
                    add_error("neighbours for pixel at %d, %d", i, j);
                    result = ERROR;
                    break;
                }

                (ip->pixels[ i ][ j ]).r = sensor_average;
                (ip->pixels[ i ][ j ]).g = sensor_average;
                (ip->pixels[ i ][ j ]).b = sensor_average;

                (ip->pixels[ i ][ j ]).extra.invalid.pixel = VALID_PIXEL;
                (ip->pixels[ i ][ j ]).extra.invalid.r     = VALID_PIXEL;
                (ip->pixels[ i ][ j ]).extra.invalid.g     = VALID_PIXEL;
                (ip->pixels[ i ][ j ]).extra.invalid.b     = VALID_PIXEL;
            }
        }

        verbose_pso(5, "Corrected %d known bad pixels.\n", c);
    }

    free_hdrc_neighbour_vector(nv);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      set_hdrc_fixed_pixel_correction_file
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int set_hdrc_fixed_pixel_correction_file(const char* file_name)
{
    FILE* bad_pixels_fp;
    int   boolean_value;
    int   result;

    if (*file_name == '\0')
    {
        if (fs_bad_pixels_ptr == NULL)
        {
            ERE(pso("No HDRC pixel correction list has been read yet.\n"));
        }
        else if (fs_enable_fixed_pixel_correction == FALSE)
        {
            ERE(pso("HDRC fixed pixel correction is currently disabled.\n"));
        }
        else
        {
            ERE(pso("Using HDRC pixel correction list read from file %s\n",
                    fs_fixed_pixel_correction_path));
        }

        return NO_ERROR;
    }
    else if (*file_name == '?')
    {
        if (fs_bad_pixels_ptr == NULL)
        {
            ERE(pso("hdrc-fixed-pixel-correction-file = <never set>\n"));
        }
        else if (fs_enable_fixed_pixel_correction == FALSE)
        {
            ERE(pso("hdrc-fixed-pixel-correction-file = off\n"));
        }
        else
        {
            ERE(pso("hdrc-fixed-pixel-correction-file = %s\n",
                    fs_fixed_pixel_correction_path));
        }

        return NO_ERROR;
    }
    else if (   (match_pattern(file_name, "c"))
             || (match_pattern(file_name, "clr"))
             || (match_pattern(file_name, "color"))
             || (match_pattern(file_name, "colour")))
    {
        fs_camera_type = HDRC_COLOUR;
        fs_enable_fixed_pixel_correction = TRUE;
        ERE(initialize_bad_pixel_list()); /* Read default colour config file */
        return NO_ERROR;
    }
    else if (   (match_pattern(file_name, "bw"))
             || (match_pattern(file_name, "m"))
             || (match_pattern(file_name, "mono"))
             || (match_pattern(file_name, "monochrome")))
    {
        fs_camera_type = HDRC_MONOCHROME;
        fs_enable_fixed_pixel_correction = TRUE;
        ERE(initialize_bad_pixel_list()); /* Read default monochrome config file */
        return NO_ERROR;
    }

    boolean_value = get_boolean_value(file_name);

    if (boolean_value == FALSE)
    {
        fs_enable_fixed_pixel_correction = FALSE;
        return NO_ERROR;
    }
    else if (boolean_value == TRUE)
    {
        fs_enable_fixed_pixel_correction = TRUE;
        return NO_ERROR;
    }

    NRE(bad_pixels_fp = kjb_fopen(file_name, "r"));

    result = set_bad_pixel_list(bad_pixels_fp);

    if (result == NO_ERROR)
    {
        fs_enable_fixed_pixel_correction = TRUE;
    }

    (void)kjb_fclose(bad_pixels_fp);  /* Ignore return--only reading. */

    return result;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

/* =============================================================================
 *                              ow_hdrc_demosaic
 *
 * Demosaics an image in a using the HDRC sensor pattern
 *
 * This routine demosaics an image according to the HDRC colour camera "brick
 * wall" sensor pattern. With this pattern, each pixel has six, rather than the
 * usual four neighbours.
 *
 * The image is overwritten with the demosaiced result.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index:
 *    hdrc, images, image demosaicing
 *
 * -----------------------------------------------------------------------------
*/

int ow_hdrc_demosaic(KJB_image* ip)
{
    KJB_image*             tmp_ip     = NULL;
    Hdrc_neighbour_vector* nv         = NULL;
    Vector*                average_vp = NULL;
    int                    i, j;
    int                    result;
    int                    num_rows, num_cols;
    int                    current_sensor;
    double                 same_sensor_average;
    double                 direct_sensor_data;
#ifdef DUMP_HDRC_DEBUG_PIXEL_COORDS
    FILE*                  fp;
#endif

    verbose_pso(10, "Demosaicing the image using the HDRC hexagon algorithm.\n");

    if (ip == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    result = get_zero_image(&tmp_ip, num_rows, num_cols);

    if (result == ERROR)
    {
        add_error("Failed to allocate output demosaiced HDRC colour image\n");
        return result;
    }

    ERE(get_target_hdrc_neighbour_vector(&nv, HDRC_NUM_NEIGHBOURS));
    ERE(get_initialized_vector(&average_vp, HDRC_NUM_CHANNELS, DBL_NOT_SET));

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            current_sensor = get_hdrc_sensor(i, j);

            /* Obtain the average of neighbouring sensors different than
             * the current sensor. */
            if ( get_hdrc_neighbours(&nv, NEAREST_NEIGHBOURS,
                                     i, j, ip) == ERROR )
            {
                add_error("Failed to get nearest neighbours for pixel at %d,%d",
                          i, j);
                result = ERROR;
                break;
            }

            if ( get_neighbour_channel_average(&average_vp,
                                               fs_avg_space, nv) == ERROR )
            {
                add_error("Failed to get neighbour channel averages for channel %d",
                          current_sensor);
                add_error("at pixel %d, %d", i, j);
                result = ERROR;
                break;
            }

            ASSERT( average_vp->elements[ current_sensor ] == DBL_NOT_SET );

            if ( (ip->pixels[ i ][ j ]).extra.invalid.pixel == VALID_PIXEL )
            {
                /* Use the current pixel's sensor value directly */
                if ( get_pixel_data(&direct_sensor_data,
                                    current_sensor,
                                    i, j, ip) == ERROR )
                {
                    add_error("Failed to get direct sensor reading for channel %d",
                              current_sensor);
                    add_error("at pixel %d, %d", i, j);
                    result = ERROR;
                    break;
                }

                average_vp->elements[ current_sensor ] = direct_sensor_data;
            }
            else
            {
                /* Get the neigbouring pixels matching the current sensor */
                if ( get_hdrc_neighbours(&nv, SAME_SENSOR_CHANNEL,
                                         i, j, ip) == ERROR )
                {
                    add_error("Failed to get same sensor neighbours for channel %d",
                              current_sensor);
                    add_error("at pixel %d, %d", i, j);
                    result = ERROR;
                    break;
                }

                /* Average the neighbouring sensor values */
                if ( get_neighbour_average(&same_sensor_average,
                                           fs_avg_space, nv) == ERROR )
                {
                    add_error("Failed to get same sensor average for channel %d",
                              current_sensor);
                    add_error("at pixel %d, %d", i, j);
                    result = ERROR;
                    break;
                }

                average_vp->elements[ current_sensor ] = same_sensor_average;
            }

            /* Update the colour channel image data */
            (tmp_ip->pixels[ i ][ j ]).r = average_vp->elements[ RED_INDEX   ];
            (tmp_ip->pixels[ i ][ j ]).g = average_vp->elements[ GREEN_INDEX ];
            (tmp_ip->pixels[ i ][ j ]).b = average_vp->elements[ BLUE_INDEX  ];
        }

        if (result == ERROR)
        {
            break;
        }
    }

    if (result == NO_ERROR)
    {
        result = kjb_copy_image(&ip, tmp_ip);
    }

    kjb_free_image(tmp_ip);
    free_hdrc_neighbour_vector(nv);
    free_vector(average_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ===========================================================================
 *                               get_hdrc_sensor
 *
 * Returns the channel of the sensor at the given image coords
 *
 * This routine returns the colour channel of the HDRC sensor at the given
 * image coordinates. Relies on the "hexagonal" "brick wall" sensor pattern
 * used in the colour HDRC camera. Used as helper to "ow_hdrc_demosaic".
 *
 * The first row (row 0) and subsequent even numbered rows have sensors in the
 * sequence RGB RGB... The second row (row 1) and subsequent odd numbered rows
 * have sensors in the sequence GBR GBR ...
 *
 * Returns:
 *|    Return value is equivalent to either
 *|    RED_INDEX, GREEN_INDEX or BLUE_INDEX
 *
 * Index:
 *    hdrc, images, image demosaicing
 *
 * -----------------------------------------------------------------------------
*/

static int get_hdrc_sensor(int i, int j)
{
    if (IS_EVEN(i))
    {
        return (j % 3);       /* RGBRGB ... */
    }
    else
    {
        return ((j + 1) % 3); /* GBRGBR ... */
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_pixel_data
(
    double*          channel_data_ptr,
    int              channel,
    int              i,
    int              j,
    const KJB_image* ip
)
{
    int result = NO_ERROR;

    if (ip == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if ( is_pixel_in_bounds(i, j, ip) )
    {
        if (channel == RED_INDEX)
        {
            *channel_data_ptr = (ip->pixels[ i ] [ j ]).r;
        }
        else if (channel == GREEN_INDEX)
        {
            *channel_data_ptr = (ip->pixels[ i ] [ j ]).g;
        }
        else if (channel == BLUE_INDEX)
        {
            *channel_data_ptr = (ip->pixels[ i ] [ j ]).b;
        }
        else
        {
            SET_ARGUMENT_BUG();
            result = ERROR;
        }
    }
    else
    {
        set_bug("Pixel at %d, %d is out of bounds.", i, j);
        result = ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int is_pixel_in_bounds(int i, int j, const KJB_image* ip)
{
    int result;

    if      ( (i < 0) || (i > ip->num_rows - 1) )
    {
        result = FALSE;
    }
    else if ( (j < 0) || (j > ip->num_cols - 1) )
    {
        result = FALSE;
    }
    else
    {
        result = TRUE;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ===========================================================================
 *                          get_hdrc_neighbours
 *
 * Gets HDRC neighbouring pixels.
 *
 * This routine finds the six neighbours of the specified HDRC image pixel.
 * Neighbours are found for the pixel at "i", "j" from the source image "ip".
 * The source image MUST be a raw HDRC float image before demosaicing. This
 * restiction validates the assumption that the r, g, b values in the source
 * image are the same.
 *
 * Two types of neighbouring pixels are found depending on the value of the
 * "neighbour_type" argument".
 *
 * If "neighbour_type" is NEAREST_NEIGHBOURS, then the six closest neighbouring
 * pixels are examined. In the case of a colour HDRC image, these neighbours will
 * be of a different sensor than the current image pixel at "i", "j". In the case
 * of a monochrome HDRC image, these are simply the closest spatial neighbours.
 *
 * If neighbour_type" is SAME_SENSOR_CHANNEL, then the six closest neighbours
 * with the same sensor as the current pixel are returned. This value has no
 * meaning for monochrome HDR cameras.
 *
 * Information on neighbouring pixels is placed in a vector of
 * "Hdrc_neighbour_info" structures, which are defined as follows:
 *|  typedef struct Hdrc_neighbour_info
 *|  {
 *|     int         sensor;    Colour channel of the neighbouring pixel.
 *|     Pixel_info  coords;    KJB_image coordinates of neighbouring pixel.
 *|     Pixel data;      Neighbouring pixel validity and colour data.
 *|  } Hdrc_neighbour_info;
 *
 * The "data" field for each neighbour contains the standard Pixel
 * validity information taken from the source image. A neighbour may also be
 * flagged invalid by this routine if its pixel coordinates are out of bounds
 * of the image array.
 *
 * Returns:
 *|    Return ERROR on failure, NO_ERROR on success.
 *
 * Index:
 *    hdrc, images
 *
 * -----------------------------------------------------------------------------
*/

static int get_hdrc_neighbours
(
    Hdrc_neighbour_vector** target_nv_ptr,
    Hdrc_neighbour_type     neighbour_type,
    int                     i,
    int                     j,
    const KJB_image*        ip
)
{
    int n, row, col;
    Hdrc_neighbour_vector* nv;
    Pixel       pixel;

    /*
     * Offsets from current pixel to 6 closest neighbours. For colour
     * camera, these neighbours are all different sensors than the sensor
     * at the current pixel. The i-index offset is for image rows. There
     * are two corresponding j-index (column) offsets, depending on whether
     * the current pixel's row is even or odd.
     */
    int i_offset     [HDRC_NUM_NEIGHBOURS] = { 0, -1, -1,  0,  1,  1};
    int j_even_offset[HDRC_NUM_NEIGHBOURS] = {-1,  1,  1,  1,  1,  0};
    int j_odd_offset [HDRC_NUM_NEIGHBOURS] = {-1, -1,  0,  1,  0, -1};

    /*
     * Offsets from the current pixel to the 6 neighbours that are the
     * same sensor as the current pixel. Applicable to colour camera
     * data only.
     */
    int i_same_sensor_offset     [HDRC_NUM_NEIGHBOURS] = {-1, -2, -1,  1,  2,  1};
    int j_even_same_sensor_offset[HDRC_NUM_NEIGHBOURS] = {-1,  0,  2,  2,  0, -1};
    int j_odd_same_sensor_offset [HDRC_NUM_NEIGHBOURS] = {-2,  0,  1,  1,  0, -2};


    if (ip == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_hdrc_neighbour_vector(target_nv_ptr, HDRC_NUM_NEIGHBOURS));
    nv = *target_nv_ptr;

    for (n = 0; n < HDRC_NUM_NEIGHBOURS; n++)
    {
        ASSERT(n < nv->length);

        if (neighbour_type == NEAREST_NEIGHBOURS)
        {
            row = i + i_offset[ n ];

            if (IS_EVEN(i))
                col = j + j_even_offset[ n ];
            else
                col = j + j_odd_offset[ n ];
        }
        else if (neighbour_type == SAME_SENSOR_CHANNEL)
        {
            row = i + i_same_sensor_offset[ n ];

            if (IS_EVEN(i))
                col = j + j_even_same_sensor_offset[ n ];
            else
                col = j + j_odd_same_sensor_offset[ n ];
        }
        else
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        nv->elements[ n ].coords.i = row;
        nv->elements[ n ].coords.j = col;

        if (   !(is_pixel_in_bounds(row, col, ip))
            ||  (ip->pixels[ row ][ col ]).extra.invalid.pixel)
        {
            nv->elements[ n ].sensor  = NOT_SET;

            nv->elements[ n ].data.r   = DBL_NOT_SET;
            nv->elements[ n ].data.b   = DBL_NOT_SET;
            nv->elements[ n ].data.g   = DBL_NOT_SET;

            nv->elements[ n ].data.extra.invalid.pixel = INVALID_PIXEL;
            nv->elements[ n ].data.extra.invalid.r     = INVALID_PIXEL;
            nv->elements[ n ].data.extra.invalid.g     = INVALID_PIXEL;
            nv->elements[ n ].data.extra.invalid.b     = INVALID_PIXEL;
        }
        else
        {
            pixel = ip->pixels[ row ][ col ];

            nv->elements[ n ].sensor  = get_hdrc_sensor(row, col);

            if (neighbour_type == NEAREST_NEIGHBOURS)
            {
                ASSERT( nv->elements[ n ].sensor != get_hdrc_sensor(i, j) );
            }
            else if (neighbour_type == SAME_SENSOR_CHANNEL)
            {
                ASSERT( nv->elements[ n ].sensor == get_hdrc_sensor(i, j) );
            }

            nv->elements[ n ].data.r = pixel.r;
            nv->elements[ n ].data.g = pixel.g;
            nv->elements[ n ].data.b = pixel.b;

            nv->elements[ n ].data.extra.invalid.pixel = pixel.extra.invalid.pixel;
            nv->elements[ n ].data.extra.invalid.r     = pixel.extra.invalid.r;
            nv->elements[ n ].data.extra.invalid.g     = pixel.extra.invalid.g;
            nv->elements[ n ].data.extra.invalid.b     = pixel.extra.invalid.b;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ===========================================================================
 *                  get_neighbour_channel_average
 *
 * Averages neighbouring sensor values by channel.
 *
 * This routine finds the average of neighbouring colour HDRC pixels and returns
 * the result in a Vector indexed by sensor channel. The neighbouring pixels
 * must already be in the neighbouring pixel vector "nv".
 *
 * Averages are computed in either sensor space ("avg_space" = LOG_SENSOR_SPACE),
 * or in linearized intensity space ("avg_space" = LINEAR_INTENSITY_SPACE). In
 * either case, the average value returned is in the log sensor space.
 *
 * Any neighbouring pixels flagged as invalid are ignored by this routine.
 *
 * Returns:
 *|    Return ERROR on failure, NO_ERROR on success.
 *
 * Index:
 *    hdrc, images
 *
 * -----------------------------------------------------------------------------
*/

static int get_neighbour_channel_average
(
    Vector**                 average_vpp,
    Hdrc_neighbour_avg_space avg_space,
    Hdrc_neighbour_vector*   nv
)
{
    double sum[ HDRC_NUM_CHANNELS ];
    int  count[ HDRC_NUM_CHANNELS ];

    Vector* average_vp;
    int i, channel, result;

    if (   avg_space != LOG_SENSOR_SPACE
        && avg_space != LINEAR_INTENSITY_SPACE)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (nv == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (i = 0; i < HDRC_NUM_CHANNELS; i++)
    {
        sum[ i ] = 0.0f;
        count[ i ] = 0;
    }

    result = get_initialized_vector(average_vpp,
                                    HDRC_NUM_CHANNELS,
                                    DBL_NOT_SET);
    if (result == ERROR)
    {
        add_error("Failed to allocate neighbour average vector");
        return ERROR;
    }

    average_vp = *average_vpp;

    for (i = 0; i < nv->length; i++)
    {
        if ( (nv->elements[ i ]).data.extra.invalid.pixel == INVALID_PIXEL )
        {
            continue;
        }

        channel = nv->elements[ i ].sensor;

        ASSERT( channel < HDRC_NUM_CHANNELS );

        if (avg_space == LOG_SENSOR_SPACE)
        {
            /* Ignore lint */
            sum[ channel ] += (double)nv->elements[ i ].data.r;
        }
        else if (avg_space == LINEAR_INTENSITY_SPACE)
        {
            sum[ channel ]
                += pow(HDRC_SENSOR_LOG_BASE, (double)nv->elements[ i ].data.r);
        }

        count[ channel ] += 1;
    }

    for (i = 0; i < HDRC_NUM_CHANNELS; i++)
    {
        if (count[ i ] > 0)
        {
            average_vp->elements[ i ]
                = (double)(sum[ i ] / (double)count[ i ]);
        }

        if (avg_space == LINEAR_INTENSITY_SPACE)
        {
            average_vp->elements[ i ]
                =  (double)(   log((*average_vpp)->elements[ i ])
                           / log(HDRC_SENSOR_LOG_BASE));
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* ===========================================================================
 *                           get_neighbour_average
 *
 * Averages all neighbouring sensor values.
 *
 * This routine finds the average of all valid neighbouring HDRC sensor values.
 * The neighbouring pixels must already be in the neighbouring pixel vector "nv".
 *
 * Averages are computed in either sensor space ("avg_space" = LOG_SENSOR_SPACE),
 * or in linearized intensity space ("avg_space" = LINEAR_INTENSITY_SPACE). In
 * either case, the average value returned is in the log sensor space.
 *
 * Any neighbouring pixels flagged as invalid are ignored by this routine.
 *
 * Returns:
 *|    Return ERROR on failure, NO_ERROR on success.
 *
 * Index:
 *    hdrc, images
 *
 * -----------------------------------------------------------------------------
*/

static int get_neighbour_average
(
    double*                  average_ptr,
    Hdrc_neighbour_avg_space avg_space,
    Hdrc_neighbour_vector*   nv
)
{
    int n, num_neighbours, result;
    double neighbour_sum;

    if (   avg_space != LOG_SENSOR_SPACE
        && avg_space != LINEAR_INTENSITY_SPACE)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (nv == NULL)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_neighbours = 0;
    neighbour_sum  = 0.0;

    for (n = 0; n < nv->length; n++)
    {
        if ( (nv->elements[ n ]).data.extra.invalid.pixel != VALID_PIXEL )
        {
            continue;
        }

        if (avg_space == LOG_SENSOR_SPACE)
        {
            neighbour_sum += (double)nv->elements[ n ].data.r;
        }
        else if (avg_space == LINEAR_INTENSITY_SPACE)
        {
            neighbour_sum += pow(HDRC_SENSOR_LOG_BASE,
                                 (double)nv->elements[ n ].data.r);
        }
        else
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

        num_neighbours++;
    }

    if (num_neighbours == 0)
    {
        set_error("No valid neighbours to average");
        result = ERROR;
    }
    else
    {
        *average_ptr = neighbour_sum / (double)num_neighbours;

        if (avg_space == LINEAR_INTENSITY_SPACE)
        {
            *average_ptr = log(*average_ptr) / log(HDRC_SENSOR_LOG_BASE);
        }

        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_target_hdrc_neighbour_vector
(
    Hdrc_neighbour_vector** target_nv_ptr,
    int                     length
)
{
    Hdrc_neighbour_vector* nv = *target_nv_ptr;

    if (length <= 0)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (nv == NULL)
    {
        NRE(nv = TYPE_MALLOC(Hdrc_neighbour_vector));
        NRE(nv->elements = N_TYPE_MALLOC(Hdrc_neighbour_info, length));
        nv->length = length;

        *target_nv_ptr = nv;
    }
    else if (length == nv->length)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else if (length < nv->length)
    {
        nv->length = length;
    }
    else
    {
        kjb_free(nv->elements);
        NRE(nv->elements = N_TYPE_MALLOC(Hdrc_neighbour_info, length));
        nv->length = length;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void free_hdrc_neighbour_vector(Hdrc_neighbour_vector* nv)
{
    if (nv != NULL)
    {
        if (nv->elements != NULL)
        {
#ifdef DISABLE
#ifdef TRACK_MEMORY_ALLOCATION
            check_initialization(nv->elements, nv->length,
                                 sizeof(Hdrc_neighbour_vector));
#endif
#endif
            kjb_free(nv->elements);
        }
    }

    kjb_free(nv);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\  */

static int initialize_bad_pixel_list(void)
{
    int         result                                = NO_ERROR;
    FILE*       bad_pixels_fp;
    const char* message_str                           = "HDRC pixel coord";
    char        camera_data_dir[ MAX_FILE_NAME_SIZE ];

    /* This function reads the default fixed pixel correction file.
     * Calls set_bad_pixel_list() to fetch the bad pixel list
     */

    if ((fs_enable_fixed_pixel_correction == FALSE) || (fs_bad_pixels_ptr != NULL))
    {
        return NO_ERROR;
    }

    BUFF_CPY(camera_data_dir, DATA_DIR);
    BUFF_CAT(camera_data_dir, DIR_STR);
    BUFF_CAT(camera_data_dir, HDRC_DIR);

    if (fs_camera_type == HDRC_COLOUR)
    {
        bad_pixels_fp = open_config_file((char*)NULL,
                                         camera_data_dir,
                                         HDRC_COLOUR_FPC_FILE,
                                         message_str);
    }
    else if (fs_camera_type == HDRC_MONOCHROME)
    {
        bad_pixels_fp = open_config_file((char*)NULL,
                                         camera_data_dir,
                                         HDRC_MONO_FPC_FILE,
                                         message_str);
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    if (bad_pixels_fp != NULL)
    {
        result = set_bad_pixel_list(bad_pixels_fp);
    }

    (void)kjb_fclose(bad_pixels_fp);  /* Ignore return--only reading. */

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_bad_pixel_list(FILE* bad_pixels_fp)
{
    Pixel_info* temp_pixels_ptr = NULL;
    int         temp_num_bad    = NOT_SET;
    int         result          = NO_ERROR;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (bad_pixels_fp != NULL)
    {

        result = fp_read_bad_pixel_list(&temp_pixels_ptr,
                                        &temp_num_bad,
                                        bad_pixels_fp);

        if (result != ERROR)
        {
            if (fs_bad_pixels_ptr != NULL)
                kjb_free(fs_bad_pixels_ptr);

            fs_bad_pixels_ptr = temp_pixels_ptr;
            fs_num_bad_pixels = temp_num_bad;

            BUFF_GET_USER_FD_NAME(fileno(bad_pixels_fp),
                                  fs_fixed_pixel_correction_path);

            fs_enable_fixed_pixel_correction = TRUE;
        }
        else
        {
            add_error("Unable to use HDRC fixed pixel correction file\n%P.",
                      bad_pixels_fp);
            add_error("Bad pixels NOT corrected.");
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* --------------------------------------------------------------------------
 *                          fp_read_bad_pixel_list
 *
 * Reads a list of known bad pixel coordinates from a file.
 *
 * Reads a list of known bad pixel coordinates from an HDRC configuration
 * file and converts the pixel coordinates to the (i, j) format (row, col)
 * used in the KJB "Image" structure.
 *
 * Places the pixel coordinates into the array of Pixel_info structures
 * referenced by "bad_pixels_ptr_ptr". The length of this array is placed
 * into the integer variable referenced by "num_bad_pixels_ptr".
 *
 * The configuration file must already be opened and have its filehandle
 * in "fp".
 *
 * Note:
 *|  This routine currently does not read IMS configuration files as supplied
 *|  by the vendor. The file header must be stripped, up to and including the
 *|  line "CORPIXEL=<X>". i.e. the file MUST just contain pixel coordinates.
 *
 * Returns:
 *   NO_ERROR on success, or ERROR on failure, with a descriptive error
 *   message being set.
 *
 * Index:
 *  images, float images, HDRC, fixed pixel correction
 *
 * Author: Lindsay Martin
 *
 * -----------------------------------------------------------------------------
*/

static int fp_read_bad_pixel_list
(
    Pixel_info** bad_pixels_ptr_ptr,
    int*         num_bad_pixels_ptr,
    FILE*        fp
)
{
    Matrix* temp_mp = NULL;
    int     pix, count;

    ERE(fp_read_formatted_matrix(&temp_mp, fp));

    if (temp_mp == NULL)
    {
        add_error("Failed to read bad pixel list from fixed");
        add_error("pixel correction file %F", fp);
        return ERROR;
    }

    count = temp_mp->num_rows;

    if (count <= 1)
    {
        set_error("Bad number of coords in fixed pixel correction file %F",
                  fp);
        add_error(" (count=%d)", count);
        add_error(" (expected at count to be at least 1)");
        free_matrix(temp_mp);
        return ERROR;
    }

    if (temp_mp->num_cols != 2)
    {
        set_error("Bad number of columns in fixed pixel correction file %F",
                  fp);
        add_error(" (num_cols=%d)", temp_mp->num_cols);
        add_error(" (expected 2)");
        free_matrix(temp_mp);
        return ERROR;
    }

    /* db_mat(temp_mp); */

    if (*bad_pixels_ptr_ptr != NULL)
    {
        kjb_free(*bad_pixels_ptr_ptr);
        *num_bad_pixels_ptr = NOT_SET;
    }

    *bad_pixels_ptr_ptr = N_TYPE_MALLOC(Pixel_info, count);

    if (*bad_pixels_ptr_ptr == NULL)
    {
        set_error("Failed to allocate array of pixel coords of length %d",
                  count);
        free_matrix(temp_mp);
        return ERROR;
    }

    for (pix = 0; pix < count; pix++)
    {
        ((*bad_pixels_ptr_ptr)[ pix ]).i
            = (HDRC_NUM_ROWS - 1) - (int)temp_mp->elements[ pix ][ 1 ];

        ((*bad_pixels_ptr_ptr)[ pix ]).j
            = (int)temp_mp->elements[ pix ][ 0 ];
    }

    *num_bad_pixels_ptr = count;

    free_matrix(temp_mp);

    return NO_ERROR;
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
    if (fs_bad_pixels_ptr != NULL)
    {
        kjb_free(fs_bad_pixels_ptr);
    }
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

