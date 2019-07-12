
/* $Id: i_offset.c 4727 2009-11-16 20:53:54Z kobus $ */

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

#include "i/i_arithmetic.h"
#include "i/i_float_io.h"
#include "i/i_offset.h"
#include "i/i_set_aux.h"
#include "i/i_stat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef DATA_DIR
#    define DATA_DIR               "data"
#endif

#ifndef CAMERA_DIR
#    define CAMERA_DIR             "camera"
#endif

#ifndef OFFSET_VECTOR_FILE
#    define OFFSET_VECTOR_FILE     "offset_vector.rgb"
#endif

#ifndef OFFSET_IMAGE_FILE
#    define OFFSET_IMAGE_FILE      "offset_image.kiff"
#endif

#ifndef SPATIAL_CHROM_FILE
#    define SPATIAL_CHROM_FILE      "spatial_chrom_correction.kiff"
#endif

#ifndef RED_LINEARIZATION_FILE
#    define RED_LINEARIZATION_FILE "red_linearization.lut"
#endif

#ifndef GREEN_LINEARIZATION_FILE
#    define GREEN_LINEARIZATION_FILE "green_linearization.lut"
#endif

#ifndef BLUE_LINEARIZATION_FILE
#    define BLUE_LINEARIZATION_FILE "blue_linearization.lut"
#endif

#define DEFAULT_RED_OFFSET                 7.5681
#define DEFAULT_GREEN_OFFSET               10.1518
#define DEFAULT_BLUE_OFFSET                8.7881

/* -------------------------------------------------------------------------- */

static Vector*    fs_offset_vector_vp                               = NULL;
static int        fs_disable_offset_vector_removal                  = TRUE;
static KJB_image* fs_spatial_chrom_ip                               = NULL;
static int        fs_disable_spatial_chrom_removal                  = TRUE;
static char       fs_spatial_chrom_path[ MAX_FILE_NAME_SIZE ]       = { '\0' };
static KJB_image* fs_offset_image_ip                                = NULL;
static Vector*    fs_offset_image_ave_vp                            = NULL;
static int        fs_disable_offset_image_removal                   = TRUE;
static char       fs_offset_image_path[ MAX_FILE_NAME_SIZE ]        = { '\0' };
static int        fs_linearize_image                                = FALSE;
static char       fs_red_linearization_file[ MAX_FILE_NAME_SIZE ]   = {'\0'};
static char       fs_green_linearization_file[ MAX_FILE_NAME_SIZE ] = {'\0'};
static char       fs_blue_linearization_file[ MAX_FILE_NAME_SIZE ]  = {'\0'};
static Lut*       fs_red_linearization_lut_ptr                      = NULL;
static Lut*       fs_green_linearization_lut_ptr                    = NULL;
static Lut*       fs_blue_linearization_lut_ptr                     = NULL;

/* ------------------------------------------------------------------------- */

static int initialize_spatial_chrom(void);
static int set_spatial_chrom(FILE* fp);

static int initialize_offset_image(void);
static int set_offset_image(FILE* fp);
static int initialize_offset_vector(void);
static int set_offset_vector(FILE* fp);

static int set_linearization_file
(
    Lut**       lut_ptr_ptr,
    const char* linearization_file,
    const char* linearization_file_id,
    char*       resolved_linearization_file,
    size_t      resolved_linearization_file_size
);

static int initialize_linearize(void);
static int ow_linearize_image(KJB_image* ip);
static int ow_linearize_data(Matrix* mp);

#ifdef TRACK_MEMORY_ALLOCATION
    static void prepare_memory_cleanup(void);
    static void free_allocated_static_data(void);
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                           set_offset_removal_options
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int set_offset_removal_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "cvof"))
         || (match_pattern(lc_option, "camera-vector-offset-file"))
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(set_camera_vector_offset(value));
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "cscf"))
         || (match_pattern(lc_option, "camera-spatial-chrom-file"))
         || (match_pattern(lc_option, "camera-spatial-chromaticity-file"))
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(set_camera_spatial_chrom_file(value));
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "ciof"))
         || (match_pattern(lc_option, "camera-image-offset-file"))
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(set_camera_image_offset(value));
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "linearization"))
       )
    {
        int boolean_value;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (*value == '\0')
        {
            ERE(pso("Images %s linearized.\n",
                fs_linearize_image ? "are" : "are not"));
        }
        else if (*value == '?')
        {
            ERE(pso("linearization = %s\n", fs_linearize_image ? "t" : "f"));
        }
        else
        {
            ERE(boolean_value = get_boolean_value(value));
            fs_linearize_image = boolean_value;

            call_image_data_invalidation_fn();
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "rlf"))
         || (match_pattern(lc_option, "red-linearization-file"))
         || (match_pattern(lc_option, "red-linearization-lut"))
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(set_linearization_file(&fs_red_linearization_lut_ptr, value, "red",
                                   fs_red_linearization_file,
                                   sizeof(fs_red_linearization_file)));

        call_image_data_invalidation_fn();

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "glf"))
         || (match_pattern(lc_option, "green-linearization-file"))
         || (match_pattern(lc_option, "green-linearization-lut"))
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(set_linearization_file(&fs_green_linearization_lut_ptr, value, "green",
                                   fs_green_linearization_file,
                                   sizeof(fs_green_linearization_file)));

        call_image_data_invalidation_fn();

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "blf"))
         || (match_pattern(lc_option, "blue-linearization-file"))
         || (match_pattern(lc_option, "blue-linearization-lut"))
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(set_linearization_file(&fs_blue_linearization_lut_ptr, value, "blue",
                                   fs_blue_linearization_file,
                                   sizeof(fs_blue_linearization_file)));

        call_image_data_invalidation_fn();

        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             set_camera_vector_offset
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int set_camera_vector_offset(const char* file_name)
{
    FILE* offset_fp;
    int   result;
    int   boolean_value;


    if (*file_name == '\0')
    {
        if (fs_offset_vector_vp == NULL)
        {
            ERE(pso("No offset vector has been set yet.\n"));
        }
        else if (fs_disable_offset_vector_removal)
        {
            ERE(pso("Offset vector use is currently disabled.\n"));
        }
        else
        {
            ERE(pso("Using an offset vector of (%.2f, %.2f, %.2f).\n",
                    fs_offset_vector_vp->elements[ 0 ],
                    fs_offset_vector_vp->elements[ 1 ],
                    fs_offset_vector_vp->elements[ 2 ]));
        }
        return NO_ERROR;
    }
    else if (*file_name == '?')
    {
        if (fs_offset_vector_vp == NULL)
        {
            ERE(pso("cvof = <never set>\n"));
        }
        else if (fs_disable_offset_vector_removal)
        {
            ERE(pso("cvof = off\n"));
        }
        else
        {
            ERE(pso("cvof = <file>\n"));
        }
        return NO_ERROR;
    }

    call_image_data_invalidation_fn();

    boolean_value = get_boolean_value(file_name);

    if (boolean_value == FALSE)
    {
        fs_disable_offset_vector_removal = TRUE;
        return NO_ERROR;
    }
    else if (boolean_value == TRUE)
    {
        fs_disable_offset_vector_removal = FALSE;
        return NO_ERROR;
    }

    NRE(offset_fp = kjb_fopen(file_name, "r"));

    result = set_offset_vector(offset_fp);

    if (result == NO_ERROR)
    {
        fs_disable_offset_vector_removal = FALSE;
    }

    (void)kjb_fclose(offset_fp);  /* Ignore return--only reading. */

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             set_camera_spatial_chrom_file
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int set_camera_spatial_chrom_file(const char* file_name)
{
    FILE* offset_fp;
    int   result;
    int   boolean_value;


    if (*file_name == '\0')
    {
        if (fs_spatial_chrom_ip == NULL)
        {
            ERE(pso("No spatial chromaticity image has been read yet.\n"));
        }
        else if (fs_disable_spatial_chrom_removal)
        {
            ERE(pso("Spatial chromaticity correction is currently disabled.\n"));
        }
        else
        {
            ERE(pso("Using spatial chromaticity image read from file %s.\n",
                    fs_spatial_chrom_path));
        }

        return NO_ERROR;
    }
    else if (*file_name == '?')
    {
        if (fs_spatial_chrom_ip == NULL)
        {
            ERE(pso("cscf = <never set>\n"));
        }
        else if (fs_disable_spatial_chrom_removal)
        {
            ERE(pso("cscf = off\n"));
        }
        else
        {
            ERE(pso("cscf = %s\n", fs_spatial_chrom_path));
        }

        return NO_ERROR;
    }

    call_image_data_invalidation_fn();

    boolean_value = get_boolean_value(file_name);

    if (boolean_value == FALSE)
    {
        fs_disable_spatial_chrom_removal = TRUE;
        return NO_ERROR;
    }
    else if (boolean_value == TRUE)
    {
        fs_disable_spatial_chrom_removal = FALSE;
        return NO_ERROR;
    }

    NRE(offset_fp = kjb_fopen(file_name, "r"));

    result = set_spatial_chrom(offset_fp);

    (void)kjb_fclose(offset_fp);  /* Ignore return--only reading. */

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             set_camera_image_offset
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int set_camera_image_offset(const char* file_name)
{
    FILE* offset_fp;
    int   result;
    int   boolean_value;


    if (*file_name == '\0')
    {
        if (fs_offset_image_ip == NULL)
        {
            ERE(pso("No offset image has been read yet.\n"));
        }
        else if (fs_disable_offset_image_removal)
        {
            ERE(pso("Offset image use is currently disabled.\n"));
        }
        else
        {
            ERE(pso("Using offset image read from file %s.\n",
                    fs_offset_image_path));
        }

        return NO_ERROR;
    }
    else if (*file_name == '?')
    {
        if (fs_offset_image_ip == NULL)
        {
            ERE(pso("ciof = <never set>\n"));
        }
        else if (fs_disable_offset_image_removal)
        {
            ERE(pso("ciof = off\n"));
        }
        else
        {
            ERE(pso("ciof = %s\n", fs_offset_image_path));
        }

        return NO_ERROR;
    }

    call_image_data_invalidation_fn();

    boolean_value = get_boolean_value(file_name);

    if (boolean_value == FALSE)
    {
        fs_disable_offset_image_removal = TRUE;
        return NO_ERROR;
    }
    else if (boolean_value == TRUE)
    {
        fs_disable_offset_image_removal = FALSE;
        return NO_ERROR;
    }

    NRE(offset_fp = kjb_fopen(file_name, "r"));

    result = set_offset_image(offset_fp);

    (void)kjb_fclose(offset_fp);  /* Ignore return--only reading. */

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        remove_camera_offset_from_image
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int remove_camera_offset_from_image(KJB_image* ip)
{
    ERE(initialize_offset_image());

    if (fs_disable_offset_image_removal)
    {
        verbose_pso(5, "Offset image removal is disabled and thus NOT done.\n");
    }
    else if (fs_offset_image_ip == NULL)
    {
        verbose_pso(5,
                  "Offset image removal is skipped due to no offset image.\n");
    }
    else if (    (ip->num_rows != fs_offset_image_ip->num_rows)
              || (ip->num_cols != fs_offset_image_ip->num_cols)
            )
    {
        verbose_pso(5,
          "Offset image removal skipped due to incorrect offset image size.\n");
    }
    else
    {
        verbose_pso(5, "Offset image is removed using %s.\n",
                    fs_offset_image_path);

        ERE(ow_add_vector_to_image(ip, fs_offset_image_ave_vp));
        ERE(ow_subtract_images(ip, fs_offset_image_ip));
        ERE(ow_min_thresh_image(ip, 0.0));
        ERE(ow_max_thresh_image(ip, 255.0));
    }

    ERE(ow_linearize_image(ip));

    ERE(initialize_offset_vector());

    if (fs_disable_offset_vector_removal)
    {
        verbose_pso(5,
                    "Offset vector removal is disabled and thus NOT done.\n");
    }
    else if (fs_offset_vector_vp == NULL)
    {
        verbose_pso(5,
                "Offset vector removal is skipped due to no offset vector.\n");
    }
    else
    {
        verbose_pso(5, "Offset vector is removed using (%.2f, %.2f, %.2f).\n",
                    fs_offset_vector_vp->elements[ 0 ],
                    fs_offset_vector_vp->elements[ 1 ],
                    fs_offset_vector_vp->elements[ 2 ]);

        ERE(ow_subtract_vector_from_image(ip, fs_offset_vector_vp));
        ERE(ow_min_thresh_image(ip, 0.0));
    }

    ERE(initialize_spatial_chrom());

    if (fs_disable_spatial_chrom_removal)
    {
        verbose_pso(5, "Spatial chrom correction is disabled and thus NOT done.\n");
    }
    else if (fs_spatial_chrom_ip == NULL)
    {
        verbose_pso(5,
           "Spatial chrom correction is skipped due to no correction image.\n");
    }
    else if (    (ip->num_rows != fs_spatial_chrom_ip->num_rows)
              || (ip->num_cols != fs_spatial_chrom_ip->num_cols)
            )
    {
        verbose_pso(3,
          "Spatial chrom correction is skipped due to incorrect correction image size.\n");
    }
    else
    {
        verbose_pso(5, "Spatial chromaticity is corrected using %s.\n",
                    fs_spatial_chrom_path);

        ERE(ow_multiply_images(ip, fs_spatial_chrom_ip));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        remove_camera_offset_from_data
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int remove_camera_offset_from_data(Matrix* mp)
{
    ERE(ow_linearize_data(mp));

    ERE(initialize_offset_vector());

    if (fs_disable_offset_vector_removal)
    {
        verbose_pso(5,"Offset vector removal is disabled and thus NOT done.\n");
    }
    else if (fs_offset_vector_vp == NULL)
    {
        verbose_pso(5,
                "Offset vector removal is skipped due to no offset vector.\n");
    }
    else
    {
        ERE(ow_subtract_row_vector_from_matrix(mp, fs_offset_vector_vp));
        ERE(ow_min_thresh_matrix(mp, FLT_ZERO));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_spatial_chrom(void)
{
    FILE*       spatial_chrom_fp;
    int         result                                = NO_ERROR;
    const char* message_str                           = "spatial chromaticity image";
    char        camera_data_dir[ MAX_FILE_NAME_SIZE ];


    if ((fs_disable_spatial_chrom_removal) || (fs_spatial_chrom_ip != NULL))
    {
        return NO_ERROR;
    }

    BUFF_CPY(camera_data_dir, DATA_DIR);
    BUFF_CAT(camera_data_dir, DIR_STR);
    BUFF_CAT(camera_data_dir, CAMERA_DIR);

    spatial_chrom_fp = open_config_file((char*)NULL, camera_data_dir,
                                        SPATIAL_CHROM_FILE, message_str);

    if (spatial_chrom_fp != NULL)
    {
        if (set_spatial_chrom(spatial_chrom_fp) == ERROR)
        {
            add_error("Unable to use spatial chromaticity correction image %P.",
                      spatial_chrom_fp);
            add_error("Spatical chromaticity correction file not set.");
            result =  ERROR;
        }
        (void)kjb_fclose(spatial_chrom_fp);  /* Ignore return--only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_offset_image(void)
{
    FILE*       offset_image_fp;
    int         result                                = NO_ERROR;
    const char* message_str                           = "camera offset image";
    char        camera_data_dir[ MAX_FILE_NAME_SIZE ];


    if ((fs_disable_offset_image_removal) || (fs_offset_image_ip != NULL))
    {
        return NO_ERROR;
    }

    BUFF_CPY(camera_data_dir, DATA_DIR);
    BUFF_CAT(camera_data_dir, DIR_STR);
    BUFF_CAT(camera_data_dir, CAMERA_DIR);

    offset_image_fp = open_config_file((char*)NULL, camera_data_dir,
                                        OFFSET_IMAGE_FILE, message_str);

    if (offset_image_fp != NULL)
    {
        if (set_offset_image(offset_image_fp) == ERROR)
        {
            add_error("Unable to use offset image %P.", offset_image_fp);
            add_error("Offset image file not set.");
            result =  ERROR;
        }
        (void)kjb_fclose(offset_image_fp);  /* Ignore return--only reading. */
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_offset_vector(void)
{
    int result;
    FILE*       offset_vector_fp;
    const char* message_str      = "camera offset vector";
    char        camera_data_dir[ MAX_FILE_NAME_SIZE ];


    if ((fs_disable_offset_vector_removal) || (fs_offset_vector_vp != NULL))
    {
        return NO_ERROR;
    }

    BUFF_CPY(camera_data_dir, DATA_DIR);
    BUFF_CAT(camera_data_dir, DIR_STR);
    BUFF_CAT(camera_data_dir, CAMERA_DIR);

    offset_vector_fp = open_config_file((char*)NULL, camera_data_dir,
                                        OFFSET_VECTOR_FILE, message_str);

    if (offset_vector_fp != NULL)
    {
        verbose_pso(5, "Offset vector file is %F.\n", offset_vector_fp);
    }

    result = set_offset_vector(offset_vector_fp);

    (void)kjb_fclose(offset_vector_fp);  /* Ignore return--only reading. */

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_linearize(void)
{
    static int first_time = TRUE;


    if (    ( ! fs_linearize_image)
         || ( ! first_time )
         || (fs_red_linearization_lut_ptr != NULL)
         || (fs_green_linearization_lut_ptr != NULL)
         || (fs_blue_linearization_lut_ptr != NULL)
       )
    {
        return NO_ERROR;
    }

    first_time = FALSE;

    ERE(set_linearization_file(&fs_red_linearization_lut_ptr,
                               RED_LINEARIZATION_FILE, "red",
                               fs_red_linearization_file,
                               sizeof(fs_red_linearization_file)));

    ERE(set_linearization_file(&fs_green_linearization_lut_ptr,
                               GREEN_LINEARIZATION_FILE, "green",
                               fs_green_linearization_file,
                               sizeof(fs_green_linearization_file)));

    ERE(set_linearization_file(&fs_blue_linearization_lut_ptr,
                               BLUE_LINEARIZATION_FILE, "blue",
                               fs_blue_linearization_file,
                               sizeof(fs_blue_linearization_file)));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_spatial_chrom(FILE* fp)
{
    int        result;
    KJB_image* temp_ip = NULL;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    /*
    // Can't be kjb_read_image(), since we are called from kjb_read_image()!
    */
    result = read_image_from_kiff(&temp_ip, fp, (int*)NULL);

    if (result != ERROR)
    {
        result = kjb_copy_image(&fs_spatial_chrom_ip, temp_ip);
    }

    if (result == NO_ERROR)
    {
        BUFF_GET_USER_FD_NAME(fileno(fp), fs_spatial_chrom_path);
        fs_disable_spatial_chrom_removal = FALSE;
    }

    kjb_free_image(temp_ip);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_offset_image(FILE* fp)
{
    int        result;
    KJB_image* temp_ip = NULL;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    /*
    // Can't be kjb_read_image(), since we are called from kjb_read_image()!
    */
    result = read_image_from_kiff(&temp_ip, fp, (int*)NULL);

    if (result != ERROR)
    {
        result = kjb_copy_image(&fs_offset_image_ip, temp_ip);
    }

    if (result != ERROR)
    {
        result = get_image_stats((int*)NULL, &fs_offset_image_ave_vp,
                                 (Vector**)NULL, fs_offset_image_ip);
    }

    if (result == NO_ERROR)
    {
        BUFF_GET_USER_FD_NAME(fileno(fp), fs_offset_image_path);
        fs_disable_offset_image_removal = FALSE;
    }

    kjb_free_image(temp_ip);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_offset_vector(FILE* offset_vector_fp)
{
    int result = NO_ERROR;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (offset_vector_fp != NULL)
    {
        Vector* temp_offset_vp;


        NRE(temp_offset_vp = create_vector(3));

        if (fp_ow_read_vector(temp_offset_vp, offset_vector_fp) == ERROR)
        {
            result = ERROR;
        }
        else
        {
            result = copy_vector(&fs_offset_vector_vp, temp_offset_vp);
        }

        if (result == ERROR)
        {
            add_error("Unable to use offset file %P.", offset_vector_fp);
            add_error("Offset not changed.");
        }

        free_vector(temp_offset_vp);
    }
    else
    {
        ERE(get_target_vector(&fs_offset_vector_vp, 3));

        fs_offset_vector_vp->elements[ 0 ] = DEFAULT_RED_OFFSET;
        fs_offset_vector_vp->elements[ 1 ] = DEFAULT_GREEN_OFFSET;
        fs_offset_vector_vp->elements[ 2 ] = DEFAULT_BLUE_OFFSET;
    }

    if (result == ERROR) return ERROR;

    verbose_pso(5, "Using a camera offset vector of (%4.1f, %4.1f, %4.1f).\n",
                fs_offset_vector_vp->elements[ 0 ],
                fs_offset_vector_vp->elements[ 1 ],
                fs_offset_vector_vp->elements[ 2 ]);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_linearization_file
(
    Lut**       lut_ptr_ptr,
    const char* linearization_file,
    const char* linearization_channel,
    char*       resolved_linearization_file,
    size_t      resolved_linearization_file_size
)
{
    char linearization_file_id[ 100 ];


    BUFF_CPY(linearization_file_id, linearization_channel);
    BUFF_CAT(linearization_file_id, " linearization");

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (*linearization_file == '\0')
    {
        if (*lut_ptr_ptr == NULL)
        {
            ERE(pso("No %s is set.\n", linearization_file_id));
        }
        else
        {
            char temp_linearization_file_id[ 100 ];

            BUFF_CAP_FIRST_LETTER_CPY(temp_linearization_file_id,
                                      linearization_file_id);
            ERE(pso("%s is %s.\n", temp_linearization_file_id,
                resolved_linearization_file));
        }
    }
    else if (*linearization_file == '?')
    {
        ERE(pso("%s-linearizaion-file = %s\n", linearization_channel,
                ((*lut_ptr_ptr) != NULL) ? resolved_linearization_file : "off"));
    }
    else
    {
        char camera_data_dir[ MAX_FILE_NAME_SIZE ];

        BUFF_CPY(camera_data_dir, DATA_DIR);
        BUFF_CAT(camera_data_dir, DIR_STR);
        BUFF_CAT(camera_data_dir, CAMERA_DIR);

        ERE(read_lut_from_config_file(lut_ptr_ptr, (char*)NULL,
                                      camera_data_dir, linearization_file,
                                      linearization_file_id,
                                      resolved_linearization_file,
                                      resolved_linearization_file_size));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int ow_linearize_image(KJB_image* ip)
{
    int    i, j, num_rows, num_cols;
    Pixel* pos;
    double r, g, b;
    double linearized_r;
    double linearized_g;
    double linearized_b;


    if ( ! fs_linearize_image)
    {
        verbose_pso(5, "Linearization is disabled and thus NOT done.\n");
        return NO_ERROR;
    }

    ERE(initialize_linearize());

    num_rows = ip->num_rows;
    num_cols = ip->num_cols;

    if (fs_red_linearization_lut_ptr != NULL)
    {
        verbose_pso(5, "Red channel is being linearized using LUT in %s.\n",
                    fs_red_linearization_file);

        for (i=0; i<num_rows; i++)
        {
            pos = ip->pixels[ i ];

            for (j=0; j<num_cols; j++)
            {
                r = pos->r;
                ERE(apply_lut(fs_red_linearization_lut_ptr, r, &linearized_r));
                pos->r = linearized_r;
                pos++;
            }
        }
    }
    else
    {
        verbose_pso(5, "Red linearization is skipped due to no LUT.\n");
    }

    if (fs_green_linearization_lut_ptr != NULL)
    {
        verbose_pso(5, "Green channel is being linearized using LUT in %s.\n",
                    fs_green_linearization_file);

        for (i=0; i<num_rows; i++)
        {
            pos = ip->pixels[ i ];

            for (j=0; j<num_cols; j++)
            {
                g = pos->g;
                ERE(apply_lut(fs_green_linearization_lut_ptr, g, &linearized_g));
                pos->g = linearized_g;
                pos++;
            }
        }
    }
    else
    {
        verbose_pso(5, "Green linearization is skipped due to no LUT.\n");
    }

    if (fs_blue_linearization_lut_ptr != NULL)
    {
        verbose_pso(5, "Blue channel is being linearized using LUT in %s.\n",
                    fs_blue_linearization_file);

        for (i=0; i<num_rows; i++)
        {
            pos = ip->pixels[ i ];

            for (j=0; j<num_cols; j++)
            {
                b = pos->b;
                ERE(apply_lut(fs_blue_linearization_lut_ptr, b, &linearized_b));
                pos->b = linearized_b;
                pos++;
            }
        }
    }
    else
    {
        verbose_pso(5, "Blue linearization is skipped due to no LUT.\n");
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int ow_linearize_data(Matrix* mp)
{
    int    i, num_rows;
    double r, g, b;


    if (! fs_linearize_image)
    {
        verbose_pso(5, "Linearization is disabled and thus NOT done.\n");
        return NO_ERROR;
    }

    ERE(initialize_linearize());

    num_rows = mp->num_rows;

    if (fs_red_linearization_lut_ptr != NULL)
    {
        verbose_pso(5, "Red channel is being linearized using LUT in %s.\n",
                    fs_red_linearization_file);

        for (i=0; i<num_rows; i++)
        {
            r = mp->elements[ i ][ 0 ];
            ERE(apply_lut(fs_red_linearization_lut_ptr, r,
                          &(mp->elements[ i ][ 0 ])));
        }
    }
    else
    {
        verbose_pso(5, "Red linearization is skipped due to no LUT.\n");
    }

    if (fs_green_linearization_lut_ptr != NULL)
    {
        verbose_pso(5, "Green channel is being linearized using LUT in %s.\n",
                    fs_green_linearization_file);

        for (i=0; i<num_rows; i++)
        {
            g = mp->elements[ i ][ 1 ];
            ERE(apply_lut(fs_green_linearization_lut_ptr, g,
                          &(mp->elements[ i ][ 1 ])));
        }
    }
    else
    {
        verbose_pso(5, "Green linearization is skipped due to no LUT.\n");
    }

    if (fs_blue_linearization_lut_ptr != NULL)
    {
        verbose_pso(5, "Blue channel is being linearized using LUT in %s.\n",
                    fs_blue_linearization_file);

        for (i=0; i<num_rows; i++)
        {
            b = mp->elements[ i ][ 2 ];
            ERE(apply_lut(fs_blue_linearization_lut_ptr, b,
                          &(mp->elements[ i ][ 2 ])));
        }
    }
    else
    {
        verbose_pso(5, "Blue linearization is skipped due to no LUT.\n");
    }

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
    free_vector(fs_offset_vector_vp);

    kjb_free_image(fs_spatial_chrom_ip);
    kjb_free_image(fs_offset_image_ip);
    free_vector(fs_offset_image_ave_vp);

    free_lut(fs_red_linearization_lut_ptr);
    free_lut(fs_green_linearization_lut_ptr);
    free_lut(fs_blue_linearization_lut_ptr);
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

