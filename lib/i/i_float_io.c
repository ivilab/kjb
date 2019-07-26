
/* $Id: i_float_io.c 22170 2018-06-23 23:01:50Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author). (Lindsay Martin
|  contributed the interface to his hdrc code).
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

#include "i/i_convert.h"
#include "i/i_demosaic.h"
#include "i/i_display.h"
#include "i/i_float_io.h"
#include "i/i_gamma.h"
#include "i/i_hdrc.h"      /* Lindsay */
#include "i/i_map.h"
#include "i/i_matrix.h"
#include "i/i_offset.h"
#include "i/i_ras.h"
#include "i/i_set_aux.h"
#include "i/i_transform.h"
#include "i/i_valid.h"
#include "i/i_video.h"

#ifdef KJB_HAVE_TIFF
#include "tiffio.h"
#endif

#ifdef KJB_HAVE_JPEG
#ifndef __C2MAN__

#ifdef KJB_CPLUSPLUS
#ifdef SUN5
extern "C"
{
#endif
#endif

#ifndef HAVE_BOOLEAN
#    define HAVE_BOOLEAN
#endif 

#define boolean Bool
#    include "jpeglib.h"
#undef boolean 

#ifdef KJB_CPLUSPLUS
#ifdef SUN5
}
#endif
#endif

#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define MAX_NUM_DISPLAYED_IMAGES    1000

#define MID_MAGIC_NUM      0x6d69640a   /* "mid\n" in ascii */
#define KIFF_MAGIC_NUM     0x6b696666   /* "kiff" in ascii  */
#define OLD_KIFF_MAGIC_NUM 0x32363231   /* "2621" in ascii  */

#define TIFF_LSB_FIRST_MAGIC_NUM     0x49492a00
#define TIFF_MSB_FIRST_MAGIC_NUM     0x4d4d002a

#ifdef LOOKS_BUGGY
/*
// I think the second pair of bytes has some variation amoung jpegs. I don't
// know the meaning or range of the possibilities. We will go for treating the
// first 2 bytes as being the magic number.
*/
#define JPEG_MAGIC_NUM               0xffd8ffe0
#else
#define JPEG_MAGIC_NUM               0xffd8
#endif

#define PCD_MAGIC_NUM_OFFSET         0x800
#define PCD_ROTATE_OFFSET            0x0e02

#define NUM_DCS_460_ROWS    2036
#define NUM_DCS_460_COLS    3060
#define RAW_DCS_460_16_BIT_SIZE   (6 * NUM_DCS_460_ROWS * NUM_DCS_460_COLS)

/* -------------------------------------------------------------------------- */

static int    fs_internal_pcd_read = FALSE;
static int    fs_pcd_ycc_to_rgb_forward_conversion = TRUE;
static int    fs_convert_pcd_ycc = FALSE;
static int    fs_write_validity_kiff  = TRUE;
static Matrix* fs_display_matrix_mp    = NULL;
static int     fs_strip_images_on_read = FALSE;
static int     fs_do_gamma_correction  = FALSE;
static int     fs_scale_by_max_rgb  = FALSE;
static int     fs_display_log  = FALSE;
static int     fs_adjust_image_range  = FALSE;
static Pixel   fs_invalid_pixel_colour =
                                     { FLT_NOT_SET, FLT_NOT_SET, FLT_NOT_SET,
                                         {{  INVALID_PIXEL, INVALID_PIXEL,
                                            INVALID_PIXEL, INVALID_PIXEL }} };

static int    fs_bloom_removal_count        = 0;

static int    fs_reset_invalid_pixels       = FALSE; /* Only applies to v-kiff */
static int    fs_invalidate_negative_pixels = TRUE; /* Only applies to reg kiff */

static double fs_input_gamma_before_offset  = DBL_NOT_SET;
static double fs_input_gamma_after_offset   = DBL_NOT_SET;
static int    fs_linearize_pcd   = FALSE;

static int    fs_strip_top                  = 0;
static int    fs_strip_bottom               = 0;
static int    fs_strip_left                 = 0;
static int    fs_strip_right                = 0;

static int    fs_force_valid_kiff_on_read = FALSE;

static int    fs_tiff_write_bps = 8;

/* This buffer could be overrun by the jpeg library! */
static char   jpeg_error_buff[ 1000 ]; 

/* Lindsay - Nov 18, 1999 */
static int   fs_hdrc_enable_write_pixel_coords = FALSE;
static float fs_hdrc_pixel_threshold_value = FLT_ZERO;
/* End Lindsay */

static const char* conversion_explanation_str = NULL;


/* -------------------------------------------------------------------------- */

static int set_display_matrix(const char* file_name);
static int set_invalid_pixel_colour(const char* value);
static int print_invalid_pixel_colour(void);
static int print_invalid_pixel_colour_option(void);
static int print_display_matrix(void);
static void free_display_matrix(void);

static int kjb_read_image_3
(
    KJB_image** ipp,
    const char* file_name_and_sub_image,
    int* validity_data_is_part_of_image_ptr
);

static int read_image_from_raster(KJB_image** ip, FILE* fp);

#ifdef KJB_HAVE_TIFF
static int read_image_from_tiff(KJB_image** ipp, FILE* fp);
#endif

#ifdef KJB_HAVE_JPEG
#ifndef __C2MAN__
static int read_image_from_jpeg(KJB_image** ipp, FILE* fp);
#endif
#endif

static int read_image_from_pcd
(
    KJB_image** ipp,
    const char* file_name,
    const char* sub_image,
    int         pcd_rotate
);

static int ow_convert_pcd_ycc_to_rgb(KJB_image* ip);

static int read_image_from_bmp(KJB_image** ipp, FILE* fp);

static int read_image_from_pnm_P6(KJB_image** ipp, FILE* fp);

static int read_pnm_6_header_num(FILE* fp, int* num_ptr);

static int read_image_from_MID_file(KJB_image** ip, FILE* fp);

static int read_image_from_raw_dcs_460_16_bit(KJB_image** ipp, FILE* fp);

/* Lindsay - Sept 28, 1999 */
static int read_image_from_raw_hdrc_16_bit(KJB_image** ipp, FILE* fp);

static int write_threshold_pixel_coords
(
    KJB_image* ip,
    char*      file_name,
    double     threshold
);

/* End Lindsay - Sept 28, 1999 */

static int read_byte_pixels
(
    KJB_image** ip,
    FILE*       fp,
    int         num_rows,
    int         num_cols
);

static int read_kiff_header
(
    FILE*           fp,
    Kiff_data_type* format_ptr,
    int*            num_rows_ptr,
    int*            num_cols_ptr,
    int*            reversed_ptr
);

static int read_kiff_header_guts
(
    FILE*           fp,
    Kiff_data_type* format_ptr,
    int*            num_rows_ptr,
    int*            num_cols_ptr,
    int*            reversed_ptr
);

static int read_float_pixels
(
    KJB_image** ipp,
    FILE*       fp,
    int         num_rows,
    int         num_cols,
    int         reversed
);

static int read_float_with_validity_pixels
(
    KJB_image** ipp,
    FILE*       fp,
    int         num_rows,
    int         num_cols,
    int         reversed
);

static int read_float_channel_data
(
    KJB_image** ipp,
    FILE*       fp,
    int         num_rows,
    int         num_cols,
    int         reversed
);

static int write_image_as_raster
(
    const KJB_image* ip,
    const char*      file_name
);

#ifdef KJB_HAVE_TIFF
static int write_image_as_tiff
(
    const KJB_image* ip,
    const char*      file_name
);

#endif

#ifdef KJB_HAVE_JPEG
#ifndef __C2MAN__
static int write_image_as_jpeg
(
    const KJB_image* ip,
    const char*      file_name
);
#endif
#endif

static int write_image_as_validity_kiff
(
    const KJB_image* ip,
    const char*      file_name
);

static int write_image_as_float_kiff
(
    const KJB_image* ip,
    const char*      file_name
);

static int write_image_as_MID_file
(
    const KJB_image* ip,
    const char*      file_name
);

static int write_image_as_raw_16_bit
(
    const KJB_image* ip,
    const char*      file_name
);

static int write_validity_kiff_image_data(const KJB_image* ip, FILE* fp);

static int write_float_pixel_image_data(const KJB_image* ip, FILE* fp);

static int write_float_channel_image_data(const KJB_image* ip, FILE* fp);

static int write_image_for_display(const void* ip, char* title);



/* -------------------------------------------------------------------------- */

int set_image_input_options(const char* option, const char* value)
{
    char   lc_option[ 100 ];
    int    temp_int;
    int    temp_boolean_value;
    double temp_double;
    int    result             = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "read-pcd")
          || match_pattern(lc_option, "pcd-read")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("read-pcd = %s\n",
                    fs_internal_pcd_read ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("PCD images %s read internally.\n",
                    fs_internal_pcd_read ? "are" : "are not"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_internal_pcd_read = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "pcd-ycc-forward-conversion")
          || match_pattern(lc_option, "pcd-ycc-to-rgb-forward-conversion")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("pcd-ycc-to-rgb-forward-conversion = %s\n",
                    fs_pcd_ycc_to_rgb_forward_conversion ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("PCD ycc to rgb forward conversion %s used.\n",
                    fs_pcd_ycc_to_rgb_forward_conversion ? "is" : "not"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_pcd_ycc_to_rgb_forward_conversion = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "convert-pcd-ycc")
          || match_pattern(lc_option, "convert-pcd-ycc-to-rgb")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("convert-pcd-ycc-to-rgb = %s\n",
                    fs_convert_pcd_ycc ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Images %s converted from PCD ycc to rgb.\n",
                    fs_convert_pcd_ycc ? "are" : "are not"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_convert_pcd_ycc = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "image-stripping"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("image-stripping = %s\n",
                    fs_strip_images_on_read ? "t" : "f")); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Images of size %dx%d ", FULL_VIDEO_NUM_ROWS,
                    FULL_VIDEO_NUM_COLS));
            ERE(pso("%s stripped on read (image-stripping=%s).\n",
                    fs_strip_images_on_read ? "are" : "are not",
                    fs_strip_images_on_read ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_strip_images_on_read = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "reset-invalid-pixels"))
         || (match_pattern(lc_option, "rip"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("reset-invalid-pixels = %s\n",
                    fs_reset_invalid_pixels ? "t" : "f")); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Invalid pixels %s re-computed when kiff images ",
                    fs_reset_invalid_pixels ? "are" : "are not"));
            ERE(pso("read (rip=%s)\n", fs_reset_invalid_pixels? "t"   : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_reset_invalid_pixels = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "invalidate-negative-pixels"))
         || (match_pattern(lc_option, "inp"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("invalidate-negative-pixels = %s\n",
                    fs_invalidate_negative_pixels ? "t" : "f")); 
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_invalidate_negative_pixels)
            {
                ERE(pso("When reading kiff images WITHOUT validity, "));
                ERE(pso("negative pixels are treated as\n"));
                ERE(pso("     invalid positive ones (inp=t).\n"));
            }
            else
            {
                ERE(pso("When reading kiff images WITHOUT validity, "));
                ERE(pso("negative pixels are not\n"));
                ERE(pso("assumed to indicate invalid ones (inp=f).\n"));
            }
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_invalidate_negative_pixels = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "input-gamma-before-offset"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("input-gamma-before-offset = %5.2f\n",
                    fs_input_gamma_before_offset)); 
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_input_gamma_before_offset > 0.0)
            {
                ERE(pso("Input gamma before offset is %5.2f.\n",
                        fs_input_gamma_before_offset));
            }
            else
            {
                ERE(pso("Input gamma before offset is not being used.\n"));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_input_gamma_before_offset = DBL_NOT_SET;
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_input_gamma_before_offset = temp_double;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "input-gamma-after-offset"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("input-gamma-after-offset = %5.2f\n",
                    fs_input_gamma_after_offset)); 
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_input_gamma_after_offset > 0.0)
            {
                ERE(pso("Input gamma after offset is %5.2f.\n",
                        fs_input_gamma_after_offset));
            }
            else
            {
                ERE(pso("Input gamma after offset is not being used.\n"));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_input_gamma_after_offset = DBL_NOT_SET;
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_input_gamma_after_offset = temp_double;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "linearize-pcd"))
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("linearize-pcd = %s\n",
                    fs_linearize_pcd ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("PCD linearization %s applied duing ycc conversion.\n",
                    fs_linearize_pcd ? "is" : "is not"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_linearize_pcd = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "strip-image"))
         || (match_pattern(lc_option, "strip"))
       )
    {
        IMPORT int export_strip_top; /* A hack for the ARC code. */
        IMPORT int export_strip_bottom; /* A hack for the ARC code. */
        IMPORT int export_strip_left; /* A hack for the ARC code. */
        IMPORT int export_strip_right; /* A hack for the ARC code. */

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("strip-image-top = %d\n", fs_strip_top));
            ERE(pso("strip-image-bottom = %d\n", fs_strip_bottom));
            ERE(pso("strip-image-left = %d\n", fs_strip_left));
            ERE(pso("strip-image-right = %d\n", fs_strip_right));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("The top %d image rows are stripped on image read.\n",
                    fs_strip_top));
            ERE(pso("The bottom %d image rows are stripped on image read.\n",
                    fs_strip_bottom));
            ERE(pso("The left %d image rows are stripped on image read.\n",
                    fs_strip_left));
            ERE(pso("The right %d image rows are stripped on image read.\n",
                    fs_strip_right));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int < 0) temp_int = 0;

            fs_strip_top = temp_int;
            export_strip_top = fs_strip_top;

            fs_strip_bottom = temp_int;
            export_strip_bottom = fs_strip_bottom;

            fs_strip_left = temp_int;
            export_strip_left = fs_strip_left;

            fs_strip_right = temp_int;
            export_strip_right = fs_strip_right;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "strip-image-top"))
         || (match_pattern(lc_option, "strip-top"))
       )
    {
        IMPORT int export_strip_top; /* A hack for the ARC code. */

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("strip-image-top = %d\n", fs_strip_top));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("The top %d image rows are stripped on image read.\n",
                    fs_strip_top));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int < 0) temp_int = 0;
            fs_strip_top = temp_int;
            export_strip_top = fs_strip_top;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "strip-image-bottom"))
         || (match_pattern(lc_option, "strip-bottom"))
         || (match_pattern(lc_option, "strip-bot"))
         || (match_pattern(lc_option, "strip-bottem"))
       )
    {
        IMPORT int export_strip_bottom; /* A hack for the ARC code. */

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("strip-image-bottom = %d\n", fs_strip_bottom));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("The bottom %d image rows are stripped on image read.\n",
                    fs_strip_bottom));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int < 0) temp_int = 0;
            fs_strip_bottom = temp_int;
            export_strip_bottom = fs_strip_bottom;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "strip-image-left"))
         || (match_pattern(lc_option, "strip-left"))
       )
    {
        IMPORT int export_strip_left; /* A hack for the ARC code. */

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("strip-image-left = %d\n", fs_strip_left));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("The left %d image columns are stripped on image read.\n",
                    fs_strip_left));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int < 0) temp_int = 0;
            fs_strip_left = temp_int;
            export_strip_left = fs_strip_left;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "strip-image-right"))
         || (match_pattern(lc_option, "strip-right"))
       )
    {
        IMPORT int export_strip_right; /* A hack for the ARC code. */

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("strip-image-right = %d\n", fs_strip_right));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("The right %d image columns are stripped on image read.\n",
                    fs_strip_right));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int < 0) temp_int = 0;
            fs_strip_right = temp_int;
            export_strip_right = fs_strip_right;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "tiff-image-write-bps"))
         || (match_pattern(lc_option, "tiff-image-write-bits-per-sample"))
         || (match_pattern(lc_option, "tiff-image-bps"))
         || (match_pattern(lc_option, "tiff-image-bpp"))
         || (match_pattern(lc_option, "tiff-image-bits-per-sample"))
         || (match_pattern(lc_option, "tiff-bps"))
         || (match_pattern(lc_option, "tiff-bpp"))
         || (match_pattern(lc_option, "tiff-bits-per-sample"))
         || (match_pattern(lc_option, "tif-image-write-bps"))
         || (match_pattern(lc_option, "tif-image-write-bits-per-sample"))
         || (match_pattern(lc_option, "tif-image-bps"))
         || (match_pattern(lc_option, "tif-image-bpp"))
         || (match_pattern(lc_option, "tif-image-bits-per-sample"))
         || (match_pattern(lc_option, "tif-bps"))
         || (match_pattern(lc_option, "tif-bpp"))
         || (match_pattern(lc_option, "tif-bits-per-sample"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("tiff-image-write-bps = %d\n", fs_tiff_write_bps));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Tiff images are written using %d bits per sample.\n",
                    fs_tiff_write_bps));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int < 0) temp_int = 0;

            if ((temp_int != 8) && (temp_int != 16))
            {
                set_error("\"fs_tiff_write_bps\" must be either 8 or 16.");
                add_error("A value of %d is invalid.", temp_int);
                return ERROR;
            }

            fs_tiff_write_bps = temp_int;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "bloom-removal-count"))
         || (match_pattern(lc_option, "bloom"))
         || (match_pattern(lc_option, "brc"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("bloom-removal-count = %d\n",
                    fs_bloom_removal_count));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Bloom removal count (brc) is %d.\n",
                    fs_bloom_removal_count));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int < 0) temp_int = 0;
            fs_bloom_removal_count = temp_int;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "force-valid-kiff-on-read"))
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("force-valid-kiff-on-read = %s\n",
                    fs_force_valid_kiff_on_read ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Kiff validity %s ignored on read.\n",
                    fs_force_valid_kiff_on_read ? "is" : "is not"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_force_valid_kiff_on_read = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if ((result == NO_ERROR) && (value[ 0 ] != '\0'))
    {
        call_image_data_invalidation_fn();
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_image_output_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  temp_boolean_value;
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "write-validity-kiff"))
         || (match_pattern(lc_option, "wvk"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("write-validity-kiff = %s\n",
                    fs_write_validity_kiff ? "t" : "f")); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Kiff file format is %s.\n",
                    fs_write_validity_kiff ? "float_with_validity" : "float"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_write_validity_kiff = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || match_pattern(lc_option, "do-gamma-correction")
         || match_pattern(lc_option, "gamma")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("do-gamma-correction = %s\n",
                    fs_do_gamma_correction ? "t" : "f")); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Images are displayed %s gamma correction.\n",
                    fs_do_gamma_correction ? "with" : "without"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_do_gamma_correction = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "display-log-image"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("display-log-image = %s\n",
                    fs_display_log ? "t" : "f")); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Logarithm of images %s displayed.\n",
                    fs_display_log ? "are" : "are not"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_display_log = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "scale-by-max-rgb"))
         || (match_pattern(lc_option, "scale-image-by-max-rgb"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("scale-image-by-max-rgb = %s\n",
                    fs_scale_by_max_rgb ? "t" : "f")); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Images for display %s scaled so that the max is 255.\n",
                    fs_scale_by_max_rgb ? "are" : "are not"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_scale_by_max_rgb = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if ((lc_option[ 0 ] == '\0') || match_pattern(lc_option, "adjust-image-range"))
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("adjust-image-range = %s\n",
                    fs_adjust_image_range ? "t" : "f")); 
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Images for display %s scaled so min is 0 nad max is 255.\n",
                    fs_adjust_image_range ? "are" : "are not"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_adjust_image_range = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "display-matrix-file"))
         || (match_pattern(lc_option, "dmf"))
       )
    {
        if (value == NULL) return NO_ERROR;

        result = set_display_matrix(value);
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "ipc"))
         || (match_pattern(lc_option, "invalid-pixel-colour"))
       )
    {
        if (value == NULL) return NO_ERROR;

        result = set_invalid_pixel_colour(value);
    }

    /* Lindsay - Nov 18, 1999 */
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "hdrc-write-coords"))
         || (match_pattern(lc_option, "hdrc-write-pixel-coords"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hdrc-write-pixel-coords=%s\n",
                    fs_hdrc_enable_write_pixel_coords ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HDRC pixel coords greater than threshold %s are written.\n",
                     fs_hdrc_enable_write_pixel_coords ? "are" : "are not"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_hdrc_enable_write_pixel_coords = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "hdrc-write-thresh"))
         || (match_pattern(lc_option, "hdrc-write-threshold"))
       )
    {
        float temp;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hdrc-write-threshold=%6.1f\n",
                    (double)fs_hdrc_pixel_threshold_value));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HDRC pixel coords threshold valie is %6.1f.\n",
                    (double)fs_hdrc_pixel_threshold_value));
        }
        else
        {
            ERE(ss1f(value, &temp));
            fs_hdrc_pixel_threshold_value = temp;
        }
        result = NO_ERROR;
    }
    /* End Lindsay */

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_invalid_pixel_colour(const char* value)
{
    int temp_boolean_value;


    if (*value == '\0')
    {
        return print_invalid_pixel_colour();
    }
    else if (*value == '?')
    {
        return print_invalid_pixel_colour_option();
    }

    temp_boolean_value = get_boolean_value(value);

    if ((temp_boolean_value != ERROR) && ( ! temp_boolean_value))
    {
        fs_invalid_pixel_colour.r = FLT_NOT_SET;
        fs_invalid_pixel_colour.g = FLT_NOT_SET;
        fs_invalid_pixel_colour.b = FLT_NOT_SET;
    }
    else
    {
        ERE(string_scan_float_rgb(value, &fs_invalid_pixel_colour));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int print_invalid_pixel_colour(void)
{
    if (    ((fs_invalid_pixel_colour.r) < FLT_ZERO)
         || ((fs_invalid_pixel_colour.g) < FLT_ZERO)
         || ((fs_invalid_pixel_colour.b) < FLT_ZERO)
       )
    {
        ERE(pso("Invalid pixels are displayed like valid ones.\n"));
    }
    else
    {
        ERE(pso("Invalid pixel colour is (%d, %d, %d)\n",
                kjb_rintf(fs_invalid_pixel_colour.r), 
                kjb_rintf(fs_invalid_pixel_colour.g), 
                kjb_rintf(fs_invalid_pixel_colour.b))); 
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int print_invalid_pixel_colour_option(void)
{
    if (    ((fs_invalid_pixel_colour.r) < FLT_ZERO)
         || ((fs_invalid_pixel_colour.g) < FLT_ZERO)
         || ((fs_invalid_pixel_colour.b) < FLT_ZERO)
       )
    {
        ERE(pso("ipc = off\n"));
    }
    else
    {
        ERE(pso("ipc = %d,%d,%d\n",
                kjb_rintf(fs_invalid_pixel_colour.r), 
                kjb_rintf(fs_invalid_pixel_colour.g), 
                kjb_rintf(fs_invalid_pixel_colour.b))); 
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_display_matrix(const char* file_name)
{
    static int first_time = TRUE;
    Matrix*    temp_mp    = NULL;


    if (*file_name == '\0')
    {
        ERE(print_display_matrix());
        return NO_ERROR;
    }
    else if (*file_name == '?')
    {
        ERE(pso("display-matrix-file = <file>\n"));
        return NO_ERROR;
    }

    if (first_time)
    {
        add_cleanup_function(free_display_matrix);
        first_time = FALSE;
    }

    if (is_no_value_word(file_name))
    {
        free_matrix(fs_display_matrix_mp);
        fs_display_matrix_mp = NULL;
        return NO_ERROR;
    }

    ERE(read_matrix(&temp_mp, file_name));

    if ((temp_mp->num_rows != 3) || (temp_mp->num_cols != 3))
    {
        free_matrix(temp_mp);
        set_error("The display matrix must be a three by three matrix");
        return ERROR;
    }

    ERE(copy_matrix(&fs_display_matrix_mp, temp_mp));

    free_matrix(temp_mp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int print_display_matrix(void)
{
    if (fs_display_matrix_mp == NULL)
    {
        ERE(pso("No display matrix is currently set.\n"));
    }
    else
    {
        ERE(pso("Display matrix is :\n"));
        ERE(fp_write_matrix(fs_display_matrix_mp, stdout));
        ERE(pso("\n"));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void free_display_matrix(void)
{
    free_matrix(fs_display_matrix_mp);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              kjb_read_image
 *
 * Reads an image into the kjb image structure
 *
 * This routine reads an image from a file into the kjb image structure which is
 * a floating point precession RGB format. It also does pre-processing under the
 * control of options which have normally been set through calls to
 * process_kjb_options(3) or kjb_set(3), normally with user input, but sometimes
 * with hard coded constant strings. For reading without pre-processing, see
 * kjb_read_image_2.
 *
 * This routine natively reads sun-raster, simple jpeg, simple tiff, and the
 * home grown floating point formats kiff, and mid. In addition, it will arrange
 * conversion from most other 24-bit formats, provided that a conversion program
 * is available.
 *
 * Currently the pre-processing (optionally) done includes offset removal,
 * linearization, removing fixed pattern noise, correcting for fixed gradients
 * due to optics, bloom removal, striping of images exactly the videograbber
 * size, striping images by a specifiied amount,  marking clipped pixels as
 * invald, and marking dark pixels as invalid.
 *
 * It is dangerous to rely on defaults of these behaviours, but currently the
 * default is to remove the offset only if the required configuration files are
 * in place, NOT to remove bloom, NOT to strip videograbber size images, to mark
 * clipped pixels (ones that are 255 or greater) EVEN in the case of float kiff
 * as explained below, and to mark dark pixels.
 *
 * Float kiff images store validity information as well as pixel values. By
 * default, this stored validity information is used instead the validity being
 * recomputed based on the clip point and dark pixel levels. To obtain the
 * equally useful alternate behaviour, whereby the validity information is
 * recomputed, use the fs_reset_invalid_pixels option (rip=t).
 *
 * Warning:
 *     It is dangerous to rely on defaults. They change, and it is hard to keep
 *     documentation in sync. If it makes a difference to the application, it
 *     is best to set the options formally.
 *
 * Warning:
 *     The fancy nature of this routine has caused confusion. It is quite
 *     useful, but often starting with kjb_read_image_2 is better. However, if
 *     this routine can be used to manipulate images on read under user control
 *     which has a lot of advantages in some cirumstances.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure, with an error message being set.
 *
 * Index : images, image I/O, float images, kiff
 *
 * -----------------------------------------------------------------------------
*/

int kjb_read_image
(
    KJB_image** ipp,
    const char* file_name_gz_and_sub_image
)
{
    int result                         = NO_ERROR;
    int validity_data_is_part_of_image = NOT_SET;


    ERE(kjb_read_image_3(ipp, file_name_gz_and_sub_image,
                         &validity_data_is_part_of_image));

    if (    (result != ERROR)
         && (kjb_get_verbose_level() >= 5)
       )
    {
        verbose_pso(5, "Image read from %s has %d invalid pixels before pre-processing.\n",
                    file_name_gz_and_sub_image,
                    count_invalid_pixels(*ipp));
    }

    if (    (result != ERROR)
         && (fs_strip_images_on_read)
         && ((*ipp)->num_rows == FULL_VIDEO_NUM_ROWS)
         && ((*ipp)->num_cols == FULL_VIDEO_NUM_COLS)
       )
    {
        KJB_image* temp_ip = NULL;

        result = get_image_window(&temp_ip, *ipp,
                                  STRIP_TOP, STRIP_LEFT,
                                  STRIPPED_VIDEO_NUM_ROWS,
                                  STRIPPED_VIDEO_NUM_COLS);

        if (result == ERROR)
        {
            kjb_free_image(temp_ip);
        }
        else
        {
            kjb_free_image(*ipp);
            *ipp = temp_ip;
        }
    }

    /*
    // We mark clipped pixels as invalid unless invalid pixels are already part
    // of the image, and only then, if we do not explicitly reset invalid
    // pixels. Resetting invalid pixels over-rides the existing validity data.
    */
    if (    (result != ERROR)
         && (    (fs_reset_invalid_pixels)
              || ( ! validity_data_is_part_of_image)
            )
       )
    {
        result = mark_clipped_pixels(*ipp);
    }

    if (    (result != ERROR)
             /* If space is YCC, then this does not make so much sense. */
         && ( ! fs_convert_pcd_ycc )
       )
    {
        int i = 0;

        while ((i<fs_bloom_removal_count) && (result != ERROR))
        {
            KJB_image* temp_ip = NULL;

            if (mark_blooming_candidates(&temp_ip, *ipp) == ERROR)
            {
                kjb_free_image(temp_ip);
                result = ERROR;
            }
            else
            {
                kjb_free_image(*ipp);
                *ipp = temp_ip;
            }

            i++;
        }
    }

    if (fs_convert_pcd_ycc)
    {
        /*
        // July 14, 2002: This conversion now does not do linearization NOR
        // shaping. Also, moved to after marking of clipped pixels and bloom
        // removal.
        */
        ERE(ow_convert_pcd_ycc_to_rgb(*ipp));
    }

    /*
    // July 14, 2002
    //
    // It use to be like this, then we put linearization into the conversion,
    // and now it is back to how it was.
    */
    if ((result != ERROR) && (fs_linearize_pcd))
    {
        if (    (fs_input_gamma_before_offset > 0.0)
             || (fs_input_gamma_after_offset  > 0.0)
           )
        {
            warn_pso("Linearization of PCD is over-riding other input ");
            warn_pso("gamma correction\n");
        }

        result = ow_linearize_pcd(*ipp);
    }

    if (    (result != ERROR)
         && (fs_input_gamma_before_offset > 0.0)
         && ( ! fs_linearize_pcd )
       )
    {
        Vector* gamma_vp = NULL;

        result = get_initialized_vector(&gamma_vp, 3,
                                        1.0 / fs_input_gamma_before_offset);

        if (result != ERROR) result = ow_gamma_correct_image(*ipp, gamma_vp);

        free_vector(gamma_vp);
    }

    if ((result != ERROR) && ( ! fs_linearize_pcd ))
    {
        result = remove_camera_offset_from_image(*ipp);
    }

    if ((result != ERROR) && ( ! fs_linearize_pcd ))
    {
        result = ow_demosaic(*ipp);
    }

    if (    (result != ERROR)
         && (fs_input_gamma_after_offset > 0.0)
         && ( ! fs_linearize_pcd )
       )
    {
        Vector* gamma_vp = NULL;

        result = get_initialized_vector(&gamma_vp, 3,
                                        1.0 / fs_input_gamma_after_offset);

        if (result != ERROR) result = ow_gamma_correct_image(*ipp, gamma_vp);

        free_vector(gamma_vp);
    }

    /*
    // We mark the invalid dark pixels unless they are part of the image, and
    // only then, if we do not explicitly reset invalid pixels. Resetting
    // invalid pixels over-rides the existing validity data.
    */
    if (    (result != ERROR)
         && (    (fs_reset_invalid_pixels)
              || ( ! validity_data_is_part_of_image)
            )
       )
    {
        result = mark_dark_pixels(*ipp);
    }

    if (    (result != ERROR)
         && (kjb_get_verbose_level() >= 5)
       )
    {
        verbose_pso(5, "Image read from %s has %d invalid pixels after pre-processing.\n",
                    file_name_gz_and_sub_image,
                    count_invalid_pixels(*ipp));
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              kjb_read_image_2
 *
 * Reads an image into the kjb image data structure
 *
 * This routine reads an image from a file into the kjb image structure which is
 * a floating point RGB representation. For doing so with various pre-processing
 * options, see kjb_read_image.
 *
 * This routine natively reads sun-raster, simple jpeg, simple tiff, and the
 * home grown floating point formats kiff, and mid. In addition, it will arrange
 * conversion from most other 24-bit formats, provided that a conversion program
 * is available.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure, with an error message being set.
 *
 * Index : images, image I/O, float images, kiff
 *
 * -----------------------------------------------------------------------------
*/

int kjb_read_image_2
(
    KJB_image** ipp,
    const char* file_name_and_sub_image
)
{
    return kjb_read_image_3(ipp, file_name_and_sub_image, (int*)NULL);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int kjb_read_image_3
(
    KJB_image** ipp,
    const char* file_name_and_sub_image,
    int* validity_data_is_part_of_image_ptr
)
{
    FILE*         fp = NULL;
    kjb_uint32        magic_number;
    kjb_uint32        reversed_magic_number;
    int           result = NO_ERROR;
#ifdef REGRESS_COMPRESSION /* kjb_fopen now uncompresses */
    size_t        file_name_gz_len;
    char          file_name_gz_and_sub_image_copy[ MAX_FILE_NAME_SIZE ];
    char*         file_name_gz_and_sub_image_pos;
    char          file_name_gz[ MAX_FILE_NAME_SIZE ];
    char          uncompressed_temp_name[ MAX_FILE_NAME_SIZE ];
#endif
    char          file_name[ MAX_FILE_NAME_SIZE ];
    char          sub_image[ MAX_FILE_NAME_SIZE ];
    const char*   file_name_and_sub_image_pos;
    int           validity_data_is_part_of_image  = FALSE;
    off_t         file_size         = 0;
    int pcd_image = FALSE;
    int pcd_rotate = 0;


#ifdef REGRESS_COMPRESSION /* kjb_fopen now uncompresses */
    uncompressed_temp_name[ 0 ] = '\0';

    /*
    // We may have to add or insert error without being sure that the error
    // message list is clean, so clear it.
    */
    kjb_clear_error();

    BUFF_CPY(file_name_gz_and_sub_image_copy, file_name_gz_and_sub_image);
    file_name_gz_and_sub_image_pos = file_name_gz_and_sub_image_copy;
    BUFF_GEN_GET_TOKEN(&file_name_gz_and_sub_image_pos, file_name_gz, "[");
    BUFF_CPY(sub_image, file_name_gz_and_sub_image_pos);

    file_name_gz_len = strlen(file_name_gz);
#else
    file_name_and_sub_image_pos = file_name_and_sub_image;
    BUFF_CONST_GEN_GET_TOKEN(&file_name_and_sub_image_pos, file_name, "[");
    BUFF_CPY(sub_image, file_name_and_sub_image_pos);
#endif

#ifdef REGRESS_COMPRESSION /* kjb_fopen now uncompresses */
    if (    (file_name_gz_len > 3)
         && (    (    (file_name_gz[ file_name_gz_len - 3 ] == '.')
                   && (file_name_gz[ file_name_gz_len - 2 ] == 'g')
                   && (file_name_gz[ file_name_gz_len - 1 ] == 'z')
                 )
              ||
                 (    (file_name_gz[ file_name_gz_len - 2 ] == '.')
                   && (file_name_gz[ file_name_gz_len - 1 ] == 'Z')
                 )
            )
       )
    {
        char        command[ 20 + 2 * MAX_FILE_NAME_SIZE ];
        const char* suffix = (file_name_gz[ file_name_gz_len - 1 ] == 'z')
                                                                ? ".gz" : ".Z";
        char        compressed_temp_name[ MAX_FILE_NAME_SIZE ];

        compressed_temp_name[ 0 ] = '\0';

        BUFF_GET_TEMP_FILE_NAME(uncompressed_temp_name);
        BUFF_CPY(compressed_temp_name, uncompressed_temp_name);
        BUFF_CAT(compressed_temp_name, suffix);

        ERE(kjb_sprintf(command, sizeof(command), "/bin/cp %s %s",
                        file_name_gz, compressed_temp_name));
        ERE(kjb_system(command));

        result = kjb_sprintf(command, sizeof(command), "gunzip %s",
                             compressed_temp_name);

        if (result != ERROR)
        {
            verbose_pso(10, "Uncompressing a copy of %s as %s.\n",
                        file_name_gz, uncompressed_temp_name);
            result = kjb_system(command);
        }

        if (result != ERROR)
        {
            verbose_pso(10, "Uncompression seems to have succeeded.\n");
            BUFF_CPY(file_name, uncompressed_temp_name);
        }
        else
        {
            (void)kjb_unlink(compressed_temp_name);  /* Just in case. */
        }

    }
    else
    {
        BUFF_CPY(file_name, file_name_gz);
    }
#endif

    if (result != ERROR)
    {
        fp = kjb_fopen(file_name, "rb");
        if (fp == NULL) { NOTE_ERROR(); result = ERROR; }
    }

    if (result != ERROR)
    {
        result = FIELD_READ(fp, magic_number);

        if (    (result != ERROR)
             && (kjb_fseek(fp, 0L, SEEK_SET) == ERROR)
           )
        {
            result = ERROR;
        }
    }

    if (result != ERROR)
    {
        reverse_four_bytes(&magic_number, &reversed_magic_number);

        result = fp_get_byte_size(fp, &file_size);
    }

    if (    (result != ERROR)
         && (file_size > 4 + PCD_MAGIC_NUM_OFFSET)
         && (file_size > 1 + PCD_ROTATE_OFFSET)
       )
    {
        char pcd_str[ 4 ];

        result = kjb_fseek(fp, PCD_MAGIC_NUM_OFFSET, SEEK_SET);

        if (result != ERROR)
        {
            result = FIELD_READ(fp, pcd_str);
        }

        if (    (result != ERROR)
             && (fs_internal_pcd_read)
             && (STRNCMP_EQ(pcd_str, "PCD_", 4))
           )
        {
            unsigned char rotate_str[ 1 ];

            result = kjb_fseek(fp, PCD_ROTATE_OFFSET, SEEK_SET);

            if (result != ERROR)
            {
                result = FIELD_READ(fp, rotate_str);

                pcd_rotate = rotate_str[ 0 ] & 0x03;
                pcd_image = TRUE;

                dbi(pcd_rotate);
            }
        }

        if (kjb_fseek(fp, 0L, SEEK_SET) == ERROR)
        {
            result = ERROR;
        }
    }

    /*
    dbx(magic_number);
    dbx(reversed_magic_number);
    dbx(JPEG_MAGIC_NUM);
    */

    if (result == ERROR)
    {
        /*EMPTY*/
        ; /* Do nothing. */
    }
    else if (pcd_image)
    {
        result = read_image_from_pcd(ipp, file_name, sub_image,
                                     pcd_rotate);

        if (result == NOT_FOUND)
        {
#ifdef TEST
            char error_buff[ 1000 ];

            kjb_get_error(error_buff, sizeof(error_buff)); 
            TEST_PSO(("\n"));
            TEST_PSO((error_buff)); 
            TEST_PSO(("\n"));
            TEST_PSO(("\n"));
#endif
            verbose_pso(2, "Limited PCD reader can't read the image.\n");
            verbose_pso(2, "Conversion will be attempted.\n");
        }
    }
    else if (file_size == RAW_DCS_460_16_BIT_SIZE)
    {
        result = read_image_from_raw_dcs_460_16_bit(ipp, fp);
    }
    /* Lindsay - Sept 28, 1999 */
    else if (file_size == HDRC_RAW_16_BIT_SIZE)
    {
        verbose_pso(5, "10-bit HDRC data detected in %F\n", fp);

        result = read_image_from_raw_hdrc_16_bit(ipp, fp);

        if (result != ERROR && fs_hdrc_enable_write_pixel_coords)
        {
            char img_name[ MAX_FILE_NAME_SIZE ];

            (void)kjb_sprintf(img_name, MAX_FILE_NAME_SIZE, "%F", fp);

            result = write_threshold_pixel_coords(*ipp,
                                                  img_name,
                                         (double)fs_hdrc_pixel_threshold_value);
        }

        if (result != ERROR)
        {
            result = ow_correct_hdrc_fixed_pixels(*ipp);
        }
    }
    /* End Lindsay - Sept 28, 1999 */
    else if (    (magic_number == KIFF_MAGIC_NUM)
              || (magic_number == OLD_KIFF_MAGIC_NUM)
              || (reversed_magic_number == KIFF_MAGIC_NUM)
              || (reversed_magic_number == OLD_KIFF_MAGIC_NUM)
            )
    {
        result = read_image_from_kiff(ipp, fp,
                                      &validity_data_is_part_of_image);
    }
    else if (    (magic_number == MID_MAGIC_NUM)
              || (reversed_magic_number == MID_MAGIC_NUM)
            )
    {
        result = read_image_from_MID_file(ipp, fp);
    }
    else if (    (magic_number == TIFF_LSB_FIRST_MAGIC_NUM)
              || (magic_number == TIFF_MSB_FIRST_MAGIC_NUM)
              || (reversed_magic_number == TIFF_LSB_FIRST_MAGIC_NUM)
              || (reversed_magic_number == TIFF_MSB_FIRST_MAGIC_NUM)
            )
    {
#ifdef KJB_HAVE_TIFF
        verbose_pso(5, "Attempting to read tiff file with builtin routine.\n");

        result = read_image_from_tiff(ipp, fp);

        if (result == NO_ERROR)
        {
            verbose_pso(5, "Read of tiff file with builtin routine succeeded.\n");
        }
        else
        {
            verbose_pso(5, "Read of tiff file with builtin routine failed.\n");
        }

        if (result == NOT_FOUND)
        {
#ifdef TEST
            /* char error_buff[ 1000 ]; */

            add_error("We will attempt to convert the TIFF to a different format and read that instead.");
            kjb_print_error(); 

            /*
            kjb_get_error(error_buff, sizeof(error_buff)); 
            TEST_PSO(("\n"));
            TEST_PSO((error_buff)); 
            TEST_PSO(("\n"));
            TEST_PSO(("\n"));
            */
#endif 
            conversion_explanation_str = "Limited TIFF reader returned error reading the file";
            /*
            verbose_pso(2, "Limited TIFF reader can't read the image.\n");
            verbose_pso(2, "Conversion will be attempted.\n");
            */
        }
#else
        conversion_explanation_str = "This appears to be a tiff file but the code is not built with libtiff .";
        result = NOT_FOUND;
#endif
    }
#ifndef __C2MAN__
    else if (    (magic_number>>16 == JPEG_MAGIC_NUM)
              || (reversed_magic_number>>16 == JPEG_MAGIC_NUM)
            )
    {
#ifdef KJB_HAVE_JPEG
        result = read_image_from_jpeg(ipp, fp);

        if (result == NOT_FOUND)
        {
#ifdef TEST
            /* char error_buff[ 1000 ]; */

            add_error("We will attempt to convert the JPEG to different format and read that instead.");
            kjb_print_error(); 
            /*
            kjb_get_error(error_buff, sizeof(error_buff)); 
            TEST_PSO(("\n"));
            TEST_PSO((error_buff)); 
            TEST_PSO(("\n"));
            TEST_PSO(("\n"));
            */
#endif
            /* verbose_pso(2, "Limited JPEG reader can't read the image.\n"); */
            /* verbose_pso(2, "Conversion will be attempted.\n"); */
            conversion_explanation_str = "Limited JPEG reader returned error reading the file";
        }
#else 
        conversion_explanation_str = "This appears to be a jpeg file but the code is not built with libjpeg";
        result = NOT_FOUND;
#endif
    }
#endif
    else if (    (magic_number == RASTER_MAGIC_NUM)
              || (reversed_magic_number == RASTER_MAGIC_NUM)
            )
    {
        result = read_image_from_raster(ipp, fp);
    }
    else if (    (((char*)&magic_number)[ 0 ] == 'P')
              && (((char*)&magic_number)[ 1 ] == '6')
            )
    {
        result = read_image_from_pnm_P6(ipp, fp);
    }
    else if (
                 (    (((char*)&magic_number)[ 0 ] == 'B')
                   && (((char*)&magic_number)[ 1 ] == 'M')
                 )
#ifdef THIS_DOES_NOT_MAKE_SENSE
              ||
                 (    (((char*)&reversed_magic_number)[ 0 ] == 'B')
                   && (((char*)&reversed_magic_number)[ 1 ] == 'M')
                 )
#endif
            )
    {
        result = read_image_from_bmp(ipp, fp);
    }
    else
    {
        conversion_explanation_str = "File format is not one we read natively";
        result = NOT_FOUND;
    }

    push_error_action(FORCE_ADD_ERROR_ON_ERROR);
    if (kjb_fclose(fp) == ERROR) result = ERROR;
    pop_error_action();

    if (result == NOT_FOUND)
    {
        FILE* ras_fp = NULL;
        char  raster_temp_name[ MAX_FILE_NAME_SIZE ];

        verbose_pso(2, "Arranging conversion of %s to temporary file in raster format.\n", file_name);

        if (conversion_explanation_str != NULL)
        {
            verbose_pso(2, "(%s).\n", conversion_explanation_str);
        }

        result = BUFF_GET_TEMP_FILE_NAME(raster_temp_name);

        if (result != ERROR)
        {
            BUFF_CAT(raster_temp_name, ".sun");
            BUFF_CAT(file_name, sub_image);
            result = convert_image_file_to_raster(file_name, raster_temp_name);
        }

        if (result != ERROR) {
           verbose_pso(2, "Conversion to raster succeeded.\n");
           ras_fp = kjb_fopen(raster_temp_name, "rb");
        }

        if (ras_fp == NULL) result = ERROR;
        else
        {
            result = read_image_from_raster(ipp, ras_fp);

            if (result == ERROR)
            {
                insert_error("Failed to read converted temporary image.");
            }
            else 
            {
                verbose_pso(2, "Read of temporary raster image file succeeded.\n");
            }
        }

        push_error_action(FORCE_ADD_ERROR_ON_ERROR);

        if (kjb_fclose(ras_fp) == ERROR) result = ERROR;
        if (kjb_unlink(raster_temp_name) == ERROR) result = ERROR;

        pop_error_action();
    }

    if (validity_data_is_part_of_image_ptr != NULL)
    {
        *validity_data_is_part_of_image_ptr = validity_data_is_part_of_image;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_image_from_raster(KJB_image** ipp, FILE* fp)
{
    Sun_header sun_header;
    kjb_uint32 magic_number;
    kjb_uint32 reversed_magic_number;
    kjb_int32  temp_int32;
    KJB_image* ip;
    int        i, j, num_rows, num_cols;
    int        row_length;
    off_t      raster_size;
    off_t      file_size;
    unsigned char*     data_row;
    unsigned char*     data_row_pos;
    Pixel*     out_pos;
    int        result;


    ERE(FIELD_READ(fp, magic_number));
    reverse_four_bytes(&magic_number, &reversed_magic_number);

    if (magic_number == RASTER_MAGIC_NUM)
    {
        ERE(FIELD_READ(fp, sun_header.width));
        ERE(FIELD_READ(fp, sun_header.height));
        ERE(FIELD_READ(fp, sun_header.depth));
        ERE(FIELD_READ(fp, sun_header.length));
        ERE(FIELD_READ(fp, sun_header.type));
        ERE(FIELD_READ(fp, sun_header.maptype));
        ERE(FIELD_READ(fp, sun_header.maplength));
    }
    else if (reversed_magic_number == RASTER_MAGIC_NUM)
    {
        ERE(FIELD_READ(fp, temp_int32));
        reverse_four_bytes(&temp_int32, &(sun_header.width));
        ERE(FIELD_READ(fp, temp_int32));
        reverse_four_bytes(&temp_int32, &(sun_header.height));
        ERE(FIELD_READ(fp, temp_int32));
        reverse_four_bytes(&temp_int32, &(sun_header.depth));
        ERE(FIELD_READ(fp, temp_int32));
        reverse_four_bytes(&temp_int32, &(sun_header.length));
        ERE(FIELD_READ(fp, temp_int32));
        reverse_four_bytes(&temp_int32, &(sun_header.type));
        ERE(FIELD_READ(fp, temp_int32));
        reverse_four_bytes(&temp_int32, &(sun_header.maptype));
        ERE(FIELD_READ(fp, temp_int32));
        reverse_four_bytes(&temp_int32, &(sun_header.maplength));
    }
    else
    {
        set_error("%F is not a valid raster file (incorrect magic number).",
                  fp);
        return ERROR;
    }

    /*
     * Check for each kind of raster image that we support. If we fall through,
     * then we don't support it.
    */

    if (    (sun_header.maptype == RMT_NONE)
         &&
            (    (sun_header.type == RT_STANDARD)
              || (sun_header.type == RT_FORMAT_RGB)
            )
         &&
            (    (sun_header.depth == 8)
              || (sun_header.depth == 24)
              || (sun_header.depth == 32)
            )
       )
    {
#ifdef TEST
        if (sun_header.depth == 8)
        {
            /* Have not seen grey scale with no color map */
            UNTESTED_CODE();
        }
#endif
        /*EMPTY*/
        ;  /* Do nothing. */
    }
    else if (    (sun_header.maptype == RMT_EQUAL_RGB)
              &&
                 (    (sun_header.type == RT_STANDARD)
                   || (sun_header.type == RT_FORMAT_RGB)
                 )
              &&
                 /* So far, we only deal with gray scale with full color maps
                  * (which can be ignored) */
                 (    (sun_header.depth == 8)
                   && (sun_header.maplength == 768)
                 )
            )
    {
        /*EMPTY*/
        ;  /* Do nothing. */
    }
    else
    {
        dbi(sun_header.type);
        dbi(sun_header.maptype);
        dbi(sun_header.maplength);
        dbi(sun_header.depth);

        set_error("Raster file is not a type we read yet.");
        return ERROR;
    }

    num_rows = sun_header.height;
    num_cols = sun_header.width;

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    /*
    // Documentation suggests that lines are rounded to nearest 16 bits. In the
    // two simple cases of 24 or 32 bit images, this simply means making sure
    // that row_length is even in the case of 24 bits.
    */

    if (sun_header.depth == 8)
    {
        row_length = num_cols;

        if (IS_ODD(row_length))
        {
            row_length++;
        }
    }
    else if (sun_header.depth == 24)
    {
        row_length = 3 * num_cols;

        if (IS_ODD(row_length))
        {
            row_length++;
        }
    }
    else
    {
        row_length = 4 * num_cols;
    }

    raster_size = row_length * num_rows + 32 + sun_header.maplength;

    ERE(fp_get_byte_size(fp, &file_size));

    if (file_size != raster_size)
    {
        set_error("%F is not a valid raster file.", fp);
        add_error("It has too %s data.",
                  (file_size > raster_size) ? "much" : "little");
        return ERROR;
    }

    NRE(data_row = BYTE_MALLOC(MAX_OF(row_length, sun_header.maplength)));

    if (sun_header.maptype > 0)
    {
        if (kjb_fread(fp, data_row, (size_t)sun_header.maplength) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }
    }

    num_rows -= fs_strip_top;
    num_rows -= fs_strip_bottom;

    num_cols -= fs_strip_left;
    num_cols -= fs_strip_right;

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F after stripping (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    result = get_target_image(ipp, num_rows, num_cols);

    if (result == ERROR)
    {
        kjb_free(data_row);
        return ERROR;
    }

    ip = *ipp;

    for(i=0; i<fs_strip_top; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }
    }

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row;
        out_pos = ip->pixels[ i ];

        if (sun_header.depth == 8)
        {
            data_row_pos += fs_strip_left;

            for(j=0; j<num_cols; j++)
            {
                out_pos->r = *data_row_pos;
                out_pos->g = *data_row_pos;
                out_pos->b = *data_row_pos;

                out_pos->extra.invalid.r     = VALID_PIXEL;
                out_pos->extra.invalid.g     = VALID_PIXEL;
                out_pos->extra.invalid.b     = VALID_PIXEL;
                out_pos->extra.invalid.pixel = VALID_PIXEL;

                out_pos++;
                data_row_pos++;
            }
        }
        else if (sun_header.type == RT_FORMAT_RGB)
        {
            if (sun_header.depth == 32)
            {
                data_row_pos += fs_strip_left;
            }

            data_row_pos += (3 * fs_strip_left);

            for(j=0; j<num_cols; j++)
            {
                if (sun_header.depth == 32) data_row_pos++;

                out_pos->r = *data_row_pos++;
                out_pos->g = *data_row_pos++;
                out_pos->b = *data_row_pos++;

                out_pos->extra.invalid.r     = VALID_PIXEL;
                out_pos->extra.invalid.g     = VALID_PIXEL;
                out_pos->extra.invalid.b     = VALID_PIXEL;
                out_pos->extra.invalid.pixel = VALID_PIXEL;

                out_pos++;
            }
        }
        else
        {
            for(j=0; j<fs_strip_left; j++)
            {
                if (sun_header.depth == 32) data_row_pos++;

                data_row_pos += 3;
            }

            for(j=0; j<num_cols; j++)
            {
                if (sun_header.depth == 32) data_row_pos++;

                out_pos->b = *data_row_pos++;
                out_pos->g = *data_row_pos++;
                out_pos->r = *data_row_pos++;

                out_pos->extra.invalid.b     = VALID_PIXEL;
                out_pos->extra.invalid.g     = VALID_PIXEL;
                out_pos->extra.invalid.r     = VALID_PIXEL;
                out_pos->extra.invalid.pixel = VALID_PIXEL;

                out_pos++;
            }
        }
    }

    kjb_free(data_row);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_TIFF

/*
#define USE_TIFF_PHOTMETRIC
*/

static int read_image_from_tiff(KJB_image** ipp, FILE* fp)
{
    TIFF* tiff_ptr;
    char  file_name[ MAX_FILE_NAME_SIZE ];
    int   num_cols, num_rows, i, j;
    int    scan_line_length;
    unsigned short bits_per_sample;
    unsigned short samples_per_pixel;
#ifdef USE_TIFF_PHOTMETRIC   /* Used for interpreting the bytes. */
    unsigned short photometric;
#endif 
    Pixel* target_row;
    tsample_t sample = 0;


    TIFFSetWarningHandler(NULL);

    BUFF_GET_FD_NAME(fileno(fp), file_name);
    tiff_ptr = TIFFOpen(file_name, "r");

    if (tiff_ptr == NULL)
    {
        /*
        // We could be out of descriptors, but this is usually a bug, anyway.
        */
        set_bug("Unable to open a file which we were already able to open.");
        return ERROR;
    }

    TIFFGetFieldDefaulted(tiff_ptr, TIFFTAG_BITSPERSAMPLE, &bits_per_sample);
#ifdef USE_TIFF_PHOTMETRIC   /* Used for interpreting the bytes. */
    TIFFGetFieldDefaulted(tiff_ptr,TIFFTAG_PHOTOMETRIC,&photometric);
    dbi(photometric); 
#endif 
    TIFFGetFieldDefaulted(tiff_ptr,TIFFTAG_SAMPLESPERPIXEL,&samples_per_pixel);

    if (    ((samples_per_pixel != 3) && (samples_per_pixel != 1))
         || ((bits_per_sample != 8) && (bits_per_sample != 16))
       )
    {
        TIFFClose(tiff_ptr);

        /*
        if (samples_per_pixel == 1)
        {
            verbose_pso(3,
                   "Black and white tiff files are not currently handled.\n");
        }
        */
        return NOT_FOUND;
    }

    TIFFGetField(tiff_ptr, TIFFTAG_IMAGEWIDTH, &num_cols);
    TIFFGetField(tiff_ptr, TIFFTAG_IMAGELENGTH, &num_rows);

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    num_rows -= fs_strip_top;
    num_rows -= fs_strip_bottom;

    num_cols -= fs_strip_left;
    num_cols -= fs_strip_right;

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F after stripping (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    if (get_target_image(ipp, num_rows, num_cols) == ERROR)
    {
        TIFFClose(tiff_ptr);
        return ERROR;
    }

    scan_line_length = TIFFScanlineSize(tiff_ptr);

    if (bits_per_sample == 8)
    {
        unsigned char* row;
        unsigned char* row_pos;

        row = BYTE_MALLOC(scan_line_length);

        if (row == NULL)
        {
            TIFFClose(tiff_ptr);
            return ERROR;
        }

        for (i = 0; i<num_rows; i++)
        {
            if (TIFFReadScanline(tiff_ptr, row, (kjb_uint32)(i + fs_strip_top),
                                 sample)
                == -1)
            {
                set_error("Read of tiff file %s failed.", file_name);
                kjb_free(row);
                TIFFClose(tiff_ptr);
                return ERROR;
            }

            target_row = (*ipp)->pixels[ i ];
            row_pos = row;

            row_pos += (samples_per_pixel * fs_strip_left);

            if (samples_per_pixel == 3)
            {
                for (j = 0; j<num_cols; j++)
                {
                    target_row->r = *row_pos++;
                    target_row->g = *row_pos++;
                    target_row->b = *row_pos++;

                    target_row->extra.invalid.r = VALID_PIXEL;
                    target_row->extra.invalid.g = VALID_PIXEL;
                    target_row->extra.invalid.b = VALID_PIXEL;
                    target_row->extra.invalid.pixel = VALID_PIXEL;

                    target_row++;
                }
            }
            else  /* Black and white case follows. */
            {
                /*
                 * Since we are reading it into a color image, all channels are
                 * just set to the same thing.
                */
                for (j = 0; j<num_cols; j++)
                {
                    target_row->r = *row_pos;
                    target_row->g = *row_pos;
                    target_row->b = *row_pos;

                    target_row->extra.invalid.r = VALID_PIXEL;
                    target_row->extra.invalid.g = VALID_PIXEL;
                    target_row->extra.invalid.b = VALID_PIXEL;
                    target_row->extra.invalid.pixel = VALID_PIXEL;

                    target_row++;
                    row_pos++; 
                }

            }
        }
        kjb_free(row);
    }
    else
    {
        kjb_uint16* row_16;
        kjb_uint16* row_16_pos;

        row_16 = UINT16_MALLOC(scan_line_length/2);

        if (row_16 == NULL)
        {
            TIFFClose(tiff_ptr);
            return ERROR;
        }

        for (i = 0; i<num_rows; i++)
        {
            if (TIFFReadScanline(tiff_ptr, (unsigned char*)row_16,
                                 (kjb_uint32)(i + fs_strip_top), sample) == -1)
            {
                set_error("Read of tiff file %s failed.", file_name);
                kjb_free(row_16);
                TIFFClose(tiff_ptr);
                return ERROR;
            }

            target_row = (*ipp)->pixels[ i ];
            row_16_pos = row_16;

            for(j=0; j<fs_strip_left; j++)
            {
                row_16_pos += samples_per_pixel;
            }

            if (samples_per_pixel == 3)
            {
                for (j = 0; j<num_cols; j++)
                {
                    target_row->r = *row_16_pos;
                    target_row->r /= (float)256.0;
                    row_16_pos++;

                    target_row->g = *row_16_pos;
                    target_row->g /= (float)256.0;
                    row_16_pos++;

                    target_row->b = *row_16_pos;
                    target_row->b /= (float)256.0;
                    row_16_pos++;

                    target_row->extra.invalid.r = VALID_PIXEL;
                    target_row->extra.invalid.g = VALID_PIXEL;
                    target_row->extra.invalid.b = VALID_PIXEL;
                    target_row->extra.invalid.pixel = VALID_PIXEL;

                    target_row++;
                }
            }
            else  /* Black and white case follows. */
            {
                /*
                 * Since we are reading it into a color image, all channels are
                 * just set to the same thing.
                */
                for (j = 0; j<num_cols; j++)
                {
                    float bw =  (float)*row_16_pos / (float)256.0;

                    target_row->r = bw;
                    target_row->g = bw;
                    target_row->b = bw;

                    target_row->extra.invalid.r = VALID_PIXEL;
                    target_row->extra.invalid.g = VALID_PIXEL;
                    target_row->extra.invalid.b = VALID_PIXEL;
                    target_row->extra.invalid.pixel = VALID_PIXEL;

                    target_row++;
                    row_16_pos++;
                }
            }
        }

        kjb_free(row_16);
    }

    TIFFClose(tiff_ptr);

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_JPEG
#ifndef __C2MAN__

struct my_error_mgr
{
    struct jpeg_error_mgr pub;  /* "public" fields */
    jmp_buf setjmp_buffer;      /* for return to caller */
};

typedef struct my_error_mgr* my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

static void my_error_exit(j_common_ptr cinfo)
{
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    /* Grab the message. */
    (*cinfo->err->format_message) (cinfo, jpeg_error_buff);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_image_from_jpeg(KJB_image** ipp, FILE* fp)
{
#ifdef TEST
    static int num_steps_completed = 0;  /* Static so no clobber on longjmp */
#endif
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr           jerr;
    JSAMPARRAY buffer;
    int        num_cols, num_rows, i, j;
    unsigned int       scan_line_length;
    Pixel*     target_row;
    unsigned char*      row_pos;


    kjb_clear_error();

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
#ifdef TEST
        insert_error("Completed %d steps.", num_steps_completed);
#endif
        insert_error("Limited JPEG reader failed to read %F.", fp);
        add_error(jpeg_error_buff); 
        jpeg_destroy_decompress(&cinfo);

        return NOT_FOUND;
    }

    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, fp);

    (void)jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.doc for more info.
     */

    /* Set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */

    /* Start decompressor */


    if ((cinfo.output_components != 3) || (cinfo.out_color_space != JCS_RGB)) 
    {
        /*
         * If the input image is color quantized, then
         * output_components will be 1. But we want to tell the decoder that we
         * want to have full color in the output. 
        */

#if 0
        TEST_PSO(("\nChanging JPEG cinfo.output_components from %d to 3.\n",
                  cinfo.output_components));
        TEST_PSO(("Changing JPEG cinfo.out_color_space from %d to %d.\n\n",
                  cinfo.out_color_space, JCS_RGB));
#endif

        cinfo.output_components = 3;
        cinfo.out_color_space = JCS_RGB;
    }

    (void)jpeg_start_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */
#ifdef TEST
    num_steps_completed = 1;
#endif

    num_rows = cinfo.output_height;
    num_cols = cinfo.output_width;

#ifdef HOW_IT_WAS    
    if (cinfo.output_components != 3)
    {
        set_error("Limited JPEG reader cannot deal non simple RGB image.");
        add_error("In particular cinfo.output_components is %d, "
                  "not 3 as we are prepared for.", cinfo.output_components);
        add_error("Error occured while reading %F.", fp);
        jpeg_destroy_decompress(&cinfo);
        return NOT_FOUND;
    }
#endif 

#ifdef TEST
    num_steps_completed = 2;
#endif

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        jpeg_destroy_decompress(&cinfo);
        return ERROR;
    }

#ifdef TEST
    num_steps_completed = 3;
#endif

    num_rows -= fs_strip_top;
    num_rows -= fs_strip_bottom;

    num_cols -= fs_strip_left;
    num_cols -= fs_strip_right;

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F after stripping (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        jpeg_destroy_decompress(&cinfo);
        return ERROR;
    }

#ifdef TEST
    num_steps_completed = 4;
#endif

    if (get_target_image(ipp, num_rows, num_cols) == ERROR)
    {
        jpeg_destroy_decompress(&cinfo);
        return ERROR;
    }

#ifdef TEST
    num_steps_completed = 5;
#endif

    scan_line_length = cinfo.output_width * cinfo.output_components;

    /* Make a one-row-high sample array that will go away when done with image*/

    buffer = (*cinfo.mem->alloc_sarray)
                      ((j_common_ptr) &cinfo, JPOOL_IMAGE, scan_line_length, 1);

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
    */

#ifdef TEST
    num_steps_completed = 6;
#endif

    for (i = 0; i<fs_strip_top; i++)
    {
        (void)jpeg_read_scanlines(&cinfo, buffer, 1);
    }

#ifdef TEST
    num_steps_completed = 7;
#endif

    for (i = 0; i<num_rows; i++)
    {
#ifdef TEST
        /*
         * Fill the buffer to verify that the decompressor is writing the number
         * of bytes we expect.
        */
        unsigned count; 

        for (count = 0; count<cinfo.output_width; count++)
        {
            row_pos = buffer[ 0 ];
            row_pos[ 3*count     ] = 50;
            row_pos[ 3*count + 1 ] = 150;
            row_pos[ 3*count + 2 ] = 250;
        }
#endif 
        (void)jpeg_read_scanlines(&cinfo, buffer, 1);

        target_row = (*ipp)->pixels[ i ];
        row_pos = buffer[ 0 ];

        row_pos += (3 * fs_strip_left);

        for (j = 0; j<num_cols; j++)
        {
            target_row->r = *row_pos++;
            target_row->g = *row_pos++;
            target_row->b = *row_pos++;

            target_row->extra.invalid.r = VALID_PIXEL;
            target_row->extra.invalid.g = VALID_PIXEL;
            target_row->extra.invalid.b = VALID_PIXEL;
            target_row->extra.invalid.pixel = VALID_PIXEL;

            target_row++;
        }
    }

#ifdef TEST
    num_steps_completed = 8;
#endif

    /*
    // Suck up the ones we don't want, otherwise the JPEG engine will think
    // that we are closing up before we are done!
    */
    for (i = 0; i<fs_strip_bottom; i++)
    {
        (void)jpeg_read_scanlines(&cinfo, buffer, 1);
    }

#ifdef TEST
    num_steps_completed = 9;
#endif

    (void)jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

#ifdef TEST
    num_steps_completed = 10;
#endif

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    verbose_pso(10, "Read of jpeg file with builtin routine succeeded.\n");

    return NO_ERROR;
}

#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
// This routine reads PCD using a process halfway between true implementation,
// and using the hpcdtoppm program. We use hpcdtoppm to read in a ppm file
// containing the YCC, which we convert here in floating point to avoid losing
// precision and information loss due to clipping.
*/
static int read_image_from_pcd
(
    KJB_image** ipp,
    const char* file_name,
    const char* sub_image,
    int         pcd_rotate
)
{
    KJB_image* ycc_ip;
    int        i, j, num_rows, num_cols;
    char       temp_name[ MAX_FILE_NAME_SIZE ];
    char       ppm_name[ MAX_FILE_NAME_SIZE ];
    char       command_str[ 2 * MAX_FILE_NAME_SIZE + 100 ];
    int        result = NO_ERROR;
    char       sub_image_num_str[ 100 ];
    int         sub_image_num = 3;
    const char* sub_image_pos = sub_image;
    int         invalid_count = 0;


    /*
    // Note that we do not implement stripping here because of the recursive
    // call to kjb_read_image_2() which will do the stripping on the ycc file.
    */

    if (*sub_image_pos != '\0')
    {
        if ( ! BUFF_CONST_GEN_GET_TOKEN_OK(&sub_image_pos, sub_image_num_str, "[]"))
        {
            set_error("Unable to deterime subimage number from %q.",
                      sub_image);
            return ERROR;
        }
        else if (ss1pi(sub_image_num_str, &sub_image_num) == ERROR)
        {
            return ERROR;
        }
        else if ((sub_image_num < 1) || (sub_image_num > 5))
        {
            set_error("%d is an invalid PCD sub-image number.");
            return ERROR;
        }
    }

    ERE( BUFF_GET_TEMP_FILE_NAME( temp_name ) );

    ERE(kjb_sprintf(ppm_name, sizeof(ppm_name), "%s.ppm", temp_name));

    if (sub_image_num < 5)
    {
        ERE(kjb_sprintf(command_str, sizeof(command_str),
                        "hpcdtoppm -ycc -rep -x -%d %s %s",
                        sub_image_num, file_name, ppm_name));
    }
    else
    {
        ERE(kjb_sprintf(command_str, sizeof(command_str),
                        "hpcdtoppm -ycc -rep -%d %s %s",
                        sub_image_num, file_name, ppm_name));
    }

    if (    (kjb_system(command_str) == ERROR)
         || (kjb_read_image_2(ipp, ppm_name) != NO_ERROR)
       )
    {
        return NOT_FOUND;
    }

    EPE(kjb_unlink(ppm_name));

    ycc_ip = *ipp;

    if (pcd_rotate == 1)
    {
        KJB_image* rotated_ip = NULL;

        ERE(rotate_image_left(&rotated_ip, ycc_ip));
        ERE(kjb_copy_image(ipp, rotated_ip));
        ycc_ip = *ipp;
        kjb_free_image(rotated_ip);
    }
    else if (pcd_rotate == 3)
    {
        KJB_image* rotated_ip = NULL;

        ERE(rotate_image_right(&rotated_ip, ycc_ip));
        ERE(kjb_copy_image(ipp, rotated_ip));
        ycc_ip = *ipp;
        kjb_free_image(rotated_ip);
    }


    /*
    // If the ycc file has values at 255 (and 0 for c1 and c2) then the value
    // may be clipped. We can deal with the case of 255 with
    // mark_pixels_above_threshold(), but we use routine
    // mark_pixels_below_threshold() for the 0's because only c1 and c2 are
    // invalid if zero. We deal with the zeros in the loop below.
    */

    EPE(mark_pixels_above_threshold(ycc_ip, 254.5));

    num_rows = ycc_ip->num_rows;
    num_cols = ycc_ip->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        Pixel* ycc_pixel_pos = ycc_ip->pixels[ i ];

        for (j = 0; j < num_cols; j++)
        {
            int invalid = FALSE;

            if (ycc_pixel_pos->g < 0.5)
            {
                ycc_pixel_pos->extra.invalid.g |= CLIPPED_PIXEL;
                invalid = TRUE;
            }

            if (ycc_pixel_pos->b < 0.5)
            {
                ycc_pixel_pos->extra.invalid.g |= CLIPPED_PIXEL;
                invalid = TRUE;
            }

            if (invalid)
            {
                invalid_count++;
                ycc_pixel_pos->extra.invalid.pixel |= CLIPPED_PIXEL;
            }

            ycc_pixel_pos++;
        }
    }

    verbose_pso(3,
           "%d image pixels marked as invalid because c1 or c2 were zero.\n",
            invalid_count);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int ow_convert_pcd_ycc_to_rgb(KJB_image* ip)
{
    int i, j, num_rows, num_cols;
    int result   = NO_ERROR;
    int invalid_count = 0;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        Pixel* pixel_pos = ip->pixels[ i ];

        for (j = 0; j < num_cols; j++)
        {
            float l, c1, c2, r, g, b;
            int invalid = FALSE;


            if (fs_pcd_ycc_to_rgb_forward_conversion)
            {
                l =  1.3584 * pixel_pos->r;
                c1 = 2.2179 * (pixel_pos->g - 156.0);
                c2 = 1.8215 * (pixel_pos->b - 137.0);
            }
            else
            {
                /* Inverted backwards equations. */
                l =  1.402 * pixel_pos->r;
                c1 = 2.289 * (pixel_pos->g - 156.0);
                c2 = 1.880 * (pixel_pos->b - 137.0);
            }

            r = l + c2;
            g = l - 0.194 * c1 - 0.509 * c2;
            b = l + c1;

            /*
            // Because of the different color spaces, a problem with a YCC
            // pixel, can mean a problem with any of the RGB channels.
            */
            pixel_pos->extra.invalid.r = pixel_pos->extra.invalid.pixel;
            pixel_pos->extra.invalid.g = pixel_pos->extra.invalid.pixel;
            pixel_pos->extra.invalid.b = pixel_pos->extra.invalid.pixel;

            if (r < 0.0)
            {
                pixel_pos->extra.invalid.r |= DARK_PIXEL;
                r = 0.0;
                invalid = TRUE;
            }

            pixel_pos->r = r;

            if (g < 0.0)
            {
                pixel_pos->extra.invalid.g |= DARK_PIXEL;
                g = 0.0;
                invalid = TRUE;
            }

            pixel_pos->g = g;

            if (b < 0.0)
            {
                pixel_pos->extra.invalid.b |= DARK_PIXEL;
                b = 0.0;
                invalid = TRUE;
            }
            pixel_pos->b = b;

            if (invalid)
            {
                invalid_count++;
            }

            pixel_pos->extra.invalid.pixel = pixel_pos->extra.invalid.r |
                                           pixel_pos->extra.invalid.g |
                                           pixel_pos->extra.invalid.b;

            pixel_pos++;
        }
    }

    verbose_pso(3,
           "%d image pixels marked invalid due to negative rgb's\n",
            invalid_count);
    verbose_pso(3, "while converting from PCD YCC to RGB with gamma.\n");

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_image_from_bmp(KJB_image** ipp, FILE* fp)
{
    KJB_image* ip;
    int        i, j, num_rows, num_cols;
    int        row_length;
    unsigned char*     data_row;
    unsigned char*     data_row_pos;
    Pixel*     out_pos;
    int        result;
    int        c1, c2;
    kjb_int32  filler_int32;
    kjb_uint32 header_file_size;
    kjb_uint32 reversed_header_file_size;
    off_t      file_size;
    int        reverse_the_bytes;
    kjb_int32  reversed_num_rows;
    kjb_int32  reversed_num_cols;
    kjb_int16  num_planes;
    kjb_int16  reversed_num_planes;
    kjb_int16  bpp;
    kjb_int16  reversed_bpp;


    c1 = kjb_fgetc(fp);
    c2 = kjb_fgetc(fp);

    if ((c1 != 'B') || (c2 != 'M'))
    {
        set_error("%F is not a valid BMP file.", fp);
        return ERROR;
    }

    ERE(fp_get_byte_size(fp, &file_size));

    ERE(FIELD_READ(fp, header_file_size));
    reverse_four_bytes(&header_file_size, &reversed_header_file_size);

    if (file_size == (off_t)header_file_size)
    {
        reverse_the_bytes = FALSE;
    }
    else if (file_size == (off_t)reversed_header_file_size)
    {
        reverse_the_bytes = TRUE;
    }
    else
    {
        return NOT_FOUND;
    }

    ERE(FIELD_READ(fp, filler_int32));
    ERE(FIELD_READ(fp, filler_int32));
    ERE(FIELD_READ(fp, filler_int32));

    ERE(FIELD_READ(fp, num_cols));

    if (reverse_the_bytes)
    {
        reverse_four_bytes(&num_cols, &reversed_num_cols);
        num_cols = reversed_num_cols;
    }

    ERE(FIELD_READ(fp, num_rows));

    if (reverse_the_bytes)
    {
        reverse_four_bytes(&num_rows, &reversed_num_rows);
        num_rows = reversed_num_rows;
    }

    if (54 + 3 * num_rows * num_cols != file_size)
    {
        return NOT_FOUND;
    }

    ERE(FIELD_READ(fp, num_planes));

    if (reverse_the_bytes)
    {
        reverse_two_bytes(&num_planes, &reversed_num_planes);
        num_planes = reversed_num_planes;
    }

    if (num_planes != 1) return NOT_FOUND;

    ERE(FIELD_READ(fp, bpp));

    if (reverse_the_bytes)
    {
        reverse_two_bytes(&bpp, &reversed_bpp);
        bpp = reversed_bpp;
    }

    if (bpp != 24) return NOT_FOUND;

    ERE(kjb_fseek(fp, 54, SEEK_SET));

    row_length = 3 * num_cols;

    NRE(data_row = BYTE_MALLOC(row_length));

    num_rows -= fs_strip_top;
    num_rows -= fs_strip_bottom;

    num_cols -= fs_strip_left;
    num_cols -= fs_strip_right;

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F after stripping (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    result = get_target_image(ipp, num_rows, num_cols);

    if (result == ERROR)
    {
        kjb_free(data_row);
        return ERROR;
    }

    ip = *ipp;

    for(i=0; i<fs_strip_top; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }
    }

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row;
        out_pos = ip->pixels[ i ];

        data_row_pos += (3 * fs_strip_left);

        for(j=0; j<num_cols; j++)
        {
            out_pos->r = *data_row_pos++;
            out_pos->g = *data_row_pos++;
            out_pos->b = *data_row_pos++;

            out_pos->extra.invalid.r     = VALID_PIXEL;
            out_pos->extra.invalid.g     = VALID_PIXEL;
            out_pos->extra.invalid.b     = VALID_PIXEL;
            out_pos->extra.invalid.pixel = VALID_PIXEL;

            out_pos++;
        }
    }

    kjb_free(data_row);

    verbose_pso(10,"Read of 24 bit BMP file with builtin routine succeeded.\n");

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_image_from_pnm_P6(KJB_image** ipp, FILE* fp)
{
    KJB_image* ip;
    int        i, j, num_rows, num_cols;
    int        row_length;
    off_t      pnm_size;
    off_t      file_size;
    unsigned char*     data_row;
    unsigned char*     data_row_pos;
    Pixel*     out_pos;
    int        result;
    int        c1, c2;
    int        max_val;
    long       header_size;


    c1 = kjb_fgetc(fp);
    c2 = kjb_fgetc(fp);

    if ((c1 != 'P') || (c2 != '6'))
    {
        set_error("%F is not a valid PNM-6 file.", fp);
        return ERROR;
    }

    ERE(read_pnm_6_header_num(fp, &num_cols));
    ERE(read_pnm_6_header_num(fp, &num_rows));
    ERE(read_pnm_6_header_num(fp, &max_val));

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    if (max_val != 255)
    {
        set_error("%F is not a valid standard PNM-6 file.", fp);
        add_error("The maximun RGB value in the header is not 255");
        return ERROR;
    }

    ERE(header_size = kjb_ftell(fp));

    row_length = 3 * num_cols;

    ERE(fp_get_byte_size(fp, &file_size));

#define MUST_HACK_FOR_NOW   /* AR frame grabber is doing something funny! */

#ifdef MUST_HACK_FOR_NOW
    pnm_size = file_size - header_size;
    num_rows = pnm_size / row_length;
#else
    pnm_size = row_length * num_rows;

    if (file_size - header_size != pnm_size)
    {
        set_error("%F is not a valid PNM-P6 file.", fp);
        add_error("It is the wrong size.");
        return ERROR;
    }
#endif

    NRE(data_row = BYTE_MALLOC(row_length));

    num_rows -= fs_strip_top;
    num_rows -= fs_strip_bottom;

    num_cols -= fs_strip_left;
    num_cols -= fs_strip_right;

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F after stripping (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    result = get_target_image(ipp, num_rows, num_cols);

    if (result == ERROR)
    {
        kjb_free(data_row);
        return ERROR;
    }

    ip = *ipp;

    for(i=0; i<fs_strip_top; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }
    }

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row;
        out_pos = ip->pixels[ i ];

        data_row_pos += (3 * fs_strip_left);

        for(j=0; j<num_cols; j++)
        {
            out_pos->r = *data_row_pos++;
            out_pos->g = *data_row_pos++;
            out_pos->b = *data_row_pos++;

            out_pos->extra.invalid.r     = VALID_PIXEL;
            out_pos->extra.invalid.g     = VALID_PIXEL;
            out_pos->extra.invalid.b     = VALID_PIXEL;
            out_pos->extra.invalid.pixel = VALID_PIXEL;

            out_pos++;
        }
    }

    kjb_free(data_row);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_pnm_6_header_num(FILE* fp, int* num_ptr)
{
    int    c;
    char   buff[ 100 ];
    char*  buff_pos          = buff;
    size_t number_char_count = 0;


    /*CONSTCOND*/
    while (TRUE)
    {
        c = kjb_fgetc(fp);

        while ((c == ' ') || (c == '\n') || (c == '\r') || (c == '\t'))
        {
            c = kjb_fgetc(fp);

            if (c == EOF)
            {
                set_error("%F is not a valid PNM-6 file.", fp);
                return ERROR;
            }
        }

        if (c == '#')
        {
            while (c != '\n')
            {
                c = kjb_fgetc(fp);

                if (c == EOF)
                {
                    set_error("%F is not a valid PNM-6 file.", fp);
                    return ERROR;
                }
            }
        }
        else
        {
            number_char_count = 0;

            while ((c != ' ') && (c != '\n') && (c != '\r') && (c != '\t') )
            {
                if (number_char_count >= sizeof(buff))
                {
                    set_error("%F is not a valid PNM-6 file.", fp);
                    return ERROR;
                }

                *buff_pos = c;
                buff_pos++;
                number_char_count++;

                c = kjb_fgetc(fp);

                if (c == EOF)
                {
                    set_error("%F is not a valid PNM-6 file.", fp);
                    return ERROR;
                }
            }
        }

        if (number_char_count > 0)
        {
            *buff_pos = '\0';
            ERE(ss1pi(buff, num_ptr));
            return NO_ERROR;
        }
    }
    /*NOTREACHED*/
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_image_from_raw_dcs_460_16_bit(KJB_image** ipp, FILE* fp)
{
    KJB_image* ip;
    int        num_rows, num_cols, i, j;
    size_t     row_length;
    kjb_uint16*    data_row;
    kjb_uint16*    data_row_pos;
    Pixel*     image_pos;
    int        result;


    UNTESTED_CODE();     /* Untested since adding stripping. */

    row_length = 3 * NUM_DCS_460_COLS * sizeof(kjb_uint16);

    NRE(data_row = UINT16_MALLOC(row_length));

    num_rows = NUM_DCS_460_ROWS - fs_strip_top - fs_strip_bottom;
    num_cols = NUM_DCS_460_COLS - fs_strip_left - fs_strip_right;

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F after stripping (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    result = get_target_image(ipp, num_rows, num_cols);

    if (result == ERROR)
    {
        kjb_free(data_row);
        return ERROR;
    }

    ip = *ipp;

    for(i=0; i<fs_strip_top; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }
    }

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row;

        image_pos = ip->pixels[ i ];

        data_row_pos += (3 * fs_strip_left);

        for(j=0; j<num_cols; j++)
        {
            image_pos->r = *data_row_pos;
            image_pos->r /= (float)256.0;
            data_row_pos++;

            image_pos->g = *data_row_pos;
            image_pos->g /= (float)256.0;
            data_row_pos++;

            image_pos->b = *data_row_pos;
            image_pos->b /= (float)256.0;
            data_row_pos++;

            image_pos->extra.invalid.r = VALID_PIXEL;
            image_pos->extra.invalid.g = VALID_PIXEL;
            image_pos->extra.invalid.b = VALID_PIXEL;
            image_pos->extra.invalid.pixel = VALID_PIXEL;

            image_pos++;
        }
    }

    kjb_free(data_row);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* Lindsay - Sept 28, 1999 */
/*
 * Reads an HDRC 640 X 480 image into a floating point image structure.
 *
 * NOTE: There is NO provision for image stripping in this function,
 *       as fixed pixel correction and HDRC colour image demosaicing
 *       require the full image size to operate correctly.
 *
 * Reads 10-bit data verbatum from the .idt file. No range scaling!
 */

static int read_image_from_raw_hdrc_16_bit(KJB_image** ipp, FILE* fp)
{
    KJB_image* ip;
    int        num_rows, num_cols, i, j;
    size_t     row_length;
    size_t     ri;
    kjb_uint16*    data_row;
    kjb_uint16*    data_row_pos;
    kjb_uint16     pixel;
    kjb_uint16     reversed_pixel;
    kjb_uint16     mask;
    kjb_uint16     sensor_min;
    kjb_uint16     sensor_max;
    Pixel*     image_pos;
    int        result;

    sensor_min = 1023;
    sensor_max = 0;

    mask = (kjb_uint16)0x3FF; /* Only the lower 10 bits are significant */

    row_length = HDRC_NUM_COLS * sizeof(kjb_uint16);

    NRE(data_row = UINT16_MALLOC(row_length));

    for (ri= 0; ri < row_length; ri++)
    {
        data_row[ ri ] = (kjb_uint16)0;
    }

    num_rows = HDRC_NUM_ROWS;
    num_cols = HDRC_NUM_COLS;

    result = get_target_image(ipp, num_rows, num_cols);

    if (result == ERROR)
    {
        kjb_free(data_row);
        return ERROR;
    }

    ip = *ipp;

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row;

        image_pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            pixel = *data_row_pos;

            /* Input data has least significant byte first */
            reverse_two_bytes(&pixel, &reversed_pixel);

            /* Toss meaningless upper 6 bits */
            reversed_pixel &= mask;

            if (reversed_pixel > sensor_max) sensor_max = reversed_pixel;
            if (reversed_pixel < sensor_min) sensor_min = reversed_pixel;

            image_pos->r = reversed_pixel;
            image_pos->g = reversed_pixel;
            image_pos->b = reversed_pixel;

            data_row_pos++;

            image_pos->extra.invalid.r = VALID_PIXEL;
            image_pos->extra.invalid.g = VALID_PIXEL;
            image_pos->extra.invalid.b = VALID_PIXEL;
            image_pos->extra.invalid.pixel = VALID_PIXEL;

            image_pos++;
        }
    }

    kjb_free(data_row);

    verbose_pso(10, "Raw HDRC sensor min, max: %d, %d.\n",
                sensor_min, sensor_max);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_threshold_pixel_coords
(
    KJB_image* ip,
    char*      file_name,
    double     threshold
)
{
    int   i, j, count;
    float pix_value, thresh;
    FILE* pix_coords_fp;
    char  pix_coords_file_name[ MAX_FILE_NAME_SIZE ];

    if (ip == NULL)
        return NO_ERROR;

    thresh = (float)threshold;

    (void)kjb_sprintf(pix_coords_file_name,
                      MAX_FILE_NAME_SIZE,
                      "%s.%f.pixel_coords",
                      file_name, thresh);

    NRE(pix_coords_fp = kjb_fopen(pix_coords_file_name, "w"));

    count = 0;
    for (i = 0; i < ip->num_rows; i++)
    {
        for (j = 0; j < ip->num_cols; j++)
        {
            pix_value = (ip->pixels)[ i ][ j ].r;

            if (pix_value >= thresh)
            {
                (void)kjb_fprintf(pix_coords_fp, "%3d %3d %6.1f\n",
                                  i, j, pix_value);
                count++;
            }
        }
    }

    ERE(verbose_pso(10, "Coords for %d pixels >= %6.1f written to %s.\n",
                    count, thresh, pix_coords_file_name));

    return kjb_fclose(pix_coords_fp);
}

/* End Lindsay */
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_image_from_MID_file(KJB_image** ipp, FILE* fp)
{
    kjb_uint32 magic_number;
    kjb_uint32 reversed_magic_number;
    kjb_int32  temp_int32;
    int    num_rows;
    int    num_cols;
    int    reversed = FALSE;


    ERE(FIELD_READ(fp, magic_number));
    reverse_four_bytes(&magic_number, &reversed_magic_number);

    if (magic_number == MID_MAGIC_NUM)
    {
        ERE(FIELD_READ(fp, num_rows));
        ERE(FIELD_READ(fp, num_cols));
    }
    else if (reversed_magic_number == MID_MAGIC_NUM)
    {
        reversed = TRUE;
        ERE(FIELD_READ(fp, temp_int32));
        reverse_four_bytes(&temp_int32, &num_rows);
        ERE(FIELD_READ(fp, temp_int32));
        reverse_four_bytes(&temp_int32, &num_cols);
    }
    else
    {
        set_error("Invalid \"mid\" image. Magic number is wrong.");
        return ERROR;
    }

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    return read_float_channel_data(ipp, fp, num_rows, num_cols, reversed);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int read_image_from_kiff
(
    KJB_image** ipp,
    FILE*       fp,
    int*        validity_data_is_part_of_image_ptr
)
{
    Kiff_data_type format;
    int            num_rows;
    int            num_cols;
    int            result;
    int            validity_data_is_part_of_image = FALSE;
    int            reversed;


    ERE(read_kiff_header(fp, &format, &num_rows, &num_cols, &reversed));

    if (format == FLOAT_WITH_VALIDITY_KIFF)
    {
        validity_data_is_part_of_image = TRUE;
        verbose_pso(10, "Reading float with validity kiff image.\n");

        result = read_float_with_validity_pixels(ipp, fp, num_rows,
                                                 num_cols, reversed);
    }
    else if (format == FLOAT_KIFF)
    {
        validity_data_is_part_of_image = fs_invalidate_negative_pixels;

        verbose_pso(10, "Reading float WITHOUT validity kiff image.\n");

        result = read_float_pixels(ipp, fp, num_rows, num_cols,
                                   reversed);
    }
    else if (format == BYTE_KIFF)
    {
        /*
        // Support Byte image because Mike supports it, and his code may decided
        // to write a file in this format some day.
        */
        verbose_pso(10, "Reading Byte kiff image.\n");
        result = read_byte_pixels(ipp, fp, num_rows, num_cols);
    }
    else
    {
        /*
        // Can't happen as long as the formats accepted by the header read
        // are the same as the formats accepted above. (True 01/03/97).
        */
        SET_CANT_HAPPEN_BUG();
        result = ERROR;
    }

    if (validity_data_is_part_of_image_ptr != NULL)
    {
        *validity_data_is_part_of_image_ptr = validity_data_is_part_of_image;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_kiff_header
(
    FILE*           fp,
    Kiff_data_type* format_ptr,
    int*            num_rows_ptr,
    int*            num_cols_ptr,
    int*            reversed_ptr
)
{
    int result;


    result = read_kiff_header_guts(fp, format_ptr, num_rows_ptr, num_cols_ptr,
                                   reversed_ptr);

    if (result == ERROR)
    {
        insert_error("%F is not a valid kiff file.", fp);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_kiff_header_guts
(
    FILE*           fp,
    Kiff_data_type* format_ptr,
    int*            num_rows_ptr,
    int*            num_cols_ptr,
    int*            reversed_ptr
)
{
    kjb_uint32 magic_number;
    kjb_uint32 reversed_magic_number;
    char   line[ LARGE_IO_BUFF_SIZE ];
    char*  line_pos;
    char   word[ 100 ];
    int    annotation_len;
    /*
    // int    result;
    // char   previous_lead_char;
    */


    ERE(FIELD_READ(fp, magic_number));
    reverse_four_bytes(&magic_number, &reversed_magic_number);

    if (    (magic_number == KIFF_MAGIC_NUM)
         || (magic_number == OLD_KIFF_MAGIC_NUM)
       )
    {
        *reversed_ptr = FALSE;
    }
    else if (    (reversed_magic_number == KIFF_MAGIC_NUM)
              || (reversed_magic_number == OLD_KIFF_MAGIC_NUM)
            )
    {
        *reversed_ptr = TRUE;
    }
    else
    {
        set_error("Proposed kiff image magic number is wrong.");
        return ERROR;
    }

    ERE(BUFF_FGET_LINE(fp, line));
    ERE(BUFF_FGET_LINE(fp, line));

    line_pos = line;

    BUFF_GET_TOKEN(&line_pos, word);

    if ( ! STRCMP_EQ(word, "kformat:"))
    {
        set_error("Missing or invalid placement of \"kformat:\" field id.");
        return ERROR;
    }

    BUFF_GET_TOKEN(&line_pos, word);

    if (STRCMP_EQ(word, "float_with_validity"))
    {
        *format_ptr = FLOAT_WITH_VALIDITY_KIFF;
    }
    else if (STRCMP_EQ(word, "float"))
    {
        *format_ptr = FLOAT_KIFF;
    }
    else if (STRCMP_EQ(word, "Byte"))
    {
        *format_ptr = BYTE_KIFF;
    }
    else
    {
        set_error("Missing or invalid format field.");
        return ERROR;
    }

    ERE(BUFF_FGET_LINE(fp, line));

    line_pos = line;

    BUFF_GET_TOKEN(&line_pos, word);

    if ( ! STRCMP_EQ(word, "rows:"))
    {
        set_error("Missing or invalid placement of \"rows:\" field id.");
        return ERROR;
    }

    BUFF_GEN_GET_TOKEN(&line_pos, word, " , ");

    ERE(ss1pi(word, num_rows_ptr));

    if (*line_pos == ',')
    {
        line_pos++;
    }

    trim_beg(&line_pos);

    if (*line_pos == '\0')
    {
        verbose_pso(20, "Reading new style kiff image header.\n");
        ERE(BUFF_FGET_LINE(fp, line));
        line_pos = line;
    }
    else verbose_pso(20, "Reading old style kiff image header.\n");

    BUFF_GEN_GET_TOKEN(&line_pos, word, " ");

    if ( ! STRCMP_EQ(word, "cols:"))
    {
        set_error("Missing or invalid placement of \"cols:\" field id.");
        return ERROR;
    }

    BUFF_GET_TOKEN(&line_pos, word);

    ERE(ss1pi(word, num_cols_ptr));

    ERE(BUFF_FGET_LINE(fp, line));

    line_pos = line;

    BUFF_GET_TOKEN(&line_pos, word);

    if ( ! STRCMP_EQ(word, "annotation"))
    {
        set_error("Missing or invalid placement of \"annotation\" field id.");
        return ERROR;
    }

    BUFF_GET_TOKEN(&line_pos, word);

    if ( ! STRCMP_EQ(word, "length:"))
    {
        set_error("Missing or invalid placement of \"length:\" field id.");
        return ERROR;
    }

    BUFF_GET_TOKEN(&line_pos, word);

    /*
    // Original way. This works fine. I thought there was a problem, so I
    // wrote the alternate below, but the problem was elsewhere. Keep the
    // alternate just in case.
    */

    ERE(ss1pi(word, &annotation_len));
    ERE(kjb_fseek(fp, annotation_len, SEEK_CUR));

    /*
    // Alternate method used to verify that the above is OK. Since the above
    // is slightly more efficient, we will use it unless there are more
    // problems.
    //
    // kjb_clear_error();
    // previous_lead_char = '\0';
    //
    // while (TRUE)
    //    {
    //     result = BUFF_FGET_LINE(fp, line);
    //
    //     if (result < 0)
    //        {
    //         insert_error("Problem encountered reading kiff header.");
    //         return ERROR;
    //        }
    //
    //     if ((line[ 0 ] == ':') && (previous_lead_char == ''))
    //        {
    //         break;
    //        }
    //
    //     previous_lead_char = line[ 0 ];
    //    }
    */

    if ((*num_rows_ptr < 1) || (*num_cols_ptr < 1))
    {
        set_error("Dimensions of %F (%d by %d) are invalid.",
                  fp, *num_rows_ptr, *num_cols_ptr);
        return ERROR;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_float_with_validity_pixels
(
    KJB_image** ipp,
    FILE*       fp,
    int         num_rows,
    int         num_cols,
    int         reversed
)
{
    KJB_image* ip;
    int        i, j;
    int        row_length;
    Pixel*     data_row;
    Pixel*     data_row_pos;
    Pixel*     out_pos;
    int        result;
    off_t      file_size;
    off_t      expected_size;


#ifdef TEST
    if (reversed)
    {
        UNTESTED_CODE();
    }
    else
    {
        UNTESTED_CODE();
    }
#endif

    row_length = num_cols * sizeof(Pixel);
    expected_size = kjb_ftell(fp) + num_rows * row_length;

    ERE(fp_get_byte_size(fp, &file_size));

    if (file_size != expected_size)
    {
        /*
        // Use this once Mike fixes problem with kiff's.
        //
        // set_error("%F has too %s float data.", fp,
        //           (file_size < expected_size) ? "little" : "much" );
        */
        if (file_size < expected_size)
        {
            set_error("%F has too little float data.", fp);
            return ERROR;
        }
        else
        {
            p_stderr("Warning: %F has too much float data.\n", fp);
        }
    }

    NRE(data_row = N_TYPE_MALLOC(Pixel, num_cols));

    num_rows -= fs_strip_top;
    num_rows -= fs_strip_bottom;

    num_cols -= fs_strip_left;
    num_cols -= fs_strip_right;

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F after stripping (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    result = get_target_image(ipp, num_rows, num_cols);

    if (result == ERROR)
    {
        kjb_free(data_row);
        return ERROR;
    }

    ip = *ipp;

    for(i=0; i<fs_strip_top; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }
    }

    UNTESTED_CODE();

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row;
        out_pos = ip->pixels[ i ];

        data_row_pos += fs_strip_left;

        for(j=0; j<num_cols; j++)
        {
            if (reversed)
            {
                float r,g,b;

                reverse_four_bytes(&(data_row_pos->r), &r);
                out_pos->r = r;
                reverse_four_bytes(&(data_row_pos->g), &g);
                out_pos->g = g;
                reverse_four_bytes(&(data_row_pos->b), &b);
                out_pos->b = b;
            }
            else
            {
                out_pos->r = data_row_pos->r;
                out_pos->g = data_row_pos->g;
                out_pos->b = data_row_pos->b;
            }

            if (fs_force_valid_kiff_on_read)
            {
                out_pos->extra.invalid.r = VALID_PIXEL;
                out_pos->extra.invalid.g = VALID_PIXEL;
                out_pos->extra.invalid.b = VALID_PIXEL;
                out_pos->extra.invalid.pixel = VALID_PIXEL;
            }
            else
            {
                out_pos->extra.invalid = data_row_pos->extra.invalid;
            }

            out_pos++;
            data_row_pos++;
        }
    }

    kjb_free(data_row);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_float_pixels
(
    KJB_image** ipp,
    FILE*       fp,
    int         num_rows,
    int         num_cols,
    int         reversed
)
{
    KJB_image* ip;
    int        i, j;
    int        row_length;
    float*     data_row;
    float*     data_row_pos;
    Pixel*     out_pos;
    int        result;
    off_t      file_size;
    off_t      expected_size;
    float      temp_float;


    row_length = num_cols * sizeof(float) * 3;
    expected_size = kjb_ftell(fp) + num_rows * row_length;

    ERE(fp_get_byte_size(fp, &file_size));

    if (file_size != expected_size)
    {
        /*
        // Use this once Mike fixes problem with kiff's.
        //
        // set_error("%F has too %s float data.", fp,
        //           (file_size < expected_size) ? "little" : "much" );
        */
        if (file_size < expected_size)
        {
            set_error("%F has too little float data.", fp);
            return ERROR;
        }
        else
        {
            warn_pso("%F has too much float data.\n", fp);
        }
    }

    NRE(data_row = FLT_MALLOC(row_length));

    num_rows -= fs_strip_top;
    num_rows -= fs_strip_bottom;

    num_cols -= fs_strip_left;
    num_cols -= fs_strip_right;

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F after stripping (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    result = get_target_image(ipp, num_rows, num_cols);

    if (result == ERROR)
    {
        kjb_free(data_row);
        return ERROR;
    }

    ip = *ipp;

    for(i=0; i<fs_strip_top; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }
    }

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row + (3 * fs_strip_left);
        out_pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            if (reversed)
            {
                reverse_four_bytes(data_row_pos, &temp_float);
                out_pos->r = temp_float;
            }
            else
            {
                out_pos->r = *data_row_pos;
            }
            data_row_pos++;

            if ((out_pos->r < 0.0) && (fs_invalidate_negative_pixels))
            {
                out_pos->r = -(out_pos->r);
                out_pos->extra.invalid.r = INVALID_PIXEL;
            }
            else
            {
                out_pos->extra.invalid.r = VALID_PIXEL;
            }

            if (reversed)
            {
                reverse_four_bytes(data_row_pos, &temp_float);
                out_pos->g = temp_float;
            }
            else
            {
                out_pos->g = *data_row_pos;
            }
            data_row_pos++;

            if ((out_pos->g < 0.0) && (fs_invalidate_negative_pixels))
            {
                out_pos->g = -(out_pos->g);
                out_pos->extra.invalid.g = INVALID_PIXEL;
            }
            else
            {
                out_pos->extra.invalid.g = VALID_PIXEL;
            }

            if (reversed)
            {
                reverse_four_bytes(data_row_pos, &temp_float);
                out_pos->b = temp_float;
            }
            else
            {
                out_pos->b = *data_row_pos;
            }
            data_row_pos++;


            if ((out_pos->b < 0.0) && (fs_invalidate_negative_pixels))
            {
                out_pos->b = -(out_pos->b);
                out_pos->extra.invalid.b = INVALID_PIXEL;
            }
            else
            {
                out_pos->extra.invalid.b = VALID_PIXEL;
            }

            out_pos->extra.invalid.pixel =
                out_pos->extra.invalid.r | out_pos->extra.invalid.g | out_pos->extra.invalid.b;

            ASSERT(out_pos->r >= 0.0);
            ASSERT(out_pos->g >= 0.0);
            ASSERT(out_pos->b >= 0.0);

            out_pos++;
        }
    }

    kjb_free(data_row);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_byte_pixels
(
    KJB_image** ipp,
    FILE*       fp,
    int         num_rows,
    int         num_cols
)
{
    KJB_image* ip;
    int        i, j;
    int        row_length;
    off_t      kiff_size;
    unsigned char*     data_row;
    unsigned char*     data_row_pos;
    Pixel*     out_pos;
    int        result;
    int        header_len;
    off_t      file_size;


    UNTESTED_CODE();     /* Untested since adding stripping. */

    ERE(header_len = kjb_ftell(fp));

    row_length = num_cols * 3;
    kiff_size = num_rows * row_length + header_len;

    ERE(fp_get_byte_size(fp, &file_size));

    if (file_size != kiff_size)
    {
        set_error("%F is not a valid kiff file.", fp);
        add_error("It has too %s data.",
                  (file_size < kiff_size) ? "little" : "much" );
        return ERROR;
    }

    NRE(data_row = BYTE_MALLOC(row_length));

    num_rows -= fs_strip_top;
    num_rows -= fs_strip_bottom;

    num_cols -= fs_strip_left;
    num_cols -= fs_strip_right;

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F after stripping (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    result = get_target_image(ipp, num_rows, num_cols);

    if (result == ERROR)
    {
        kjb_free(data_row);
        return ERROR;
    }

    ip = *ipp;

    for(i=0; i<fs_strip_top; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }
    }

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row;
        out_pos = ip->pixels[ i ];

        data_row_pos += (3 * fs_strip_left);

        for(j=0; j<num_cols; j++)
        {
            out_pos->r = *data_row_pos;
            data_row_pos++;

            out_pos->g = *data_row_pos;
            data_row_pos++;

            out_pos->b = *data_row_pos;
            data_row_pos++;

            out_pos->extra.invalid.pixel = VALID_PIXEL;
            out_pos->extra.invalid.r = VALID_PIXEL;
            out_pos->extra.invalid.g = VALID_PIXEL;
            out_pos->extra.invalid.b = VALID_PIXEL;

            out_pos++;
        }
    }

    kjb_free(data_row);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_float_channel_data
(
    KJB_image** ipp,
    FILE*       fp,
    int         num_rows,
    int         num_cols,
    int         reversed
)
{
    KJB_image* ip;
    int        i, j;
    int        row_length;
    float*     data_row;
    float*     data_row_pos;
    Pixel*     image_pos;
    int        result;
    off_t      file_size;
    off_t      expected_size;
    float      temp_float;


    row_length = num_cols * sizeof(float);
    expected_size = kjb_ftell(fp) + 3 * num_rows * row_length;

    ERE(fp_get_byte_size(fp, &file_size));

    if (file_size != expected_size)
    {
        set_error("%F has too %s float data.", fp,
                  (file_size < expected_size) ? "little" : "much" );
        return ERROR;
    }

    NRE(data_row = FLT_MALLOC(row_length));

    num_rows -= fs_strip_top;
    num_rows -= fs_strip_bottom;

    num_cols -= fs_strip_left;
    num_cols -= fs_strip_right;

    if ((num_rows < 1) || (num_cols < 1))
    {
        set_error("Dimensions of %F after stripping (%d by %d) are invalid.",
                  fp, num_rows, num_cols);
        return ERROR;
    }

    result = get_target_image(ipp, num_rows, num_cols);

    if (result == ERROR)
    {
        kjb_free(data_row);
        return ERROR;
    }

    ip = *ipp;

#ifdef TEST
    pso("Reminder: This data format does not deal with clipping.\n");
#endif

    for(i=0; i<fs_strip_top; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }
    }

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row + fs_strip_left;
        image_pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            if (reversed)
            {
                reverse_four_bytes(data_row_pos, &temp_float);
                image_pos->r = temp_float;
            }
            else
            {
                image_pos->r = *data_row_pos;
            }
            data_row_pos++;
            image_pos++;
        }
    }

    for(i=0; i<fs_strip_top + fs_strip_bottom; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }
    }

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row + fs_strip_left;
        image_pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            if (reversed)
            {
                reverse_four_bytes(data_row_pos, &temp_float);
                image_pos->g = temp_float;
            }
            else
            {
                image_pos->g = *data_row_pos;
            }
            data_row_pos++;
            image_pos++;
        }
    }

    for(i=0; i<fs_strip_top + fs_strip_bottom; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }
    }

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row + fs_strip_left;
        image_pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            if (reversed)
            {
                reverse_four_bytes(data_row_pos, &temp_float);
                image_pos->b = temp_float;
            }
            else
            {
                image_pos->b = *data_row_pos;
            }
            data_row_pos++;
            image_pos++;
        }
    }

    for(i=0; i<num_rows; i++)
    {
        image_pos = ip->pixels[ i ];

        for(j=0; j<num_cols; j++)
        {
            image_pos->extra.invalid.r     = VALID_PIXEL;
            image_pos->extra.invalid.g     = VALID_PIXEL;
            image_pos->extra.invalid.b     = VALID_PIXEL;
            image_pos->extra.invalid.pixel = VALID_PIXEL;

            image_pos++;
        }
    }

    kjb_free(data_row);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 kjb_write_image
 *
 * Writes a float image to a file
 *
 * This routine writes a float image to a file. The image file format is
 * specified by the suffix of the file name. For example, if the argument
 * "file_name" is "xxx.tif", then the image is written as a TIFF file. If there
 * is no suffix, then the image file format will be sun raster. ".sun" or ".ras"
 * can also be used for sun raster. Practically any image format can be
 * specified provided that the appropriate conversion programs are in place for
 * the more unusual cases.
 *
 * The suffixes ".mid" and ".kiff" are used to write the SFU specific MID and
 * KIFF file formats.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure, with an error message being set.
 *
 * Index: images, image I/O
 *
 * -----------------------------------------------------------------------------
*/

int kjb_write_image(const KJB_image* ip, const char* file_name)
{
    char base_name[ MAX_FILE_NAME_SIZE ];
    char suffix[ MAX_FILE_NAME_SIZE ];
    char lc_suffix[ MAX_FILE_NAME_SIZE ];
    unsigned char has_alpha = ip->flags & HAS_ALPHA_CHANNEL;


    if (skip_because_no_overwrite(file_name)) return NO_ERROR;

    ERE(get_image_file_base_path(file_name, base_name, sizeof(base_name),
                                 suffix, sizeof(suffix)));

    EXTENDED_LC_BUFF_CPY(lc_suffix, suffix);

    if (    (lc_suffix[ 0 ] == '\0')
         || (STRCMP_EQ(lc_suffix, "sun"))
         || (STRCMP_EQ(lc_suffix, "ras"))
       )
    {
        if(has_alpha)
        {
            set_error("Sun raster writer doesn't currently support alpha channel.");
            return ERROR;
        }
        return write_image_as_raster(ip, file_name);
    }
#ifdef KJB_HAVE_TIFF
    else if (    (STRCMP_EQ(lc_suffix, "tiff"))
              || (STRCMP_EQ(lc_suffix, "tif"))
            )
    {
        return write_image_as_tiff(ip, file_name);
    }
#endif
#ifdef KJB_HAVE_JPEG
#ifndef __C2MAN__
    else if (    (STRCMP_EQ(lc_suffix, "jpeg"))
              || (STRCMP_EQ(lc_suffix, "jpg"))
            )
    {
        if(has_alpha)
        {
            set_error("JPEG writer doesn't currently support alpha channel.");
            return ERROR;
        }

        if (write_image_as_jpeg(ip, file_name) == NO_ERROR) 
        {
            return NO_ERROR;
        }
        else 
        {
            add_error("Trying to write into a temporary file with a different format and convert it into JPEG.");
            kjb_print_error();
            /* Fall through and see if converting will help. */
        }
    }
#endif
#endif
    else if (    (STRCMP_EQ(lc_suffix, "kiff"))
              || (STRCMP_EQ(lc_suffix, "kif"))
            )
    {
        if(has_alpha)
        {
            set_error("kiff writer doesn't currently support alpha channel.");
            return ERROR;
        }

        if (fs_write_validity_kiff)
        {
            return write_image_as_validity_kiff(ip, file_name);
        }
        else
        {
            return write_image_as_float_kiff(ip, file_name);
        }
    }
    else if (STRCMP_EQ(lc_suffix, "mid"))
    {
        if(has_alpha)
        {
            set_error("mid image writer doesn't currently support alpha channel.");
            return ERROR;
        }

        return write_image_as_MID_file(ip, file_name);
    }
    else if (STRCMP_EQ(lc_suffix, "r16"))
    {
        if(has_alpha)
        {
            set_error("r16 image writer doesn't currently support alpha channel.");
            return ERROR;
        }

        return write_image_as_raw_16_bit(ip, file_name);
    }

    /* If all else fails: Do the conversion. */
    {
        char temp_file_name[ MAX_FILE_NAME_SIZE ];
        int  result;

        ERE(BUFF_GET_TEMP_FILE_NAME(temp_file_name));

#ifdef KJB_HAVE_TIFF
        verbose_pso(2, "Attempting to write of image into a temporary TIFF image.\n");
        ERE(write_image_as_tiff(ip, temp_file_name));
        verbose_pso(2, "Write of image to temporary TIFF succeeded.\n");
#else
        if(has_alpha)
        {
            set_error("Default image writer (Sun raster) doesn't support alpha channel.  Install libtiff and recompile for alpha support.");
            return ERROR;
        }
        verbose_pso(2, "Attempting to write of image into a temporary raster image.\n");
        ERE(write_image_as_raster(ip, temp_file_name));
        verbose_pso(2, "Write of image to temporary raster image.\n");
#endif

        result = convert_image_file_from_raster(temp_file_name, file_name);
        if (result != ERROR)
        {
            verbose_pso(2, "Conversion of temporary image file to %s succeeded.\n",
                        file_name);
        }

        kjb_unlink(temp_file_name);
        return result;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_image_as_MID_file
(
    const KJB_image* ip,
    const char*      file_name
)
{
    FILE*        fp;
    kjb_int32        magic_number      = MID_MAGIC_NUM;
    int          result;
    Error_action save_error_action = get_error_action();


    NRE(fp = kjb_fopen(file_name, "wb"));

    result = FIELD_WRITE(fp, magic_number);

    if (result != ERROR) result = FIELD_WRITE(fp, ip->num_rows);
    if (result != ERROR) result = FIELD_WRITE(fp, ip->num_cols);
    if (result != ERROR) result = write_float_channel_image_data(ip, fp);

    if (result == ERROR) set_error_action(FORCE_ADD_ERROR_ON_ERROR);

    if (kjb_fclose(fp) == ERROR) result = ERROR;

    set_error_action(save_error_action);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_image_as_raw_16_bit
(
    const KJB_image* ip,
    const char*      file_name
)
{
    FILE*  fp;
    int    num_rows, num_cols, i, j;
    Pixel* in_pos;
    kjb_uint16 output;


    /*
    // FIX
    //
    // On ERROR, fp does not get closed.
    */
    NRE(fp = kjb_fopen(file_name, "wb"));

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            output = (kjb_uint16)((float)256.0 * in_pos->r);
            ERE(FIELD_WRITE(fp, output));
            output = (kjb_uint16)((float)256.0 * in_pos->g);
            ERE(FIELD_WRITE(fp, output));
            output = (kjb_uint16)((float)256.0 * in_pos->b);
            ERE(FIELD_WRITE(fp, output));

            in_pos++;
        }
    }

    ERE(kjb_fclose(fp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_image_as_validity_kiff
(
    const KJB_image* ip,
    const char*      file_name
)
{
    FILE*       fp;
    kjb_int32   magic_number = KIFF_MAGIC_NUM;
    const char* annotation   = "\n\n:\n";
    int         result;


    /*
    // FIX
    //
    // On some errors, fp does not get closed.
    */

    NRE(fp = kjb_fopen(file_name, "wb"));

    ERE(FIELD_WRITE(fp, magic_number));
    ERE(kjb_fputs(fp, "\nkformat: float_with_validity\n"));
    ERE(kjb_fprintf(fp, "rows: %d, cols: %d\n", ip->num_rows, ip->num_cols));
    ERE(kjb_fprintf(fp, "annotation length: %d\n%s", strlen(annotation),
                    annotation));

    result = write_validity_kiff_image_data(ip, fp);

    ERE(kjb_fclose(fp));

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_image_as_float_kiff
(
    const KJB_image* ip,
    const char*      file_name
)
{
    FILE*       fp;
    kjb_int32   magic_number = KIFF_MAGIC_NUM;
    const char* annotation   = "\n\n:\n";
    int         result;


    /*
    // FIX
    //
    // On some errors, fp does not get closed.
    */

    NRE(fp = kjb_fopen(file_name, "wb"));

    ERE(FIELD_WRITE(fp, magic_number));
    ERE(kjb_fputs(fp, "\nkformat: float\n"));
    ERE(kjb_fprintf(fp, "rows: %d, cols: %d\n", ip->num_rows, ip->num_cols));
    ERE(kjb_fprintf(fp, "annotation length: %d\n%s", strlen(annotation),
                    annotation));

    result = write_float_pixel_image_data(ip, fp);

    ERE(kjb_fclose(fp));

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_image_as_raster
(
    const KJB_image* ip,
    const char*      file_name
)
{
    FILE* fp;
    int   i, j;
    int        row_length;
    Sun_header sun_header;
    unsigned char*     data_row;
    unsigned char*     data_row_pos;
    Pixel*     in_pos;
    float      r_temp;
    float      g_temp;
    float      b_temp;
    kjb_uint32 bigger_if_msb_first = 0;  /* Ignore lint */
    kjb_uint32 less_if_msb_first   = 0;  /* Ignore lint */
    int        msb_first = FALSE;
    kjb_int32  magic     = RASTER_MAGIC_NUM;
    kjb_int32  width     = ip->num_cols;
    kjb_int32  height    = ip->num_rows;
    kjb_int32  depth     = 24;
    kjb_int32  length;
    kjb_int32  type      = RT_FORMAT_RGB;
    kjb_int32  maptype   = RMT_NONE;
    kjb_int32  maplength = 0;


    ((unsigned char*)&bigger_if_msb_first)[ 0 ] = 1;
    ((unsigned char*)&less_if_msb_first)[ 3 ] = 1;

    if (bigger_if_msb_first > less_if_msb_first)
    {
        msb_first = TRUE;
    }

    NRE(fp = kjb_fopen(file_name, "wb"));

    row_length = 3 * ip->num_cols;
    if (IS_ODD(row_length)) row_length++;

    length = row_length * (ip->num_rows);

    if (msb_first)
    {
        sun_header.magic = magic;
        sun_header.width = width;
        sun_header.height = height;
        sun_header.depth = depth;
        sun_header.length = length;
        sun_header.type = type;
        sun_header.maptype = maptype;
        sun_header.maplength = maplength;
    }
    else
    {
        reverse_four_bytes(&magic, &(sun_header.magic));
        reverse_four_bytes(&width, &(sun_header.width));
        reverse_four_bytes(&height, &(sun_header.height));
        reverse_four_bytes(&depth, &(sun_header.depth));
        reverse_four_bytes(&length, &(sun_header.length));
        reverse_four_bytes(&type, &(sun_header.type));
        reverse_four_bytes(&maptype, &(sun_header.maptype));
        reverse_four_bytes(&maplength, &(sun_header.maplength));
    }

    if (FIELD_WRITE(fp, sun_header) == ERROR)
    {
        Error_action save_error_action = get_error_action();

        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        kjb_fclose(fp);
        set_error_action(save_error_action);
        return ERROR;
    }

    data_row = BYTE_MALLOC(row_length);

    if (data_row == NULL)
    {
        Error_action save_error_action = get_error_action();

        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        kjb_fclose(fp);
        set_error_action(save_error_action);
        return ERROR;
    }

    for(i=0; i<height; i++)
    {
        data_row_pos = data_row;

        in_pos = ip->pixels[ i ];

        /*
        // Comprimise : Check for invalid pixel processing method once per row,
        //              and duplicate some code.
        */
        if (    (fs_invalid_pixel_colour.r >= 0)
             && (fs_invalid_pixel_colour.g >= 0)
             && (fs_invalid_pixel_colour.b >= 0)
           )
        {
            for(j=0; j<ip->num_cols; j++)
            {
                if (in_pos->extra.invalid.pixel)
                {
                    /*
                     * In some sense, fs_invalid_pixel_colour should be
                     * Byte_pixel? Anyway, cast is OK because they really should
                     * be in range.
                    */
                    *data_row_pos++ = (unsigned char)fs_invalid_pixel_colour.r;
                    *data_row_pos++ = (unsigned char)fs_invalid_pixel_colour.g;
                    *data_row_pos++ = (unsigned char)fs_invalid_pixel_colour.b;

                }
                else
                {
                    r_temp = in_pos->r;
                    g_temp = in_pos->g;
                    b_temp = in_pos->b;

#ifdef HOW_IT_WAS_APRIL_20_05
                    /*
                     * I changed my mind about this. It makes it hard to debug
                     * invalid versus just happens to be outside (0,255). Note
                     * that if this piece of code is to be resurected.
                     *
                    */

                    /*
                    // Because we are writing to a sun raster, we have to decide
                    // what to do with pixels outside (0, 255). It seems that
                    // setting those to the invalid color makes mose sense, even
                    // though they would not be considered invalid if we were
                    // writing them as floats?
                    */
                    if ((r_temp < 0.0) || (g_temp < 0.0) || (b_temp < 0.0))
                    {
                        /*
                         * In some sense, fs_invalid_pixel_colour should be
                         * Byte_pixel? Anyway, cast is OK because they really should
                         * be in range.
                        */
                        *data_row_pos++ = (unsigned char)fs_invalid_pixel_colour.r;
                        *data_row_pos++ = (unsigned char)fs_invalid_pixel_colour.g;
                        *data_row_pos++ = (unsigned char)fs_invalid_pixel_colour.b;
                    }
                    else
                    {
                        r_temp += EXTRA_FOR_ROUNDING;
                        g_temp += EXTRA_FOR_ROUNDING;
                        b_temp += EXTRA_FOR_ROUNDING;

                        /*
                         * These were 255 on April 20, 2002. However, this means
                         * that 255 show up as invalid due to the
                         * EXTRA_FOR_ROUNDING. If this piece of code gets
                         * ressurected, I think it best to go with 256.0 here.
                        */

                        if (    (r_temp > 256.0) || (g_temp > 256.0)
                             || (b_temp > 256.0)
                           )
                        {
                            /*
                             * In some sense, fs_invalid_pixel_colour should be
                             * Byte_pixel? Anyway, cast is OK because they really should
                             * be in range.
                            */
                            *data_row_pos++ = (unsigned char)fs_invalid_pixel_colour.r;
                            *data_row_pos++ = (unsigned char)fs_invalid_pixel_colour.g;
                            *data_row_pos++ = (unsigned char)fs_invalid_pixel_colour.b;
                        }
                        else
                        {
                            *data_row_pos++ = (unsigned char)r_temp;
                            *data_row_pos++ = (unsigned char)g_temp;
                            *data_row_pos++ = (unsigned char)b_temp;
                        }
                    }
#else
                    if (r_temp < FLT_ZERO) r_temp = FLT_ZERO;
                    r_temp += EXTRA_FOR_ROUNDING;
                    if (r_temp > FLT_255) r_temp = FLT_255;
                    *data_row_pos++ = (unsigned char)r_temp;

                    if (g_temp < FLT_ZERO) g_temp = FLT_ZERO;
                    g_temp += EXTRA_FOR_ROUNDING;
                    if (g_temp > FLT_255) g_temp = FLT_255;
                    *data_row_pos++ = (unsigned char)g_temp;

                    if (b_temp < FLT_ZERO) b_temp = FLT_ZERO;
                    b_temp += EXTRA_FOR_ROUNDING;
                    if (b_temp > FLT_255) b_temp = FLT_255;
                    *data_row_pos++ = (unsigned char)b_temp;
#endif
                }

                in_pos++;
            }
        }
        else
        {
            for(j=0; j<ip->num_cols; j++)
            {
                r_temp = in_pos->r;
                g_temp = in_pos->g;
                b_temp = in_pos->b;

                if (r_temp < FLT_ZERO) r_temp = FLT_ZERO;
                r_temp += EXTRA_FOR_ROUNDING;
                if (r_temp > FLT_255) r_temp = FLT_255;
                *data_row_pos++ = (unsigned char)r_temp;

                if (g_temp < FLT_ZERO) g_temp = FLT_ZERO;
                g_temp += EXTRA_FOR_ROUNDING;
                if (g_temp > FLT_255) g_temp = FLT_255;
                *data_row_pos++ = (unsigned char)g_temp;

                if (b_temp < FLT_ZERO) b_temp = FLT_ZERO;
                b_temp += EXTRA_FOR_ROUNDING;
                if (b_temp > FLT_255) b_temp = FLT_255;
                *data_row_pos++ = (unsigned char)b_temp;

                in_pos++;
            }
        }

        if (IS_ODD(ip->num_cols))
        {
            *data_row_pos++ = 0;
        }

        if (kjb_fwrite(fp, data_row, (size_t)row_length) == ERROR)
        {
            Error_action save_error_action = get_error_action();

            kjb_free(data_row);
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
            kjb_fclose(fp);
            set_error_action(save_error_action);
            return ERROR;
        }
    }

    kjb_free(data_row);

    ERE(kjb_fclose(fp));

    return NO_ERROR;
}

#if 0
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * Temporary "hack" function for writing images with a transparency mask.  Any 
 * alpha value less than 255 will be interpreted as "100% transparent".  
 *
 * This is routine is dispatched from write_image_as_tiff() when the input image 
 * has an alpha channel.  When we eventually add alpha functionality to 
 * write_image_as_tiff(), this function should be removed from the library.
 *
 * Note: Uses ImageMagick's "convert" app with the -transparent flag. Internally, 
 * this routine uses (255,255,255) as a sentinal value to 
 * indicate a transparent pixel.  If the input image has any (255,255,255) 
 * pixels, they are converted to (254,254,254) before writing.  This is an UGLY
 * hack, so please, lets add alpha channels to TIFFs soon!
 *
 * -----------------------------------------------------------------------------
*/

int write_image_with_transparency
(
    const KJB_image* ip,
    const char* out_file_name
)
{
    char               exec_string[ 1000 ];
    static const char* convert_programs[ ]  = { 
        "convert +compress -type truecolor -transparent 'rgb(255,255,255)'",
        "convert -type truecolor -transparent 'rgb(255,255,255)'",
        "convert -transparent 'rgb(255,255,255)'" };
    char* modify_program =  "mogrify -transparent 'rgb(255,255,255)'";

    int                num_convert_programs = sizeof(convert_programs) /sizeof(char*);
    int                i, row, col;
    int                result;
    char               temp_name[ MAX_FILE_NAME_SIZE ];
    int                err = NO_ERROR;

    KJB_image*         tmp_ip = NULL;
    kjb_copy_image(&tmp_ip, ip);

    assert(ip->flags & HAS_ALPHA_CHANNEL);

    /* preprocess to mark transparent pixels */
    for(row = 0; row < ip->num_rows; row++)
    {
        for(col = 0; col < ip->num_cols; col++)
        {
            /* If already the "magic" color for transparency, reassign */
            Pixel* px = &tmp_ip->pixels[row][col];
            if(px->r == 255 && px->g == 255 && px->b == 255)
            {
                px->r = px->g = px->b = 254;
            }

            /* If transparent, set to "magic" color for transparency */
            if(px->extra.alpha < 255.0)
            {
                px->r = px->g = px->b = 255;
            }
        }
    }

    /* Generate a safe temporary file name */
    result = BUFF_GET_TEMP_FILE_NAME(temp_name);
    BUFF_CAT(temp_name, ".tiff");

    tmp_ip->flags &= ~HAS_ALPHA_CHANNEL;
    EGC(err = kjb_write_image(tmp_ip, out_file_name));

    ERE(kjb_sprintf(exec_string, sizeof(exec_string), "%s %s",
            modify_program, out_file_name));

    result = kjb_system(exec_string);

    if (    (result == NO_ERROR)
        && (get_path_type(out_file_name) == PATH_IS_REGULAR_FILE)
       )
    {
      verbose_pso(2, "Conversion succeeded : %s.\n", exec_string);
      goto cleanup;
    }

    set_error("Failure converting image with all conversion programs.");
    add_error("Tried \"%s\"", modify_program);

    cat_error(".");
    err = ERROR;
 
 
    /* Kyle's code below: the problem with this method is that convert doesn't actually
     *  do its job-- it only works if the output image is a .gif.
     *  We have to call the other ImageMagick program "mogrify"
     *  Smart? No. Necessary? Yes--and aliens.
     *  Once convert has been updated and is free of this bug, we should
     *  use Kyle's code instead --Qiyam
     */
  
   /* EGC(err = kjb_write_image(tmp_ip, temp_name)); */

    /* /\* Try to save the image a number of different ways *\/ */
    /* for (i=0; i<num_convert_programs; i++) */
    /* { */
    /*     ERE(kjb_sprintf(exec_string, sizeof(exec_string), "%s %s %s", */
    /*                     convert_programs[i], temp_name, out_file_name)); */

    /*     result = kjb_system(exec_string); */

    /*     if (    (result == NO_ERROR) */
    /*          && (get_path_type(out_file_name) == PATH_IS_REGULAR_FILE) */
    /*        ) */
    /*     { */
    /*         verbose_pso(5, "Conversion succeeded : %s.\n", exec_string); */
    /*         goto cleanup; */
    /*     } */
    /* } */

    /* set_error("Failure converting image with all conversion programs."); */
    /* add_error("Tried \"%s\"", convert_programs[ 0 ]); */

    /* for (i=1; i<num_convert_programs; i++) */
    /* { */
    /*     cat_error(", \"%s\"", convert_programs[ i ]); */
    /* } */

    /* cat_error("."); */
    /* err = ERROR; */
cleanup:
    if (kjb_unlink(temp_name) == ERROR) result = ERROR;
    kjb_free_image(tmp_ip);
    return err;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_TIFF

static int write_image_as_tiff
(
    const KJB_image* ip,
    const char*      file_name
)
{
    TIFF*  tiff_ptr;
    int    num_cols, num_rows, i, j;
    int    scan_line_length;
    unsigned short bits_per_sample;
    Pixel* target_row;
    char   file_name_copy[ MAX_FILE_NAME_SIZE ];
    FILE*  fp;
    kjb_uint16 channels_per_pixel;
    const tsample_t sample = 0;


    verbose_pso(5, "Writing tiff file with builtin routine.\n");

    if (ip->flags & HAS_ALPHA_CHANNEL)
    {
        channels_per_pixel = 4; /* one channel each for R, G, B, and Alpha. */
    }
    else
    {
        channels_per_pixel = 3; /* one channel each for R, G, and B. */
    }

    /*
    // We check this in the set routine, but best to double check. If we expand
    // the options on bps, the analogous test in the set routine should be
    // checked also.
    */
    if ((fs_tiff_write_bps != 8) && (fs_tiff_write_bps != 16))
    {
        set_error("User option \"fs_tiff_write_bps\" is set to %d.",
                  fs_tiff_write_bps);
        add_error("Currently only 8 or 16 bps is supported.");
        return ERROR;
    }

    bits_per_sample = (unsigned short)fs_tiff_write_bps;

    TIFFSetWarningHandler(NULL);

    BUFF_CPY(file_name_copy, file_name);

    /* Do a trial open to use the kjb library error reporting if it does not
    // work.
    */
    if ((fp = kjb_fopen(file_name_copy, "w")) == NULL)
    {
        return ERROR;
    }
    kjb_fclose(fp);

    tiff_ptr=TIFFOpen(file_name_copy, "w");

    if (tiff_ptr == NULL)
    {
        set_bug("Unable to open a file which we were already able to open.");
        return ERROR;
    }

    TIFFSetField(tiff_ptr, TIFFTAG_DOCUMENTNAME, file_name);
    TIFFSetField(tiff_ptr, TIFFTAG_SOFTWARE, "KJB library");

    TIFFSetField(tiff_ptr, TIFFTAG_BITSPERSAMPLE, bits_per_sample);

    num_cols = ip->num_cols;
    num_rows = ip->num_rows;
    TIFFSetField(tiff_ptr, TIFFTAG_IMAGEWIDTH, num_cols);
    TIFFSetField(tiff_ptr, TIFFTAG_IMAGELENGTH, num_rows);

    /* "PHOTOMETRIC_RGB" here works for both RGB and RGBA images.  */
    TIFFSetField(tiff_ptr, TIFFTAG_PHOTOMETRIC, (kjb_uint16)PHOTOMETRIC_RGB);
    TIFFSetField(tiff_ptr, TIFFTAG_COMPRESSION, (kjb_uint16)COMPRESSION_NONE);

    TIFFSetField(tiff_ptr, TIFFTAG_SAMPLESPERPIXEL, channels_per_pixel);
    TIFFSetField(tiff_ptr, TIFFTAG_PLANARCONFIG, (kjb_uint16)1);
    TIFFSetField(tiff_ptr, TIFFTAG_RESOLUTIONUNIT, (kjb_uint16)1);

    scan_line_length = TIFFScanlineSize(tiff_ptr);

    /* TODO:  document this value 0x2000 -- what does that mean?? */
    TIFFSetField(tiff_ptr, TIFFTAG_ROWSPERSTRIP,
                 MAX_OF(1, 0x2000/scan_line_length));

    if (bits_per_sample == 8)
    {
        unsigned char* row;
        unsigned char* row_pos;

        row = BYTE_MALLOC(scan_line_length);

        if (row == NULL)
        {
            TIFFClose(tiff_ptr);
            return ERROR;
        }

        for (i = 0; i<num_rows; i++)
        {
            target_row = ip->pixels[ i ];
            row_pos = row;

            for (j = 0; j<num_cols; j++)
            {
                *row_pos++ = (unsigned char) MAX_OF(FLT_ZERO, MIN_OF(FLT_255,
                                                target_row->r));
                *row_pos++ = (unsigned char) MAX_OF(FLT_ZERO, MIN_OF(FLT_255,
                                                target_row->g));
                *row_pos++ = (unsigned char) MAX_OF(FLT_ZERO, MIN_OF(FLT_255,
                                                target_row->b));
                if (ip->flags & HAS_ALPHA_CHANNEL)
                {
                    *row_pos++ = (unsigned char)MAX_OF(FLT_ZERO,MIN_OF(FLT_255,
                                                target_row->extra.alpha));
                }
                target_row++;
            }

            if (TIFFWriteScanline(tiff_ptr, row, (uint32)i, sample) == -1)
            {
                set_error("Write of tiff file %s failed.", file_name_copy);
                kjb_free(row);
                TIFFClose(tiff_ptr);
                return ERROR;
            }
        }

        kjb_free(row);
    }
    else if (bits_per_sample == 16)
    {
        kjb_uint16* row_16 = UINT16_MALLOC(scan_line_length/2);

        if (row_16 == NULL)
        {
            TIFFClose(tiff_ptr);
            return ERROR;
        }

        UNTESTED_CODE();
        for (i = 0; i<num_rows; i++)
        {
            kjb_uint16* row_16_pos = row_16;
            target_row = ip->pixels[ i ];

            for (j = 0; j<num_cols; j++)
            {
                *row_16_pos++ = (kjb_uint16)(0.5f + MIN_OF(65535.0f,
                          256.0f * MAX_OF(FLT_ZERO, target_row->r)));
                *row_16_pos++ = (kjb_uint16)(0.5f + MIN_OF(65535.0f,
                          256.0f * MAX_OF(FLT_ZERO, target_row->g)));
                *row_16_pos++ = (kjb_uint16)(0.5f + MIN_OF(65535.0f,
                          256.0f * MAX_OF(FLT_ZERO, target_row->b)));
                if (ip->flags & HAS_ALPHA_CHANNEL)
                {
                    *row_16_pos++ = (kjb_uint16)(0.5f + MIN_OF(65535.0f,
                          256.0f * MAX_OF(FLT_ZERO, target_row->extra.alpha)));
                }
                target_row++;
            }

            if (TIFFWriteScanline(tiff_ptr, (unsigned char*)row_16, (uint32)i, sample) == -1)
            {
                set_error("Write of tiff file %s failed.", file_name_copy);
                kjb_free(row_16);
                TIFFClose(tiff_ptr);
                return ERROR;
            }
        }

        kjb_free(row_16);
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
        TIFFClose(tiff_ptr);
        return ERROR;
    }

    TIFFClose(tiff_ptr);
    verbose_pso(10, "Write of tiff file with builtin routine succeeded.\n");
    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_JPEG
#ifndef __C2MAN__

/*
// Error handler is shared with the read routine, defined above.
*/

static int write_image_as_jpeg
(
    const KJB_image* ip,
    const char*      file_name
)
{
#ifdef IMPLEMENT_JPEG_QUALITY
    /* This would be a KJB library option if we wanted it. */
    extern int jpeg_quality;
#endif
    /* This struct contains the JPEG compression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     * It is possible to have several such structures, representing multiple
     * compression/decompression processes, in existence at once.  We refer
     * to any one struct (and its associated working data) as a "JPEG object".
     */
    struct jpeg_compress_struct cinfo;
    /* This struct represents a JPEG error handler.  It is declared separately
     * because applications often want to supply a specialized error handler
     * (see the second half of this file for an example).  But here we just
     * take the easy way out and use the standard error handler, which will
     * print a message on stderr and call exit() if compression fails.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct my_error_mgr jerr;
    int num_cols = ip->num_cols;
    int num_rows = ip->num_rows;
    int i, j;
    int          scan_line_length;
    Pixel* target_row;
    unsigned char*        row = NULL;
    unsigned char*        row_pos;
    FILE*        fp;
    int result;

    verbose_pso(5, "Writing jpeg file with builtin routine.\n");

    if ((fp = kjb_fopen(file_name, "wb")) == NULL)
    {
        return ERROR;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    if (setjmp(jerr.setjmp_buffer))
    {
        Error_action save_error_action = get_error_action();

        UNTESTED_CODE();

        kjb_free(row);
        set_error("Limited JPEG writer failed to write %F.", fp);
        add_error(jpeg_error_buff); 
        add_error("We will attempt to write a different format and convert to JPEG.");

        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        kjb_fclose(fp);
        set_error_action(save_error_action);

        jpeg_destroy_compress(&cinfo);

        return ERROR;
    }

    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    /* Step 2: specify data destination (eg, a file) */
    /* Note: steps 2 and 3 can be done in either order. */
    jpeg_stdio_dest(&cinfo, fp);

    /* Step 3: set parameters for compression */

    /* First we supply a description of the input image.
     * Four fields of the cinfo struct must be filled in:
     */
    cinfo.image_width = num_cols;     /* image width and height, in pixels */
    cinfo.image_height = num_rows;;
    cinfo.input_components = 3;           /* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB;       /* colorspace of input image */

    /* Now use the library's routine to set default compression parameters.
     * (You must set at least cinfo.in_color_space before calling this,
     * since the defaults depend on the source color space.)
     */
    jpeg_set_defaults(&cinfo);

    /* Now you can set any non-default parameters you wish to.
     * Here we just illustrate the use of quality (quantization table) scaling:
     */
#ifdef IMPLEMENT_JPEG_QUALITY
    UNTESTED_CODE();
    jpeg_set_quality(&cinfo, jpeg_quality,
                     TRUE /* limit to baseline-JPEG values */);
#endif

    /* Step 4: Start compressor */

    /* TRUE ensures that we will write a complete interchange-JPEG file.
     * Pass TRUE unless you are very sure of what you're doing.
     */
    jpeg_start_compress(&cinfo, TRUE);

    scan_line_length = 3 * num_cols;

    row = BYTE_MALLOC(scan_line_length);

    if (row == NULL)
    {
        Error_action save_error_action = get_error_action();

        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
        kjb_fclose(fp);
        set_error_action(save_error_action);

        return ERROR;
    }

    for (i = 0; i<num_rows; i++)
    {
        target_row = ip->pixels[ i ];
        row_pos = row;

        for (j = 0; j<num_cols; j++)
        {
            *row_pos++ = (unsigned char)MAX_OF(FLT_ZERO, MIN_OF(FLT_255, target_row->r));
            *row_pos++ = (unsigned char)MAX_OF(FLT_ZERO, MIN_OF(FLT_255, target_row->g));
            *row_pos++ = (unsigned char)MAX_OF(FLT_ZERO, MIN_OF(FLT_255, target_row->b));
            target_row++;
        }

        (void) jpeg_write_scanlines(&cinfo, &row, 1);
    }

    jpeg_finish_compress(&cinfo);

    kjb_free(row);

    result = NO_ERROR;

    if (kjb_fclose(fp) == ERROR)
    {
        result = ERROR;
    }

    jpeg_destroy_compress(&cinfo);

    if (result != ERROR)
    {
        verbose_pso(10,"Write of jpeg file with builtin routine succeeded.\n");
    }

    return result;
}

#endif
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_validity_kiff_image_data(const KJB_image* ip, FILE* fp)
{
    int    num_rows, num_cols, i, j;
    Pixel* in_pos;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            ERE(FIELD_WRITE(fp, *in_pos));
            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_float_pixel_image_data(const KJB_image* ip, FILE* fp)
{
    int    num_rows, num_cols, i, j;
    Pixel* in_pos;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    if (fs_invalidate_negative_pixels)
    {
        for (i=0; i<num_rows; i++)
        {
            in_pos = ip->pixels[ i ];

            for (j=0; j<num_cols; j++)
            {
                if ((in_pos->extra.invalid.r) || (in_pos->extra.invalid.pixel))
                {
                    float temp_float = -(in_pos->r);
                    ERE(FIELD_WRITE(fp, temp_float));
                }
                else
                {
                    ERE(FIELD_WRITE(fp, in_pos->r));
                }

                if ((in_pos->extra.invalid.g) || (in_pos->extra.invalid.pixel))
                {
                    float temp_float = -(in_pos->g);
                    ERE(FIELD_WRITE(fp, temp_float));
                }
                else
                {
                    ERE(FIELD_WRITE(fp, in_pos->g));
                }

                if ((in_pos->extra.invalid.b) || (in_pos->extra.invalid.pixel))
                {
                    float temp_float = -(in_pos->b);
                    ERE(FIELD_WRITE(fp, temp_float));
                }
                else
                {
                    ERE(FIELD_WRITE(fp, in_pos->b));
                }

                in_pos++;
            }
        }
    }
    else
    {
        for (i=0; i<num_rows; i++)
        {
            in_pos = ip->pixels[ i ];

            for (j=0; j<num_cols; j++)
            {
                ERE(FIELD_WRITE(fp, in_pos->r));
                ERE(FIELD_WRITE(fp, in_pos->g));
                ERE(FIELD_WRITE(fp, in_pos->b));
                in_pos++;
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_float_channel_image_data(const KJB_image* ip, FILE* fp)
{
    int    num_rows, num_cols, i, j;
    Pixel* in_pos;


    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i=0; i<num_rows; i++)
    {
        in_pos = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            ERE(FIELD_WRITE(fp, in_pos->r));
            in_pos++;
        }
    }

    for (i=0; i<ip->num_rows; i++)
    {
        in_pos = ip->pixels[ i ];

        for (j=0; j<ip->num_cols; j++)
        {
            ERE(FIELD_WRITE(fp, in_pos->g));
            in_pos++;
        }
    }

    for (i=0; i<ip->num_rows; i++)
    {
        in_pos = ip->pixels[ i ];

        for (j=0; j<ip->num_cols; j++)
        {
            ERE(FIELD_WRITE(fp, in_pos->b));
            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                display_matrix
 *
 * Displays a matrix as a black and white image
 *
 * This routine is used to display a matrix as a black and white image using
 * kjb_display_image(). 
 *
 * Returns:
 *     On success, the image number is returned. This number can be used as a
 *     handle to manipulate the image from the program (actually, not much has
 *     been implemented here, but you can close the image). On failure, ERROR is
 *     returned on failure, with and appropriate error message being set .
 *
 * Index: matrices images, image I/O, image display
 *
 * -----------------------------------------------------------------------------
*/

int display_matrix(const Matrix* mp, const char* title)
{
    KJB_image* ip = NULL;
    int result = NO_ERROR;

    ERE(matrix_to_bw_image(mp, &ip));
    result = kjb_display_image(ip, title);
    kjb_free_image(ip);
 
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                kjb_display_image
 *
 * Displays an image
 *
 * This routine is used to display an image. It uses an external program to
 * actually do the displaying. The program used is the first program in a hard
 * coded list found in PATH. The list is subject to change, but on 17/08/01 it
 * was:
 *
 * |        kjb_display
 * |        display
 * |        xv
 *
 * An image title can be specified using the parameter "title". If you don't
 * care to title the displayed image, then the title paramter can be NULL.
 * Currently, all double quotes in the title string are converted to single
 * quotes, since double quotes are used down stream to delimit the title.
 *
 * Images are displayed either with (default) or without the recomended gamma
 * correction. This is controlled through the "gamma" option which is normally
 * exposed to the user.
 *
 * Images may also be corrected by a matrix before display (not by default).
 * This is controlled throught the "display-matrix-file" option which is also
 * normally exposed to the user.
 *
 * Images may be scaled so that their maximum value is 255 (not by default).
 * This is controlled by the "scale-by-max-rgb" option which is normally exposed
 * to the user. 
 *
 * Images may be scaled so that their minimum value is 0 and their maximum value
 * is 255 (not by default).  This is controlled by the "adust-image-range"
 * option which is normally exposed to the user. 
 *
 * Returns:
 *     On success, the image number is returned. This number can be used as a
 *     handle to manipulate the image from the program (actually, not much has
 *     been implemented here, but you can close the image). On failure, ERROR is
 *     returned on failure, with and appropriate error message being set .
 *
 *
 * Related:  close_displayed_image
 *
 * Index: images, image I/O, image display
 *
 * -----------------------------------------------------------------------------
*/

int kjb_display_image(const KJB_image* ip, const char* title)
{

    return display_any_image((const void*)ip, title,
                             write_image_for_display);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int fork_display_image(const KJB_image* ip, const char* title)
{

    return fork_display_any_image((const void*)ip, title,
                                  write_image_for_display);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_image_for_display(const void* ip, char* title)
{
    KJB_image* range_mapped_ip = NULL;
    KJB_image* max_scaled_ip = NULL;
    KJB_image* map_ip = NULL;
    KJB_image* log_ip = NULL;
    KJB_image* gamma_corrected_ip = NULL;
    int        result = NO_ERROR;


    if ((result != ERROR) && (fs_display_matrix_mp != NULL))
    {
        result = post_map_image(&map_ip, (const KJB_image*)ip,
                                fs_display_matrix_mp);
        ip = map_ip;
    }

    if ((result != ERROR) && (fs_adjust_image_range))
    {
        result = adjust_image_range(&range_mapped_ip, (const KJB_image*)ip);
        ip = range_mapped_ip;
    }

    if ((result != ERROR) && (fs_display_log))
    {
        result = log_one_plus_image(&log_ip, (const KJB_image*)ip);
        ip = log_ip;
    }

    if ((result != ERROR) && (fs_scale_by_max_rgb))
    {
        result = scale_image_by_max(&max_scaled_ip, (const KJB_image*)ip);
        ip = max_scaled_ip;
    }

    if ((result != ERROR) && (fs_do_gamma_correction))
    {
        result = gamma_correct_image(&gamma_corrected_ip, (const KJB_image*)ip,
                                     (Vector*)NULL);
        ip = gamma_corrected_ip;
    }

    if (result != ERROR)
    {
        result = kjb_write_image((const KJB_image*)ip, title);
    }

    kjb_free_image(map_ip);
    kjb_free_image(log_ip);
    kjb_free_image(range_mapped_ip);
    kjb_free_image(max_scaled_ip);
    kjb_free_image(gamma_corrected_ip);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

