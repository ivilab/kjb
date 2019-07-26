
/* $Id: i_float.h 21673 2017-08-05 21:28:22Z kobus $ */

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

#ifndef I_FLOAT_INCLUDED
#define I_FLOAT_INCLUDED


#include "l/l_def.h"

#ifdef __cplusplus
extern "C" {
#ifdef COMPILING_CPLUSPLUS_SOURCE
namespace kjb_c {
#endif
#endif


/* -------------------------------------------------------------------------- */


/*
// These must be different powers of two. VALID_PIXEL is a synomym for FALSE.
// INVALID_PIXEL is for general invalidity. Also, if we need to use the MSB,
// then we have to ax the debugging code which checks for initialization.
*/
#define VALID_PIXEL           0x00     /* Must be same as FALSE  */
#define INVALID_PIXEL         0x01     /* Best if same as TRUE   */
#define DARK_PIXEL            0x02
#define NOISY_PIXEL           0x04
#define ILLEGAL_MATH_OP_PIXEL 0x08

/* FIX -- better names ? */
#define TRUNCATED_PIXEL       0x10    /* Similar to clip, but computed result */
#define CLIPPED_PIXEL         0x80    /* Similar to truncated, but on IO.     */


typedef enum Kiff_data_type
{
    FLOAT_WITH_VALIDITY_KIFF,
    FLOAT_KIFF,
    BYTE_KIFF
}
Kiff_data_type;


/* =============================================================================
 *                             Invalid_pixel
 *
 * Invalid_pixel type for Pixel type
 *
 * This type is used mostly for the type Pixel, which in turn is usually
 * used for the type KJB_image.
 *
 * The structure contains a validity field for each channel, and the pixel taken
 * as a whole. The validy may be one of:
 *
 * |   VALID_PIXEL
 * |   INVALID_PIXEL
 * |   DARK_PIXEL
 * |   NOISY_PIXEL
 * |   ILLEGAL_MATH_OP_PIXEL
 * |   TRUNCATED_PIXEL
 * |   CLIPPED_PIXEL
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/
    
typedef struct Invalid_pixel
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char pixel;
}
Invalid_pixel;


/* =============================================================================
 *                             Pixel_extra
 *
 * Union for Pixel type that enables Invalid_pixel and alpha.
 *
 * This union is part of the definition of Pixel. For compatability and reduced
 * development effort, we want to keep the original disk layout of 16 bytes, so
 * we have to choose between making use of Invalid_pixel or the alpha channel,
 * hence a union.
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/

typedef union Pixel_extra
{
    Invalid_pixel invalid;
    float alpha;
} 
Pixel_extra;

#define VALID_RGB {VALID_PIXEL, VALID_PIXEL, VALID_PIXEL, VALID_PIXEL}

/* =============================================================================
 *                             Pixel
 *
 * Pixel type for KJB_image type
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Pixel
{
    float        r;
    float        g;
    float        b;
    Pixel_extra  extra;
}
Pixel;


/* =============================================================================
 *                             Pixel_info
 *
 * Type for lists of pixels
 *
 * This type is used when pixels are to be represented by their coordinates.
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/

typedef struct Pixel_info
{
    int i;
    int j;
}
Pixel_info;


/* =============================================================================
 *                             KJB_image
 *
 * Type for floating point images
 *
 * This is the basic image type for the KJB library. There is also a Byte image
 * format, but Byte images are only used by a few routines.
 *
 * The sizes (num_rows and num_cols) must both be nonnegative.
 * Conventionally, num_rows and num_cols are both positive;
 * however, it is also acceptable to have num_rows and num_cols both zero.
 * Anything else (such as a mix of positive and zero sizes) is regarded as a
 * bug in the calling program -- set_bug(3) is called and (if it returns)
 * ERROR is returned.
 *
 * Index: images
 *
 * -----------------------------------------------------------------------------
*/

/*
 * If we add resize capability via max_num_rows and max_num_cols, see the code
 * for Matrix.  Don't forget to change any copy code that uses memcpy.
*/
#define HAS_ALPHA_CHANNEL     0x01    /* Not invalid, per-se...  */
typedef struct KJB_image
{
    int     num_rows;
    int     num_cols;
    int     read_only;
    int     flags;
    Pixel** pixels;
}
KJB_image;

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION

#   define kjb_create_image(x, y) \
            debug_create_image(x, y, __FILE__, __LINE__)

#   define get_target_image(x, y, z) \
            debug_get_target_image(x, y, z, __FILE__, __LINE__)

    KJB_image* debug_create_image
    (
        int         num_rows,
        int         num_cols,
        const char* file_name,
        int         line_number
    );

    int debug_get_target_image
    (
        KJB_image** out_ipp,
        int         num_rows,
        int         num_cols,
        const char* file_name,
        int         line_number
    );
#else
    KJB_image* kjb_create_image(int num_rows, int num_cols);

    int get_target_image(KJB_image** out_ipp, int num_rows, int num_cols);
#endif

#ifdef TRACK_MEMORY_ALLOCATION
    void check_image_initialization(const KJB_image* ip);
#else
#    define check_image_initialization(x)
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             verify_image
 *
 * (MACRO) Debug verification of a image
 *
 * When TEST is defined (typically this is "development" mode), verify_image
 * checks that all elements of a image are valid numbers. NULL input is
 * considered valid. If invalid numbers are found, then they are set to zero,
 * and a warning is printed.
 *
 * Wen TEST is not defined (typically "production" mode), then
 * verify_image becomes a NOP.
 *
 * Index: debugging
 *
 * -----------------------------------------------------------------------------
*/
#ifdef __C2MAN__
    void verify_image(const KJB_image* ip);
#else
#    ifdef TEST
#       define verify_image(x)                     debug_verify_image(x, __FILE__, __LINE__)

        void debug_verify_image
        (
            const KJB_image* ip,
            const char*   file_name,
            int           line_number
        );

#    else
#        define verify_image(x)
#    endif
#endif


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void kjb_free_image(KJB_image* ip);

int kjb_copy_image(KJB_image** target_ipp, const KJB_image* source_ip);

int get_zero_image(KJB_image** target_ipp, int num_rows, int num_cols);

int get_invalid_zero_image
(
    KJB_image** target_ipp,
    int         num_rows,
    int         num_cols
);

int get_initialized_image
(
    KJB_image** target_ipp,
    int         num_rows,
    int         num_cols,
    Pixel*      initial_value_ptr
);

int get_initialized_image_2
(
    KJB_image** target_ipp,
    int         num_rows,
    int         num_cols,
    int         r,
    int         g,
    int         b
);

int get_image_window
(
    KJB_image**      target_ipp,
    const KJB_image* source_ip,
    int              row_offset,
    int              col_offset,
    int              num_target_rows,
    int              num_target_cols
);

int is_black_and_white
(
    const KJB_image * ip
);

#ifdef __cplusplus
#ifdef COMPILING_CPLUSPLUS_SOURCE
}
#endif
}
#endif

#endif

