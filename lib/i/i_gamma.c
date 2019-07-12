
/* $Id: i_gamma.c 20918 2016-10-31 22:08:27Z kobus $ */

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
#include "i/i_gamma.h"


#define USE_PCD_GAMMA

#define PCD_LINEAR_WHITE_RGB (255.0 / 2.0)
#define PCD_GAMMA_WHITE_RGB (247.0)

#define LUT_PRECISION_FACTOR  5
#define LUT_PRECISION   (LUT_PRECISION_FACTOR * 255)
#define FLT_LUT_PRECISION ((float)LUT_PRECISION)
#define LUT_RANGE_FACTOR  3
#define LUT_SIZE      (1 + LUT_RANGE_FACTOR * LUT_PRECISION)

#define DEFAULT_GAMMA    2.2

#ifndef DATA_DIR
#    define DATA_DIR                  "data"
#endif

#ifndef GAMMA_DIR
#    define GAMMA_DIR                 "monitor"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int    fs_pcd_ycc_to_rgb_shape_function = 2;

static Lut* fs_specified_r_gamma_lut_ptr = NULL;
static Lut* fs_specified_g_gamma_lut_ptr = NULL;
static Lut* fs_specified_b_gamma_lut_ptr = NULL;
static Lut* fs_default_r_gamma_lut_ptr = NULL;
static Lut* fs_default_g_gamma_lut_ptr = NULL;
static Lut* fs_default_b_gamma_lut_ptr = NULL;
static const Lut* fs_current_r_gamma_lut_ptr;
static const Lut* fs_current_g_gamma_lut_ptr;
static const Lut* fs_current_b_gamma_lut_ptr;

static Lut* fs_specified_r_linear_lut_ptr = NULL;
static Lut* fs_specified_g_linear_lut_ptr = NULL;
static Lut* fs_specified_b_linear_lut_ptr = NULL;
static Lut* fs_default_r_linear_lut_ptr = NULL;
static Lut* fs_default_g_linear_lut_ptr = NULL;
static Lut* fs_default_b_linear_lut_ptr = NULL;
static const Lut* fs_current_r_linear_lut_ptr;
static const Lut* fs_current_g_linear_lut_ptr;
static const Lut* fs_current_b_linear_lut_ptr;

/*
// Values for reversing linearize PCD
//
static double fs_linear_white_point = PCD_LINEAR_WHITE_RGB;
static double fs_gamma_white_point  = PCD_GAMMA_WHITE_RGB;
*/

static double fs_linear_white_point = 255.0;
static double fs_gamma_white_point  = 255.0;

/* -------------------------------------------------------------------------- */

static FILE* open_gamma_config_file(const char* file_name);
static int initialize_default_gamma_correction(void);
static int initialize_specified_gamma_correction(const Vector* gamma_vp);
static int initialize_specified_gamma_inversion(const Vector* gamma_vp);
static int initialize_default_gamma_inversion  (void);

#ifdef TRACK_MEMORY_ALLOCATION
    static void prepare_memory_cleanup(void);
    static void free_allocated_static_data(void);
#endif

/* -------------------------------------------------------------------------- */

int set_gamma_options(const char* option, const char* value)
{
    char   lc_option[ 100 ];
    int    temp_int;
    double temp_double;
    int    result             = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "pcd-lut")
          || match_pattern(lc_option, "pcd-output-lut")
          || match_pattern(lc_option, "pcd-ycc-shape-function")
          || match_pattern(lc_option, "pcd-ycc-to-rgb-shape-function")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("pcd-ycc-to-rgb-shape-function = %d\n",
                    fs_pcd_ycc_to_rgb_shape_function));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("PCD ycc to rgb shape function is %d.\n",
                    fs_pcd_ycc_to_rgb_shape_function));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            ASSERT(temp_int >= 0);

            if (temp_int > 2)
            {
                set_error("Current choices for ycc to rgb shape function are 0,1, and 2.");
                return ERROR;
            }

            fs_pcd_ycc_to_rgb_shape_function = temp_int;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "linear-white-point"))
         || (match_pattern(lc_option, "linear-white"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("linear-white-point = %.2f\n", fs_linear_white_point));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Linear white point is %.2f.\n", fs_linear_white_point));
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_linear_white_point = temp_double;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "gamma-white-point"))
         || (match_pattern(lc_option, "gamma-white"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("gamma-white-point = %.2f\n", fs_gamma_white_point));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Linear white point is %.2f.\n", fs_gamma_white_point));
        }
        else
        {
            ERE(ss1d(value, &temp_double));
            fs_gamma_white_point = temp_double;
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            gamma_correct_image
 *
 * Gamma corrects images
 *
 * This routine puts a gamma corection version of the input image (in_ip) in
 * the output location (out_ipp). The output image is created or resized as
 * necessary with the usual KJB library semantics.
 *
 * The third argument is a vector of three gamma values one for each channel. If
 * it is NULL, then default gamma is used, either from lookup tables in standard
 * places (usually not available), or, if that fails, using standard gamma
 * correction.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * -----------------------------------------------------------------------------
 */

int gamma_correct_image
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    const Vector*    gamma_vp
)
{


    ERE(kjb_copy_image(out_ipp, in_ip));
    ERE(ow_gamma_correct_image(*out_ipp, gamma_vp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_gamma_correct_image
 *
 * Gamma corrects images
 *
 * This routine over-writes an image with a gamma corected version.
 *
 * The second argument is a vector of three gamma values one for each channel. If
 * it is NULL, then default gamma is used, either from lookup tables in standard
 * placess (usually not available), or, if that fails, using standard gamma
 * correction.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * -----------------------------------------------------------------------------
 */

int ow_gamma_correct_image(KJB_image* in_ip, const Vector* gamma_vp)
{
    int    num_rows, num_cols;
    Pixel* in_pos;
    int    i, j;


    if (gamma_vp == NULL)
    {
        ERE(initialize_default_gamma_correction());
    }
    else
    {
        ERE(initialize_specified_gamma_correction(gamma_vp));
    }

    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

#ifdef DEF_OUT
    /*
     * Ideally this warning would stay until the comment below is resolved, but
     * it gives too many messages while running "it".
    */
    if (! IS_EQUAL_DBL(fs_linear_white_point, PCD_GAMMA_WHITE_RGB))
    {
        /* This warning should stay until comment below is resolved. */
        warn_pso("If the image source is PCD, then the linear white point should perhaps be %.3f (not %.3f).\n",
                 PCD_GAMMA_WHITE_RGB, fs_linear_white_point);
    }

    /*
     *
     * FIXME  ...   BUGGY ... CHECK
     *
     * If this is supposed to apply pcd gamma (current default?), then
     * the fs_linear_white_point needs to be the PCD_GAMMA_WHITE_RGB
     * which it is NOT by default!
     *
     * (Change all three channels!)
    */
#endif

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            double r, g, b;
            double dr, dg, db;

            r = (double)in_pos->r / fs_linear_white_point;
            if (apply_lut_2(fs_current_r_gamma_lut_ptr, r, &dr) == ERROR)
            {
                dbe(r);
                dbi(i);
                dbi(j);
                dbe((double)in_pos->r);
                return ERROR;
            }
            in_pos->r = dr * fs_gamma_white_point;

            g = (double)in_pos->g / fs_gamma_white_point;
            if (apply_lut_2(fs_current_g_gamma_lut_ptr, g, &dg) == ERROR)
            {
                dbe(g);
                dbi(i);
                dbi(j);
                dbe((double)in_pos->g);
                return ERROR;
            }
            in_pos->g = dg * fs_gamma_white_point;

            b = (double)in_pos->b / fs_gamma_white_point;
            if (apply_lut_2(fs_current_b_gamma_lut_ptr, b, &db) == ERROR)
            {
                dbe(b);
                dbi(i);
                dbi(j);
                dbe((double)in_pos->b);
                return ERROR;
            }
            in_pos->b = db * fs_gamma_white_point;

            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            invert_image_gamma
 *
 * Inverts image gamma
 *
 * This routine puts a linear version of a gamma corected image (in_ip) in the
 * output location (out_ipp). The output image is created or resized as
 * necessary with the usual KJB library semantics.
 *
 * The third argument is a vector of three gamma values one for each channel. If
 * it is NULL, then default gamma is used, either from lookup tables in standard
 * placess (usually not available), or, if that fails, using standard gamma
 * correction.
 *
 * Note:
 *     This is a separate facility to the linearization mechanism developed in
 *     the context of cameras (see i_offset.c). These two facilities have
 *     converged to be very similar. This suite of routines is meant to be
 *     inverses of the corresponding gamma ones, and were developed in the
 *     context of corercting images from random sources, rather than a
 *     calibrated camera where we generally have very specific lookup tables.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * -----------------------------------------------------------------------------
 */

int invert_image_gamma
(
    KJB_image**      out_ipp,
    const KJB_image* in_ip,
    Vector*          gamma_vp
)
{


    ERE(kjb_copy_image(out_ipp, in_ip));
    ERE(ow_invert_image_gamma(*out_ipp, gamma_vp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            ow_invert_image_gamma
 *
 * Inverts image gamma
 *
 * This routine overwrites a gamma corected image with a linear version.
 *
 * The second argument is a vector of three gamma values one for each channel. If
 * it is NULL, then default gamma is used, either from lookup tables in standard
 * placess (usually not available), or, if that fails, using standard gamma
 * correction.
 *
 * Note:
 *     This is a separate facility to the linearization mechanism developed in
 *     the context of cameras (see i_offset.c). These two facilities have
 *     converged to be very similar. This suite of routines is meant to be
 *     inverses of the corresponding gamma ones, and were developed in the
 *     context of corercting images from random sources, rather than a
 *     calibrated camera where we generally have very specific lookup tables.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * -----------------------------------------------------------------------------
 */

int ow_invert_image_gamma(KJB_image* in_ip, Vector* gamma_vp)
{
    int    num_rows, num_cols;
    Pixel* in_pos;
    int    i, j;


    if (gamma_vp == NULL)
    {
        ERE(initialize_default_gamma_inversion());
    }
    else
    {
        ERE(initialize_specified_gamma_inversion(gamma_vp));
    }

    num_rows = in_ip->num_rows;
    num_cols = in_ip->num_cols;

#ifdef DEF_OUT
    /*
     * Ideally this warning would stay until the comment below is resolved, but
     * it gives too many messages while running "it".
    */
    if (! IS_EQUAL_DBL(fs_linear_white_point, PCD_GAMMA_WHITE_RGB))
    {
        /* This warning should stay until comment below is resolved. */
        warn_pso("If the image source is PCD, then the linear white point should perhaps be %.3f (not %.3f).\n",
                 PCD_GAMMA_WHITE_RGB, fs_linear_white_point);
    }

    /*
     *
     * FIXME  ...   BUGGY ... CHECK
     *
     * If this is supposed to apply pcd gamma (current default?), then
     * the fs_linear_white_point needs to be the PCD_GAMMA_WHITE_RGB
     * which it is NOT by default!
     *
     * (Change all three channels!)
    */
#endif

    for (i=0; i<num_rows; i++)
    {
        in_pos  = in_ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            double r, g, b;
            double dr, dg, db;

            r = (double)in_pos->r / fs_linear_white_point;
            if (apply_lut_2(fs_current_r_linear_lut_ptr, r, &dr) == ERROR)
            {
                dbe(r);
                dbi(i);
                dbi(j);
                dbe((double)in_pos->r);
                return ERROR;
            }
            in_pos->r = dr * fs_gamma_white_point;

            g = (double)in_pos->g / fs_linear_white_point;
            if (apply_lut_2(fs_current_g_linear_lut_ptr, g, &dg) == ERROR)
            {
                dbe(g);
                dbi(i);
                dbi(j);
                dbe((double)in_pos->g);
                return ERROR;
            }
            in_pos->g = dg * fs_gamma_white_point;

            b = (double)in_pos->b / fs_linear_white_point;
            if (apply_lut_2(fs_current_b_linear_lut_ptr, b, &db) == ERROR)
            {
                dbe(b);
                dbi(i);
                dbi(j);
                dbe((double)in_pos->b);
                return ERROR;
            }
            in_pos->b = db * fs_gamma_white_point;

            in_pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_specified_gamma_correction(const Vector* gamma_vp)
{
    int    i;
    double inverted_r_gamma;
    double inverted_g_gamma;
    double inverted_b_gamma;
    double r_gamma;
    double g_gamma;
    double b_gamma;


    /* Untested since going to Luts for everything. */
    UNTESTED_CODE();

    if (gamma_vp->length != 3)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    verbose_pso(3, "Using gamma correction (%5.2f, %5.2f, %5.2f).\n",
                gamma_vp->elements[ 0 ], gamma_vp->elements[ 1 ],
                gamma_vp->elements[ 2 ]);

    r_gamma = gamma_vp->elements[ 0 ];
    g_gamma = gamma_vp->elements[ 1 ];
    b_gamma = gamma_vp->elements[ 2 ];

    inverted_r_gamma = 1.0 / r_gamma;
    inverted_g_gamma = 1.0 / g_gamma;
    inverted_b_gamma = 1.0 / b_gamma;

    ERE(get_target_lut(&fs_specified_r_gamma_lut_ptr, LUT_SIZE, 0.0,
                       1.0 / FLT_LUT_PRECISION));

    ERE(get_target_lut(&fs_specified_g_gamma_lut_ptr, LUT_SIZE, 0.0,
                       1.0 / FLT_LUT_PRECISION));

    ERE(get_target_lut(&fs_specified_b_gamma_lut_ptr, LUT_SIZE, 0.0,
                       1.0 / FLT_LUT_PRECISION));

    fs_specified_r_gamma_lut_ptr->lut_vp->elements[ 0 ] = 0.0;
    fs_specified_g_gamma_lut_ptr->lut_vp->elements[ 0 ] = 0.0;
    fs_specified_b_gamma_lut_ptr->lut_vp->elements[ 0 ] = 0.0;

    for (i=1; i<LUT_SIZE; i++)
    {
        fs_specified_r_gamma_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, inverted_r_gamma);
        fs_specified_g_gamma_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, inverted_g_gamma);
        fs_specified_b_gamma_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, inverted_b_gamma);
    }

    fs_current_r_gamma_lut_ptr = fs_specified_r_gamma_lut_ptr;
    fs_current_g_gamma_lut_ptr = fs_specified_g_gamma_lut_ptr;
    fs_current_b_gamma_lut_ptr = fs_specified_b_gamma_lut_ptr;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_default_gamma_correction(void)
{
    static int first_time = TRUE;
    int        i;
    FILE*      r_lut_fp;
    FILE*      g_lut_fp;
    FILE*      b_lut_fp;
#ifndef USE_PCD_GAMMA
    double     inverted_gamma = 1.0 / DEFAULT_GAMMA;
    double     gamma = DEFAULT_GAMMA;
#endif
    int        good_data      = TRUE;


    if (first_time)
    {
#ifdef TRACK_MEMORY_ALLOCATION
        prepare_memory_cleanup();
#endif

        r_lut_fp = open_gamma_config_file("gamma_red.lut");
        g_lut_fp = open_gamma_config_file("gamma_green.lut");
        b_lut_fp = open_gamma_config_file("gamma_blue.lut");

        if (    (r_lut_fp != NULL)
             && (g_lut_fp != NULL)
             && (b_lut_fp != NULL)
           )
        {
            /*
            // Untested since going to luts for help with new PCD / shaping
            // approach.
            */
            UNTESTED_CODE();

            verbose_pso(2, "Red gamma correction LUT is %F\n", r_lut_fp);
            verbose_pso(2, "Green gamma correction LUT is %F\n", g_lut_fp);
            verbose_pso(2, "Blue gamma correction LUT is %F\n", b_lut_fp);

            SKIP_HEAP_CHECK_2();

            if (    (fp_read_lut(&fs_default_r_gamma_lut_ptr, r_lut_fp) == ERROR)
                 || (fp_read_lut(&fs_default_g_gamma_lut_ptr, g_lut_fp) == ERROR)
                 || (fp_read_lut(&fs_default_b_gamma_lut_ptr, b_lut_fp) == ERROR)
               )
            {
                good_data = FALSE;
                kjb_print_error();
            }

            CONTINUE_HEAP_CHECK_2();

            kjb_fclose(r_lut_fp);
            kjb_fclose(g_lut_fp);
            kjb_fclose(b_lut_fp);
        }
        else
        {
            verbose_pso(2, "Gamma correction lookup tables for at least ");
            verbose_pso(2, "one channel is not available.\n");

            good_data = FALSE;
        }

        if ( ! good_data)
        {
#ifdef USE_PCD_GAMMA
            verbose_pso(2, "Using pcd gamma correction.\n");
#else
            verbose_pso(2, "Using standard gamma correction with ");
            verbose_pso(2, "gamma %3.1f.\n", DEFAULT_GAMMA);
#endif

            SKIP_HEAP_CHECK_2();

            ERE(get_target_lut(&fs_default_r_gamma_lut_ptr, LUT_SIZE, 0.0,
                               1.0 / FLT_LUT_PRECISION));

            ERE(get_target_lut(&fs_default_g_gamma_lut_ptr, LUT_SIZE, 0.0,
                               1.0 / FLT_LUT_PRECISION));

            ERE(get_target_lut(&fs_default_b_gamma_lut_ptr, LUT_SIZE, 0.0,
                               1.0 / FLT_LUT_PRECISION));

            CONTINUE_HEAP_CHECK_2();

            fs_default_r_gamma_lut_ptr->lut_vp->elements[ 0 ] = 0.0;
            fs_default_g_gamma_lut_ptr->lut_vp->elements[ 0 ] = 0.0;
            fs_default_b_gamma_lut_ptr->lut_vp->elements[ 0 ] = 0.0;

            for (i=1; i<LUT_SIZE; i++)
            {

#ifdef USE_PCD_GAMMA
                /*
                // Perhaps most useful, as the previous behavior is still
                // available with g=2.2,
                */

#define APPLY_PCD_GAMMA(x) \
                ((x > 0.018) ? 1.099 * pow(x, 0.45) - 0.099 : 4.5 * x)

                fs_default_r_gamma_lut_ptr->lut_vp->elements[ i ] = APPLY_PCD_GAMMA(((double)i) / FLT_LUT_PRECISION);
                fs_default_g_gamma_lut_ptr->lut_vp->elements[ i ] = APPLY_PCD_GAMMA(((double)i) / FLT_LUT_PRECISION);
                fs_default_b_gamma_lut_ptr->lut_vp->elements[ i ] = APPLY_PCD_GAMMA(((double)i) / FLT_LUT_PRECISION);

#else
                fs_default_r_gamma_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, inverted_gamma);
                fs_default_g_gamma_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, inverted_gamma);
                fs_default_b_gamma_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, inverted_gamma);
#endif
            }
        }

        first_time = FALSE;
    }

    fs_current_r_gamma_lut_ptr = fs_default_r_gamma_lut_ptr;
    fs_current_g_gamma_lut_ptr = fs_default_g_gamma_lut_ptr;
    fs_current_b_gamma_lut_ptr = fs_default_b_gamma_lut_ptr;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_specified_gamma_inversion(const Vector* gamma_vp)
{
    int    i;
    double r_gamma;
    double g_gamma;
    double b_gamma;


    /* Untested since going to Luts for everything. */
    UNTESTED_CODE();

    if (gamma_vp->length != 3)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    r_gamma = gamma_vp->elements[ 0 ];
    g_gamma = gamma_vp->elements[ 1 ];
    b_gamma = gamma_vp->elements[ 2 ];

    verbose_pso(3, "Linearizing assuming a gamma correction (%5.2f, %5.2f, %5.2f).\n",
                r_gamma, g_gamma, b_gamma);

    ERE(get_target_lut(&fs_specified_r_linear_lut_ptr, LUT_SIZE, 0.0,
                       1.0 / FLT_LUT_PRECISION));

    ERE(get_target_lut(&fs_specified_g_linear_lut_ptr, LUT_SIZE, 0.0,
                       1.0 / FLT_LUT_PRECISION));

    ERE(get_target_lut(&fs_specified_b_linear_lut_ptr, LUT_SIZE, 0.0,
                       1.0 / FLT_LUT_PRECISION));

    fs_specified_r_linear_lut_ptr->lut_vp->elements[ 0 ] = 0.0;
    fs_specified_g_linear_lut_ptr->lut_vp->elements[ 0 ] = 0.0;
    fs_specified_b_linear_lut_ptr->lut_vp->elements[ 0 ] = 0.0;

    for (i=1; i<LUT_SIZE; i++)
    {
        fs_specified_r_linear_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, r_gamma);
        fs_specified_g_linear_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, g_gamma);
        fs_specified_b_linear_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, b_gamma);
    }

    fs_current_r_linear_lut_ptr = fs_specified_r_linear_lut_ptr;
    fs_current_g_linear_lut_ptr = fs_specified_g_linear_lut_ptr;
    fs_current_b_linear_lut_ptr = fs_specified_b_linear_lut_ptr;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_default_gamma_inversion(void)
{
    static int first_time = TRUE;
    int        i;
    FILE*      r_lut_fp;
    FILE*      g_lut_fp;
    FILE*      b_lut_fp;
#ifndef USE_PCD_GAMMA
    double     inverted_gamma = 1.0 / DEFAULT_GAMMA;
    double     gamma = DEFAULT_GAMMA;
#endif
    int        good_data      = TRUE;


    if (first_time)
    {
#ifdef TRACK_MEMORY_ALLOCATION
        prepare_memory_cleanup();
#endif

        r_lut_fp = open_gamma_config_file("gamma_inversion_red.lut");
        g_lut_fp = open_gamma_config_file("gamma_inversion_green.lut");
        b_lut_fp = open_gamma_config_file("gamma_inversion_blue.lut");

        if (    (r_lut_fp != NULL)
             && (g_lut_fp != NULL)
             && (b_lut_fp != NULL)
           )
        {
            /*
            // Untested since going to luts for help with new PCD / shaping
            // approach.
            */
            UNTESTED_CODE();

            verbose_pso(2, "Red gamma inversion LUT is %F\n", r_lut_fp);
            verbose_pso(2, "Green gamma inversion LUT is %F\n", g_lut_fp);
            verbose_pso(2, "Blue gamma inversion LUT is %F\n", b_lut_fp);

            SKIP_HEAP_CHECK_2();

            if (    (fp_read_lut(&fs_default_r_linear_lut_ptr, r_lut_fp) == ERROR)
                 || (fp_read_lut(&fs_default_g_linear_lut_ptr, g_lut_fp) == ERROR)
                 || (fp_read_lut(&fs_default_b_linear_lut_ptr, b_lut_fp) == ERROR)
               )
            {
                good_data = FALSE;
                kjb_print_error();
            }

            CONTINUE_HEAP_CHECK_2();

            kjb_fclose(r_lut_fp);
            kjb_fclose(g_lut_fp);
            kjb_fclose(b_lut_fp);
        }
        else
        {
            verbose_pso(2, "Gamma correction lookup tables for at least ");
            verbose_pso(2, "one channel is not available.\n");

            good_data = FALSE;
        }

        if ( ! good_data)
        {
#ifdef USE_PCD_GAMMA
            verbose_pso(2, "Using pcd linearization.\n");
#else
            verbose_pso(2, "Using standard linearization with ");
            verbose_pso(2, "gamma %3.1f.\n", DEFAULT_GAMMA);
#endif

            SKIP_HEAP_CHECK_2();

            ERE(get_target_lut(&fs_default_r_linear_lut_ptr, LUT_SIZE, 0.0,
                               1.0 / FLT_LUT_PRECISION));

            ERE(get_target_lut(&fs_default_g_linear_lut_ptr, LUT_SIZE, 0.0,
                               1.0 / FLT_LUT_PRECISION));

            ERE(get_target_lut(&fs_default_b_linear_lut_ptr, LUT_SIZE, 0.0,
                               1.0 / FLT_LUT_PRECISION));

            CONTINUE_HEAP_CHECK_2();

            fs_default_r_linear_lut_ptr->lut_vp->elements[ 0 ] = 0.0;
            fs_default_g_linear_lut_ptr->lut_vp->elements[ 0 ] = 0.0;
            fs_default_b_linear_lut_ptr->lut_vp->elements[ 0 ] = 0.0;

            for (i=1; i<LUT_SIZE; i++)
            {

#ifdef USE_PCD_GAMMA
                /*
                // Perhaps most useful, as the previous behavior is still
                // available with g=2.2,
                */

#define APPLY_PCD_LINEARIZATION(x) \
                ((x < 0.081) ? x / 4.5 : pow((x + 0.099)/1.099, 20.0/9.0))

                fs_default_r_linear_lut_ptr->lut_vp->elements[ i ] = APPLY_PCD_LINEARIZATION(((double)i) / FLT_LUT_PRECISION);
                fs_default_g_linear_lut_ptr->lut_vp->elements[ i ] = APPLY_PCD_LINEARIZATION(((double)i) / FLT_LUT_PRECISION);
                fs_default_b_linear_lut_ptr->lut_vp->elements[ i ] = APPLY_PCD_LINEARIZATION(((double)i) / FLT_LUT_PRECISION);

#else
                fs_default_r_linear_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, gamma);
                fs_default_g_linear_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, gamma);
                fs_default_b_linear_lut_ptr->lut_vp->elements[ i ] = pow(((double)i) / FLT_LUT_PRECISION, gamma);
#endif
            }
        }

        first_time = FALSE;
    }

    fs_current_r_linear_lut_ptr = fs_default_r_linear_lut_ptr;
    fs_current_g_linear_lut_ptr = fs_default_g_linear_lut_ptr;
    fs_current_b_linear_lut_ptr = fs_default_b_linear_lut_ptr;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static FILE* open_gamma_config_file(const char* file_name)
{
    char  display_env_var[ 1000 ];
    char* display_env_var_pos;
    char  full_host_name[ 1000 ];
    char* full_host_name_pos;
    char  host_name[ 1000 ];
    char  gamma_data_dir[ MAX_FILE_NAME_SIZE ];


    host_name[ 0 ]      = '\0';
    full_host_name[ 0 ] = '\0';

    if (BUFF_GET_ENV("DISPLAY", display_env_var) != ERROR)
    {
        display_env_var_pos = display_env_var;

        if (*display_env_var_pos != ':')
        {
            BUFF_GEN_GET_TOKEN(&display_env_var_pos, full_host_name, ":");
            full_host_name_pos = full_host_name;
            BUFF_GEN_GET_TOKEN(&full_host_name_pos, host_name, ".");
        }
    }

    if (host_name[ 0 ] == 0)
    {
        BUFF_GET_HOST_NAME(host_name);
    }

    BUFF_CPY(gamma_data_dir, DATA_DIR);
    BUFF_CAT(gamma_data_dir, DIR_STR);
    BUFF_CAT(gamma_data_dir, GAMMA_DIR);
    BUFF_CAT(gamma_data_dir, DIR_STR);
    BUFF_CAT(gamma_data_dir, host_name);

    return open_config_file("GAMMA_LUT_FILE", gamma_data_dir, file_name,
                            "gamma LUT");
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int ow_linearize_pcd(KJB_image* ip)
{
    int i, j;
    Pixel* pos;
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;

    /*
    // Linearizing PCD is essentially reversing a gamma of 2.2. We are
    // normally applying this linearization to 8 bit data (from
    // Imagemagkick decode, which is based on hpcdtoppm), which implies
    // a slight loss of information going from YCC, but this should not
    // matter much as those equations are linear. On the other hand,
    // this linearization should be done into a floating point format,
    // as the gamma'd data does contain information which can be lost
    // doing the reverse gamma.
    */
#define LINEARIZE_PCD(x) \
    (PCD_LINEAR_WHITE_RGB * \
        ((x < PCD_GAMMA_WHITE_RGB * 0.081) ? (x / PCD_GAMMA_WHITE_RGB) / 4.5  \
                     : pow((x/PCD_GAMMA_WHITE_RGB + 0.099)/1.099, 20.0/9.0)))

    verbose_pso(3, "Linearizing PCD.\n");

    for (i=0; i<num_rows; i++)
    {
        pos = ip->pixels[ i ];

        for (j=0; j<num_cols; j++)
        {
            pos->r = LINEARIZE_PCD(pos->r);
            pos->g = LINEARIZE_PCD(pos->g);
            pos->b = LINEARIZE_PCD(pos->b);

            pos++;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define PCD_METHOD_ONE_LUT_TOP   347

static int pcd_method_one_lut_initialized = FALSE;

static double pcd_method_one_lut_x[ PCD_METHOD_ONE_LUT_TOP + 1 ];
static double pcd_method_one_lut_y[ PCD_METHOD_ONE_LUT_TOP + 1 ];

/* Photo CD information beyond 100% white, Gamma 2.2 */

static double pcd_method_one_table_x[ 21 ] =
{
    0, 11, 22, 46, 72, 91, 107, 134, 156, 175, 192, 207, 221, 235, 247, 255,
    271, 292, 311, 330, 347
};

static double pcd_method_one_table_y[ 21 ] =
{
    0, 13, 23, 47, 71, 88, 102, 126, 145, 161, 176, 188, 201, 213, 223, 229,
    240, 249, 253, 254, 255
};

/*
// Method one above is similar to the older ImageMagick behaviour (e.g.
// version 3.7), which used the following LUT.
//
//   static Quantum
        PCDMap[348] =
        {
            0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  11,  12,  13,  14,
           15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,
           29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,
           43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  55,
           56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  66,  67,  68,
           69,  70,  71,  72,  73,  74,  75,  76,  76,  77,  78,  79,  80,  81,
           82,  83,  84,  84,  85,  86,  87,  88,  89,  90,  91,  92,  92,  93,
           94,  95,  96,  97,  98,  99,  99, 100, 101, 102, 103, 104, 105, 106,
          106, 107, 108, 109, 110, 111, 112, 113, 114, 114, 115, 116, 117, 118,
          119, 120, 121, 122, 122, 123, 124, 125, 126, 127, 128, 129, 129, 130,
          131, 132, 133, 134, 135, 136, 136, 137, 138, 139, 140, 141, 142, 142,
          143, 144, 145, 146, 147, 148, 148, 149, 150, 151, 152, 153, 153, 154,
          155, 156, 157, 158, 158, 159, 160, 161, 162, 163, 164, 165, 165, 166,
          167, 168, 169, 170, 171, 172, 173, 173, 174, 175, 176, 177, 178, 178,
          179, 180, 181, 182, 182, 183, 184, 185, 186, 186, 187, 188, 189, 190,
          191, 192, 193, 194, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203,
          204, 205, 205, 206, 207, 208, 209, 210, 210, 211, 212, 213, 214, 215,
          216, 216, 217, 218, 219, 220, 221, 221, 222, 223, 224, 225, 225, 226,
          227, 228, 228, 229, 230, 230, 231, 232, 233, 233, 234, 235, 235, 236,
          237, 237, 238, 239, 239, 240, 241, 241, 242, 242, 243, 243, 244, 244,
          245, 245, 245, 246, 246, 247, 247, 247, 248, 248, 248, 249, 249, 249,
          250, 250, 250, 250, 251, 251, 251, 251, 252, 252, 252, 252, 252, 252,
          253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 254, 254, 254, 254,
          254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
          254, 254, 254, 254, 254, 254, 255, 255, 255, 255, 255, 255
        };
*/


#define PCD_METHOD_TWO_LUT_TOP 350

static double pcd_method_two_lut_y[ PCD_METHOD_TWO_LUT_TOP + 1 ] =
{
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
      19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 32, 33, 34, 35,
      36, 37, 38, 39, 40, 41, 42, 43, 45, 46, 47, 48, 49, 50, 51, 52,
      53, 54, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68, 69, 70,
      71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 82, 83, 84, 86, 87, 88,
      89, 90, 91, 92, 93, 94, 95, 97, 98, 99, 100, 101, 102, 103, 104,
      105, 106, 107, 108, 110, 111, 112, 113, 114, 115, 116, 117, 118,
      119, 120, 121, 122, 123, 124, 125, 126, 127, 129, 130, 131, 132,
      133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145,
      146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158,
      159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171,
      172, 173, 174, 175, 176, 176, 177, 178, 179, 180, 181, 182, 183,
      184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 193, 194, 195,
      196, 197, 198, 199, 200, 201, 201, 202, 203, 204, 205, 206, 207,
      207, 208, 209, 210, 211, 211, 212, 213, 214, 215, 215, 216, 217,
      218, 218, 219, 220, 221, 221, 222, 223, 224, 224, 225, 226, 226,
      227, 228, 228, 229, 230, 230, 231, 232, 232, 233, 234, 234, 235,
      236, 236, 237, 237, 238, 238, 239, 240, 240, 241, 241, 242, 242,
      243, 243, 244, 244, 245, 245, 245, 246, 246, 247, 247, 247, 248,
      248, 248, 249, 249, 249, 249, 250, 250, 250, 250, 251, 251, 251,
      251, 251, 252, 252, 252, 252, 252, 253, 253, 253, 253, 253, 253,
      253, 253, 253, 253, 253, 253, 253, 254, 254, 254, 254, 254, 254,
      254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255
};

int ow_apply_pcd_output_lut(KJB_image* ip)
{
    int           i, j, num_rows, num_cols;
    int           result      = NO_ERROR;
    const double* pcd_lut_y;
    int           pcd_lut_top;
    float         flt_pcd_lut_top;
    int invalid_count = 0;


    if (    (fs_pcd_ycc_to_rgb_shape_function < 2)
         && ( ! pcd_method_one_lut_initialized)
       )
    {
        for (i = 0; i <= PCD_METHOD_ONE_LUT_TOP; i++)
        {
            pcd_method_one_lut_x[ i ] = i;
        }

        if (fs_pcd_ycc_to_rgb_shape_function == 0)
        {
            warn_pso("Using BUGGY shape table, presumbably for repreducing BAD results.\n");
            pcd_method_one_table_x[ 18 ] = 211;
        }

        ERE(cubic_spline(21, pcd_method_one_table_x, pcd_method_one_table_y,
                         1 + PCD_METHOD_ONE_LUT_TOP,
                         pcd_method_one_lut_x, pcd_method_one_lut_y));
        pcd_method_one_lut_initialized = TRUE;
    }

    if (fs_pcd_ycc_to_rgb_shape_function < 2)
    {
        pcd_lut_y = pcd_method_one_lut_y;
        pcd_lut_top = PCD_METHOD_ONE_LUT_TOP;
        flt_pcd_lut_top = (float)PCD_METHOD_ONE_LUT_TOP;
    }
    else
    {
        pcd_lut_y = pcd_method_two_lut_y;
        pcd_lut_top = PCD_METHOD_TWO_LUT_TOP;
        flt_pcd_lut_top = (float)PCD_METHOD_TWO_LUT_TOP;
    }

/*
#define PLOT_PCD_LUT
*/

#ifdef PLOT_PCD_LUT
    {
        Vector* shape_vp = NULL;
        int plot_id;

        dbf(pcd_lut_y[ 100 ]);
        dbf(pcd_lut_y[ 200 ]);
        dbf(pcd_lut_y[ 255 ]);

        ERE(get_target_vector(&shape_vp, pcd_lut_top));

        for (i = 0; i < pcd_lut_top; i++)
        {
            shape_vp->elements[ i ] = pcd_lut_y[ i ];
        }

        ERE(plot_id = plot_open());
        ERE(plot_vector(plot_id, shape_vp, 0.0, 1.0, "shape"));
        free_vector(shape_vp);
        prompt_to_continue();
        ERE(plot_close(plot_id));
    }
#endif

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    for (i = 0; i < num_rows; i++)
    {
        Pixel* pixel_pos = ip->pixels[ i ];

        for (j = 0; j < num_cols; j++)
        {
            float r = pixel_pos->r;
            float g = pixel_pos->g;
            float b = pixel_pos->b;
            int invalid = FALSE;

            if (r < 0.0)
            {
                pixel_pos->extra.invalid.r |= DARK_PIXEL;
                r = 0.0;
                invalid = TRUE;
            }
            else if (r > flt_pcd_lut_top)
            {
                pixel_pos->extra.invalid.r |= CLIPPED_PIXEL;
                r = 255.0;
                invalid = TRUE;
            }
            else
            {
                int r1 = (int)r;
                int r2 = MIN_OF(r1 + 1, pcd_lut_top);
                float dr = r - (float)r1;

                ASSERT((dr >= 0.0) && (dr <= 1.0));
                ASSERT(r1 <= pcd_lut_top);
                ASSERT(r2 <= pcd_lut_top);

                r = (FLT_ONE - dr) * pcd_lut_y[ r1 ] + dr * pcd_lut_y[ r2 ];
            }

            pixel_pos->r = r;


            if (g < 0.0)
            {
                pixel_pos->extra.invalid.g |= DARK_PIXEL;
                g = 0.0;
                invalid = TRUE;
            }
            else if (g > flt_pcd_lut_top)
            {
                pixel_pos->extra.invalid.g |= CLIPPED_PIXEL;
                g = 255.0;
                invalid = TRUE;
            }
            else
            {
                int g1 = (int)g;
                int g2 = MIN_OF(g1 + 1, pcd_lut_top);
                float dg = g - (float)g1;

                ASSERT((dg >= 0.0) && (dg <= 1.0));
                ASSERT(g1 <= pcd_lut_top);
                ASSERT(g2 <= pcd_lut_top);

                g = (FLT_ONE - dg) * pcd_lut_y[ g1 ] + dg * pcd_lut_y[ g2 ];
            }

            pixel_pos->g = g;


            if (b < 0.0)
            {
                pixel_pos->extra.invalid.b |= DARK_PIXEL;
                b = 0.0;
                invalid = TRUE;
            }
            else if (b > flt_pcd_lut_top)
            {
                pixel_pos->extra.invalid.b |= CLIPPED_PIXEL;
                b = 255.0;
                invalid = TRUE;
            }
            else
            {
                int b1 = (int)b;
                int b2 = MIN_OF(b1 + 1, pcd_lut_top);
                float db = b - (float)b1;

                ASSERT((db >= 0.0) && (db <= 1.0));
                ASSERT(b1 <= pcd_lut_top);
                ASSERT(b2 <= pcd_lut_top);

                b = (FLT_ONE - db) * pcd_lut_y[ b1 ] + db * pcd_lut_y[ b2 ];
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
           "%d image pixels marked invalid due to rgb's outside range\n",
            invalid_count);
    verbose_pso(3, "while converting from PCD YCC to RGB with gamma.\n");

    return result;
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
    free_lut(fs_default_r_gamma_lut_ptr);
    free_lut(fs_default_g_gamma_lut_ptr);
    free_lut(fs_default_b_gamma_lut_ptr);
    free_lut(fs_specified_r_gamma_lut_ptr);
    free_lut(fs_specified_g_gamma_lut_ptr);
    free_lut(fs_specified_b_gamma_lut_ptr);

    free_lut(fs_default_r_linear_lut_ptr);
    free_lut(fs_default_g_linear_lut_ptr);
    free_lut(fs_default_b_linear_lut_ptr);
    free_lut(fs_specified_r_linear_lut_ptr);
    free_lut(fs_specified_g_linear_lut_ptr);
    free_lut(fs_specified_b_linear_lut_ptr);
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

