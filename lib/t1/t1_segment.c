
/* $Id: t1_segment.c 7987 2010-12-15 09:32:33Z kobus $ */

 
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

#include "t1/t1_gen.h"     /* Only safe as first include in a ".c" file. */
#include "t1/t1_segment.h"

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define USE_NO_VALUE
#define CONNECTION_MAX_STEP_OPTION
#define IMPLEMENT_MERGING
#define CONNECT_CORNER_OPTION

/* #define DEFAULT_IS_TO_CONNECT_CORNERS */  /* 8 connected by default, ow 4 */

#define RGB_SUM_EPSILON 0.0001
#define CHROM_EPSILON   0.000001

/* -------------------------------------------------------------------------- */

typedef enum Boundary_approach
{
    SEG_DOWN,
    SEG_UP,
    SEG_LEFT,
    SEG_RIGHT
}
Boundary_approach;

/* -------------------------------------------------------------------------- */

#ifdef TEST
static int seg_verify_segmentation = FALSE;
#endif

static int seg_fill_hole_level       = 1;

#ifdef CONNECTION_MAX_STEP_OPTION
static int connection_max_step       = 2;
#endif
static int seg_resegment_level       = 1;
#ifdef CONNECT_CORNER_OPTION
#ifdef DEFAULT_IS_TO_CONNECT_CORNERS
static int seg_connect_corners       = TRUE;
#else
static int seg_connect_corners       = FALSE;
#endif
#endif
static int seg_min_segment_size      = 100;
static int seg_min_resegment_size    = 20;

/*
// We set the variance of each channel, but for differences we use the sum of
// squares.
//
// Note that most defaults are very large, effectively making it so that that
// particular threshold is not used.
*/
#ifdef USE_NO_VALUE
static float max_rel_RGB_var = FLT_NOT_SET;
static float max_abs_RGB_var = FLT_NOT_SET;

static float max_rel_sum_RGB_var = FLT_NOT_SET;
static float max_abs_sum_RGB_var = FLT_NOT_SET;

static float max_rel_chrom_var = FLT_NOT_SET;
static float max_abs_chrom_var = 0.03;

static float max_rel_RGB_sqr_diff = FLT_NOT_SET;
static float max_abs_RGB_sqr_diff = FLT_NOT_SET;

static float max_rel_sum_RGB_diff = 0.08;
static float max_abs_sum_RGB_diff = 15;

static float max_rel_chrom_diff = FLT_NOT_SET;
static float max_abs_chrom_diff = FLT_NOT_SET;
#else
static float max_rel_RGB_var = 100.0;
static float max_abs_RGB_var = 10000;

static float max_rel_sum_RGB_var = 500.0;
static float max_abs_sum_RGB_var = 50000;

static float max_rel_chrom_var = 100.0;
static float max_abs_chrom_var = 0.03;

static float max_rel_RGB_sqr_diff = 10000;
static float max_abs_RGB_sqr_diff = 1000000;

static float max_rel_sum_RGB_diff = 0.08;
static float max_abs_sum_RGB_diff = 15;

static float max_rel_chrom_diff = 100.0;
static float max_abs_chrom_diff = 20.0;
#endif

static float min_sum_RGB_for_chrom_test = 50.0;
static float min_sum_RGB_for_relative_sum_diff = 50.0;

#ifdef IMPLEMENT_MERGING
static int seg_merge_level           = 0;
static int merge_min_num_pixels  = 20;
static double merge_rg_threshold  = 0.05;
static double merge_sum_RGB_abs_threshold = 30;
static double merge_sum_RGB_rel_threshold = 30;
static double merge_RGB_noise_estimate = 3.0;
#endif

/* -------------------------------------------------------------------------- */

static int cached_segment_count;
static int num_cached_pixels;
static Pixel_info* cached_pixels = NULL;
static Int_matrix*  cached_magnified_seg_map = NULL;
static int cached_i_min;
static int cached_i_max;
static int cached_j_min;
static int cached_j_max;
static double cached_i_sum;
static double cached_j_sum;
static double cached_R_sum;
static double cached_G_sum;
static double cached_B_sum;
static int          cached_num_rows          = 0;
static int          cached_num_cols          = 0;
static int*         cached_neighbours = NULL;
#ifdef IMPLEMENT_MERGING
static int**        merge_counts                   = NULL;
static int* segment_indices_after_merging = NULL;
static Matrix* merge_r_diff_mp = NULL;
static Matrix* merge_g_diff_mp = NULL;
static Matrix* merge_sum_diff_mp = NULL;
#endif
/* We play it a bit fast and loose to reduce stack use during recursion. */
static const KJB_image* cached_ip;    /* Do NOT free this! */
static int** cached_seg_map;            /* Do NOT free this! */
static float cached_R_min;
static float cached_R_max;
static float cached_G_min;
static float cached_G_max;
static float cached_B_min;
static float cached_B_max;
static float cached_r_chrom_min;
static float cached_r_chrom_max;
static float cached_g_chrom_min;
static float cached_g_chrom_max;
static float cached_sum_RGB_min;
static float cached_sum_RGB_max;

/* -------------------------------------------------------------------------- */

static void segment_image_helper(int i, int j);
static int  grow_segment(int i, int j);
static int  grow_segment_using_rg_var_and_lum_diff(int i, int j);
static void fill_holes(void);

static int find_boundary_pixels(Segmentation_t1* image_segment_info_ptr);



#ifdef IMPLEMENT_MERGING
static int merge_segments(Segmentation_t1* image_segment_info_ptr);

static int merge_segment_pair
(
    int           i,
    int           j,
    Segmentation_t1* image_segment_info_ptr
);

#endif

static int find_outside_boundary(Segmentation_t1* image_segment_info_ptr);

static int find_neighbours(Segmentation_t1* image_segment_info_ptr);

static int find_interior_points(Segmentation_t1* image_segment_info_ptr);

static void zero_seg_map(Segmentation_t1* image_segment_info_ptr);

static void update_seg_map(Segmentation_t1* image_segment_info_ptr);

static int get_target_image_segment_info
(
    Segmentation_t1** image_seg_info_ptr_ptr,
    int            num_rows,
    int            num_cols
);

static Segmentation_t1* create_image_segment_info(void);

static Segment_t1* create_image_segment(int segment_number);

static void free_image_segment_storage(void* segment_ptr);

static void free_image_segment(Segment_t1* segment_ptr);

static int initialize_pixel_cache(int new_cache_size);

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_allocated_static_data(void);
    static void prepare_memory_cleanup(void);
#endif

/* -------------------------------------------------------------------------- */

static Method_option segmentation_methods[ ] =
{
    { "general",         "g",      (int(*)())grow_segment},
    { "rg-var-lum-diff", "rg-lum", (int(*)())grow_segment_using_rg_var_and_lum_diff}
};

static const int num_segmentation_methods =
                                sizeof(segmentation_methods) /
                                          sizeof(segmentation_methods[ 0 ]);
static int         segmentation_method                  = 0;
static const char* segmentation_method_short_str= "t1-seg-method";
static const char* segmentation_method_long_str = "t1-segmentation-method";

/* -------------------------------------------------------------------------- */

int t1_set_segmentation_options(const char* option, const char* value)
{
    char   lc_option[ 100 ];
    int    temp_int;
    double   temp_real;
    float  temp_float;
    int    result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, segmentation_method_short_str)
          || match_pattern(lc_option, segmentation_method_long_str)
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(parse_method_option(segmentation_methods, num_segmentation_methods,
                                segmentation_method_long_str,
                                "T1 segmentation method",
                                value, &segmentation_method));
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-segmentation-fill-hole-level"))
         || (match_pattern(lc_option, "t1-seg-fill-hole-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-fill-hole-level = %d\n",
                    seg_fill_hole_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation holes are %s .\n",
                    (seg_fill_hole_level <= 0) ? "not filled"
                    : (seg_fill_hole_level == 1) ? "filled once"
                    : "always filled, even after merges"));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            seg_fill_hole_level = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-segmentation-min-segment-size"))
         || (match_pattern(lc_option, "t1-seg-min-segment-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-min-segment-size = %d\n",
                    seg_min_segment_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation min segment size is %d.\n",
                    seg_min_segment_size));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) seg_min_segment_size = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-segmentation-resegment-size"))
         || (match_pattern(lc_option, "t1-seg-resegment-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-resegment-size = %d\n",
                    seg_min_resegment_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation resegment size is %d.\n",
                    seg_min_resegment_size));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) seg_min_resegment_size = temp_int;
        }

        result = NO_ERROR;
    }

#ifdef CONNECTION_MAX_STEP_OPTION
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-segmentation-connection-max-step"))
         || (match_pattern(lc_option, "t1-seg-connection-max-step"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-connection-max-step = %d\n", connection_max_step));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation connection max step is %d.\n",
                    connection_max_step));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) connection_max_step = temp_int;
        }

        result = NO_ERROR;
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-segmentation-resegment-level"))
         || (match_pattern(lc_option, "t1-seg-resegment-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-resegment-level = %d\n",
                    seg_resegment_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation resegment level is %d.\n",
                    seg_resegment_level));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            seg_resegment_level = temp_int;
        }

        result = NO_ERROR;
    }

#ifdef IMPLEMENT_MERGING
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-merge-rgb-noise-estimate"))
         || (match_pattern(lc_option, "t1-seg-merge-rgb-noise"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-merge-rgb-noise-estimate = %.2f\n",
                    merge_RGB_noise_estimate));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation RGB noise estimate for merge threshold is %.2f.\n",
                    merge_RGB_noise_estimate));
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            merge_RGB_noise_estimate = temp_real;
        }

        result = NO_ERROR;
    }
#endif

#ifdef IMPLEMENT_MERGING
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-merge-rg-threshold"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-merge-rg-threshold = %.2f\n",
                    merge_rg_threshold));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation merge rg threshold is %.2f.\n",
                    merge_rg_threshold));
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            merge_rg_threshold = temp_real;
        }

        result = NO_ERROR;
    }
#endif

#ifdef IMPLEMENT_MERGING
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-merge-sum-abs-threshold"))
         || (match_pattern(lc_option, "t1-seg-merge-rgb-sum-abs-threshold"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-merge-rgb-sum-abs-threshold = %.2f\n",
                    merge_sum_RGB_abs_threshold));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation merge RGB sum threshold is %.2f.\n",
                    merge_sum_RGB_abs_threshold));
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            merge_rg_threshold = temp_real;
        }

        result = NO_ERROR;
    }
#endif

#ifdef IMPLEMENT_MERGING
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-merge-sum-rel-threshold"))
         || (match_pattern(lc_option, "t1-seg-merge-rgb-sum-rel-threshold"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-merge-rgb-sum-rel-threshold = %.2f\n",
                    merge_sum_RGB_rel_threshold));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation merge RGB sum threshold is %.2f.\n",
                    merge_sum_RGB_rel_threshold));
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            merge_rg_threshold = temp_real;
        }

        result = NO_ERROR;
    }
#endif

#ifdef IMPLEMENT_MERGING
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-merge-min-num-pixels"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-merge-min-num-pixels = %d\n",
                    merge_min_num_pixels));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation merge min number of adjacent pixels is %d.\n",
                    merge_min_num_pixels));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            merge_min_num_pixels = temp_int;
        }

        result = NO_ERROR;
    }
#endif

#ifdef IMPLEMENT_MERGING
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-segmentation-merge-level"))
         || (match_pattern(lc_option, "t1-seg-merge-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-merge-level = %d\n",
                    seg_merge_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation merge level is %d.\n",
                    seg_merge_level));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            seg_merge_level = temp_int;
        }

        result = NO_ERROR;
    }
#endif

#ifdef CONNECT_CORNER_OPTION
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-segmentation-connect-corners"))
         || (match_pattern(lc_option, "t1-seg-connect-corners"))
       )
    {
        int temp_boolean;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-connect-corners = %s\n",
                    seg_connect_corners ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T1 segmentation connects corners %s used.\n",
                    seg_connect_corners ? "is" : "is not"));
        }
        else
        {
            ERE(temp_boolean = get_boolean_value(value));
            seg_connect_corners = temp_boolean;
        }

        result = NO_ERROR;
    }
#endif

#ifdef USE_NO_VALUE
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_rel_RGB_var < 0.0)
            {
                ERE(pso("t1-seg-max-rel-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-rel-rgb-var = %.3f\n",
                        (double)max_rel_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_RGB_var < 0.0)
            {
                ERE(pso("Maximum relative RGB variance is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative RGB variance for T1 segmenting is %.3f.\n",
                        (double)max_rel_RGB_var));
            }
        }
        else if (is_no_value_word(value))
        {
            max_rel_RGB_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_abs_RGB_var < 0.0)
            {
                ERE(pso("t1-seg-max-abs-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-abs-rgb-var = %.3f\n",
                        (double)max_abs_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_RGB_var < 0.0)
            {
                ERE(pso("Maximum absolute RGB variance is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute RGB variance for T1 segmenting is %.3f.\n",
                        (double)max_abs_RGB_var));
            }
        }
        else if (is_no_value_word(value))
        {
            max_abs_RGB_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-sum-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_rel_sum_RGB_var < 0.0)
            {
                ERE(pso("t1-seg-max-rel-sum-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-rel-sum-rgb-var = %.3f\n",
                        (double)max_rel_sum_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_sum_RGB_var < 0.0)
            {
                ERE(pso("Maximum relative sum RGB variance is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative sum RGB variance for T1 segmenting is %.3f.\n",
                        (double)max_rel_sum_RGB_var));
            }
        }
        else if (is_no_value_word(value))
        {
            max_rel_sum_RGB_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_sum_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-sum-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_abs_sum_RGB_var < 0.0)
            {
                ERE(pso("t1-seg-max-abs-sum-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-abs-sum-rgb-var = %.3f\n",
                        (double)max_abs_sum_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_sum_RGB_var < 0.0)
            {
                ERE(pso("Maximum absolute sum RGB variance is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute sum RGB variance for T1 segmenting is %.3f.\n",
                        (double)max_abs_sum_RGB_var));
            }
        }
        else if (is_no_value_word(value))
        {
            max_abs_sum_RGB_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_sum_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-chrom-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_rel_chrom_var < 0.0)
            {
                ERE(pso("t1-seg-max-rel-chrom-var = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-rel-chrom-var = %.3f\n",
                        (double)max_rel_chrom_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_chrom_var < 0.0)
            {
                ERE(pso("Maximum relative chrom variance is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative chrom variance for T1 segmenting is %.3f.\n",
                        (double)max_rel_chrom_var));
            }
        }
        else if (is_no_value_word(value))
        {
            max_rel_chrom_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_chrom_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-chrom-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_abs_chrom_var < 0.0)
            {
                ERE(pso("t1-seg-max-abs-chrom-var = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-abs-chrom-var = %.3f\n",
                        (double)max_abs_chrom_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_chrom_var < 0.0)
            {
                ERE(pso("Maximum absolute chrom variance is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute chrom variance for T1 segmenting is %.3f.\n",
                        (double)max_abs_chrom_var));
            }
        }
        else if (is_no_value_word(value))
        {
            max_abs_chrom_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_chrom_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-rgb-sqr-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_rel_RGB_sqr_diff < 0.0)
            {
                ERE(pso("t1-seg-max-rel-rgb-sqr-diff = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-rel-rgb-sqr-diff = %.3f\n",
                        (double)max_rel_RGB_sqr_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_RGB_sqr_diff < 0.0)
            {
                ERE(pso("Maximum relative sum RGB sqr difference is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative sum RGB sqr difference for T1 segmenting is %.3f.\n",
                        (double)max_rel_RGB_sqr_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            max_rel_RGB_sqr_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_RGB_sqr_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-rgb-sqr-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_abs_RGB_sqr_diff < 0.0)
            {
                ERE(pso("t1-seg-max-abs-rgb-sqr-diff = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-abs-rgb-sqr-diff = %.3f\n",
                        (double)max_abs_RGB_sqr_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_RGB_sqr_diff < 0.0)
            {
                ERE(pso("Maximum absolute sum RGB sqr difference is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute sum RGB sqr difference for T1 segmenting is %.3f.\n",
                        (double)max_abs_RGB_sqr_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            max_abs_RGB_sqr_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_RGB_sqr_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-sum-rgb-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_rel_sum_RGB_diff < 0.0)
            {
                ERE(pso("t1-seg-max-rel-sum-rgb-diff = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-rel-sum-rgb-diff = %.3f\n",
                        (double)max_rel_sum_RGB_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_sum_RGB_diff < 0.0)
            {
                ERE(pso("Maximum relative sum RGB difference is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative sum RGB difference for T1 segmenting is %.3f.\n",
                        (double)max_rel_sum_RGB_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            max_rel_sum_RGB_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_sum_RGB_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-sum-rgb-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_abs_sum_RGB_diff < 0.0)
            {
                ERE(pso("t1-seg-max-abs-sum-rgb-diff = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-abs-sum-rgb-diff = %.3f\n",
                        (double)max_abs_sum_RGB_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_sum_RGB_diff < 0.0)
            {
                ERE(pso("Maximum absolute sum RGB difference is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute sum RGB difference for T1 segmenting is %.3f.\n",
                        (double)max_abs_sum_RGB_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            max_abs_sum_RGB_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_sum_RGB_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-chrom-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_rel_chrom_diff < 0.0)
            {
                ERE(pso("t1-seg-max-rel-chrom-diff = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-rel-chrom-diff = %.3f\n",
                        (double)max_rel_chrom_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_chrom_diff < 0.0)
            {
                ERE(pso("Maximum relative chrom difference is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative chrom difference for T1 segmenting is %.3f.\n",
                        (double)max_rel_chrom_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            max_rel_chrom_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_chrom_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-chrom-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (max_abs_chrom_diff < 0.0)
            {
                ERE(pso("t1-seg-max-abs-chrom-diff = off\n"));
            }
            else
            {
                ERE(pso("t1-seg-max-abs-chrom-diff = %.3f\n",
                        (double)max_abs_chrom_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_chrom_diff < 0.0)
            {
                ERE(pso("Maximum absolute chrom difference is not used for T1 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute chrom difference for T1 segmenting is %.3f.\n",
                        (double)max_abs_chrom_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            max_abs_chrom_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_chrom_diff = temp_float;
        }
        result = NO_ERROR;
    }
#else
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-rgb-var"))
       )
    {
        extern float max_rel_RGB_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-rel-rgb-var = %.3f\n",
                    (double)max_rel_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative RGB variance for T1 segmenting is %.3f.\n",
                    (double)max_rel_RGB_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-rgb-var"))
       )
    {
        extern float max_abs_RGB_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-abs-rgb-var = %.3f\n", (double)max_abs_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute RGB variance for T1 segmenting is %.3f.\n",
                    (double)max_abs_RGB_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-sum-rgb-var"))
       )
    {
        extern float max_rel_sum_RGB_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-rel-sum-rgb-var = %.3f\n",
                    (double)max_rel_sum_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative sum RGB variance for T1 segmenting is %.3f.\n",
                    (double)max_rel_sum_RGB_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_sum_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-sum-rgb-var"))
       )
    {
        extern float max_abs_sum_RGB_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-abs-sum-rgb-var = %.3f\n",
                    (double)max_abs_sum_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute sum RGB variance for T1 segmenting is %.3f.\n",
                    (double)max_abs_sum_RGB_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_sum_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-chrom-var"))
       )
    {
        extern float max_rel_chrom_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-rel-chrom-var = %.3f\n",
                    (double)max_rel_chrom_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative chrom variance for T1 segmenting is %.3f.\n",
                    (double)max_rel_chrom_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_chrom_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-chrom-var"))
       )
    {
        extern float max_abs_chrom_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-abs-chrom-var = %.3f\n",
                    (double)max_abs_chrom_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute chrom variance for T1 segmenting is %.3f.\n",
                    (double)max_abs_chrom_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_chrom_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-rgb-sqr-diff"))
       )
    {
        extern float max_rel_RGB_sqr_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-rel-rgb-sqr-diff = %.3f\n",
                    (double)max_rel_RGB_sqr_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative sum RGB sqr difference for T1 segmenting is %.3f.\n",
                    (double)max_rel_RGB_sqr_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_RGB_sqr_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-rgb-sqr-diff"))
       )
    {
        extern float max_abs_RGB_sqr_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-abs-rgb-sqr-diff = %.3f\n",
                    (double)max_abs_RGB_sqr_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute sum RGB sqr difference for T1 segmenting is %.3f.\n",
                    (double)max_abs_RGB_sqr_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_RGB_sqr_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-sum-rgb-diff"))
       )
    {
        extern float max_rel_sum_RGB_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-rel-sum-rgb-diff = %.3f\n",
                    (double)max_rel_sum_RGB_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative sum RGB difference for T1 segmenting is %.3f.\n",
                    (double)max_rel_sum_RGB_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_sum_RGB_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-sum-rgb-diff"))
       )
    {
        extern float max_abs_sum_RGB_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-abs-sum-rgb-diff = %.3f\n",
                    (double)max_abs_sum_RGB_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute sum RGB difference for T1 segmenting is %.3f.\n",
                    (double)max_abs_sum_RGB_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_sum_RGB_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-rel-chrom-diff"))
       )
    {
        extern float max_rel_chrom_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-rel-chrom-diff = %.3f\n",
                    (double)max_rel_chrom_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative chrom difference for T1 segmenting is %.3f.\n",
                    (double)max_rel_chrom_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_rel_chrom_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-max-abs-chrom-diff"))
       )
    {
        extern float max_abs_chrom_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-max-abs-chrom-diff = %.3f\n",
                    (double)max_abs_chrom_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute chrom difference for T1 segmenting is %.3f.\n",
                    (double)max_abs_chrom_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            max_abs_chrom_diff = temp_float;
        }
        result = NO_ERROR;
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-min-rgb-sum-for-chrom-test"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-min-rgb-sum-for-chrom-test = %.3f\n",
                    (double)min_sum_RGB_for_chrom_test));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Min sum RGB for chrom tests is %.3f.\n",
                    (double)min_sum_RGB_for_chrom_test));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            min_sum_RGB_for_chrom_test = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-seg-min-rgb-sum-for-relative-sum-diff"))
         || (match_pattern(lc_option, "t1-seg-min-rgb-sum-for-relative-diff"))
         || (match_pattern(lc_option, "t1-seg-min-rgb-sum-for-rel-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-min-rgb-sum-for-relative-sum-diff = %.3f\n",
                    (double)min_sum_RGB_for_relative_sum_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Min sum RGB for relative sum differences is %.3f.\n",
                    (double)min_sum_RGB_for_relative_sum_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            min_sum_RGB_for_relative_sum_diff = temp_float;
        }
        result = NO_ERROR;
    }

#ifdef TEST
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t1-segmentation-verify-segmentation"))
         || (match_pattern(lc_option, "t1-seg-verify-segmentation"))
       )
    {
        int temp_boolean;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t1-seg-verify-segmentation = %s\n",
                    seg_verify_segmentation ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Segmentations %s verified.\n",
                    seg_verify_segmentation ? "are" : "are not"));
        }
        else
        {
            ERE(temp_boolean = get_boolean_value(value));
            seg_verify_segmentation = temp_boolean;
        }

        result = NO_ERROR;
    }
#endif

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          t1_segment_image
 *
 * Segments the image into regions
 *
 * This routine segments an image into regions. The segmentation is done under
 * the control of a large number of options normally exposed to the user. The
 * routine makes minor changes to the image to ensure consistency between the
 * segmentation and the image. If you do not want the image to be changed, you
 * MUST make a copy. The routine fills a structure which includes:
 *     The number of segments
 *     An image size integer array with the segment number at each (i,j)
 *     An array of segments (Type Segment_t1*).
 *
 * Each segment contains (among other things!):
 *     Pixel count
 *     A list of pixels (values and their coordinates)
 *     Boundary pixel count
 *     A list of boundary pixels
 *     A matrix of outer boudary coordinates IN ORDER
 *     Segment averge R, G, B, r, g, sum_RGB
 *     An array of segment numbers of neighbours
 *     Coordinates of segment center of mass
 *     Coordinates of a pixel inside the segment
 *
 * Returns:
 *     The number of image segments on sucess and ERROR on failure.
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

int t1_segment_image
(
    const KJB_image* ip,
    Segmentation_t1**   image_segment_info_ptr_ptr
)
{
    Segmentation_t1* image_segment_info_ptr;
    Segment_t1*      seg_ptr;
    int              num_rows, num_cols, i, j;
    Queue_element*   segment_list_head = NULL;
    int              num_good_segments = 0;
    long             initial_cpu_time  = get_cpu_time();


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    dbp("___________________________________");

    cached_num_rows = num_rows = ip->num_rows;
    cached_num_cols = num_cols = ip->num_cols;

    ERE(get_target_image_segment_info(image_segment_info_ptr_ptr,
                                      num_rows, num_cols));
    image_segment_info_ptr = *image_segment_info_ptr_ptr;

    zero_seg_map(image_segment_info_ptr);

    ERE(initialize_pixel_cache(4 * num_rows * num_cols));

    /* Normally bad practice, but we want to improve performace! */
    cached_ip = ip;
    cached_seg_map = image_segment_info_ptr->seg_map;

    cached_segment_count = 0;

    /*
    // Visit each pixel. For each one that is not marked, create a segment, and
    // segment the image by growing the region. (Note that as the region
    // grows, those pixels get marked).
    */
    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            if (    (cached_seg_map[ i ][ j ] == 0)
                 && (cached_ip->pixels[ i ][ j ].extra.invalid.pixel == VALID_PIXEL)
               )
            {
                cached_segment_count++;

                segment_image_helper(i, j);

                if (num_cached_pixels >= seg_min_segment_size)
                {
                    /*
                    // Note, this is not the cached_segment_count, because we
                    // are only counting bigger segments.
                    */
                    num_good_segments++;

                    seg_ptr = create_image_segment(num_good_segments);

                    if (seg_ptr == NULL)
                    {
                        UNTESTED_CODE();
                        num_good_segments = ERROR;
                        break;
                    }

                    if (insert_at_end_of_queue(&segment_list_head,
                                               (Queue_element**)NULL,
                                               seg_ptr)
                        == ERROR)
                    {
                        UNTESTED_CODE();
                        free_image_segment(seg_ptr);
                        num_good_segments = ERROR;
                        break;
                    }
                }
            }

            if (num_good_segments == ERROR) break;
        }
    }

    if (num_good_segments != ERROR)
    {
        image_segment_info_ptr->num_segments = num_good_segments;
        image_segment_info_ptr->segments = N_TYPE_MALLOC(Segment_t1*,
                                                         num_good_segments);

        if (image_segment_info_ptr->segments == NULL)
        {
            UNTESTED_CODE();
            num_good_segments = ERROR;
        }
    }

    /* Put list of segments into segment info structure. */
    if (num_good_segments != ERROR)
    {
        Queue_element* cur_elem = segment_list_head;

        for (i=0; i<num_good_segments; i++)
        {
            Queue_element* save_cur_elem = cur_elem;

            seg_ptr = (Segment_t1*)(cur_elem->contents);
            image_segment_info_ptr->segments[ i ] = seg_ptr;
            cur_elem = cur_elem->next;
            kjb_free(save_cur_elem);
        }
    }
    else
    {
        UNTESTED_CODE();
        free_queue(&segment_list_head, (Queue_element**)NULL,
                   free_image_segment_storage);
    }

    /*
    // Once the storage has been moved from the queue to image_segment_info_ptr,
    // we no longer need to free on failure, as this is now the callers
    // responsibility.
    */

    if (num_good_segments != ERROR)
    {
        verbose_pso(3, "I found %d segments in %ldms.\n",
                    image_segment_info_ptr->num_segments,
                    get_cpu_time() - initial_cpu_time);
    }

    /*
    // The call to find_boundary_pixels updates the segment map.
    //
    // update_seg_map(image_segment_info_ptr);
    */

    if (num_good_segments != ERROR)
    {
        if (find_boundary_pixels(image_segment_info_ptr) == ERROR)
        {
            num_good_segments = ERROR;
        }
    }

#ifdef IMPLEMENT_MERGING
    if (num_good_segments != ERROR)
    {
        int merge;

        for (merge = 0; merge < seg_merge_level; merge++)
        {
            num_good_segments = merge_segments(image_segment_info_ptr);

            if (num_good_segments == ERROR) break;

            if (find_boundary_pixels(image_segment_info_ptr) == ERROR)
            {
                num_good_segments = ERROR;
            }
            /*
            if (seg_fill_hole_level > 1)
            {
                fill_all_holes();
            }
            */
        }
    }
#endif

    if (num_good_segments != ERROR)
    {
        if (find_outside_boundary(image_segment_info_ptr) == ERROR)
        {
            num_good_segments = ERROR;
        }
    }

    if (num_good_segments != ERROR)
    {
        if (find_neighbours(image_segment_info_ptr) == ERROR)
        {
            num_good_segments = ERROR;
        }
    }

    if (num_good_segments != ERROR)
    {
        if (find_interior_points(image_segment_info_ptr) == ERROR)
        {
            num_good_segments = ERROR;
        }
    }

    if (num_good_segments != ERROR)
    {
        verbose_pso(3, "Complete segmentation took %ldms.\n",
                    get_cpu_time() - initial_cpu_time);
    }

    return num_good_segments;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void segment_image_helper(int i, int j)
{
    float R, G, B;
    float sum_RGB;
    float r_chrom;
    float g_chrom;
    int   num_seg;
    int   reseg;
    int (*seg_fn)(int,int) =  (int(*)(int,int))segmentation_methods[ segmentation_method ].fn;

    num_seg = 1;

    /* There is no point in re-segmenting, unless we are setting the overall
    // colour (as opposed to only relying on differences.)
    */
    if (    (max_abs_RGB_var >= FLT_ZERO)
         || (max_rel_RGB_var >= FLT_ZERO)
         || (max_abs_chrom_var >= FLT_ZERO)
         || (max_rel_chrom_var >= FLT_ZERO)
         || (max_abs_sum_RGB_var >= FLT_ZERO)
         || (max_rel_sum_RGB_var >= FLT_ZERO)
       )
    {
        num_seg += seg_resegment_level;
    }

    for (reseg = 0; reseg < num_seg; reseg++)
    {
        if (reseg != 0)
        {
            int k;

            R = cached_R_sum / num_cached_pixels;
            G = cached_G_sum / num_cached_pixels;
            B = cached_B_sum / num_cached_pixels;

            sum_RGB = R + G + B + RGB_SUM_EPSILON;

            r_chrom = R / sum_RGB;
            g_chrom = G / sum_RGB;

            /*
            // If we are going to re-segment, we have to undo what we just did
            // to the marked pixel database.
            */
            for (k=0; k<num_cached_pixels; k++)
            {
                int ii = cached_pixels[ k ].i;
                int jj = cached_pixels[ k ].j;

                cached_seg_map[ ii ][ jj ] = 0;
            }
        }
        else
        {
            R = cached_ip->pixels[ i  ][ j ].r;
            G = cached_ip->pixels[ i  ][ j ].g;
            B = cached_ip->pixels[ i  ][ j ].b;
            sum_RGB = R + G + B + RGB_SUM_EPSILON;
            r_chrom = R / sum_RGB;
            g_chrom = G / sum_RGB;
        }

        num_cached_pixels = 0;

        cached_i_min = cached_num_rows - 1;
        cached_i_max = 0;
        cached_j_min = cached_num_cols - 1;
        cached_j_max = 0;

        cached_i_sum = 0.0;
        cached_j_sum = 0.0;

        cached_R_sum = 0.0;
        cached_G_sum = 0.0;
        cached_B_sum = 0.0;

#ifdef USE_NO_VALUE
        if (    (max_abs_RGB_var >= FLT_ZERO)
             && (max_rel_RGB_var >= FLT_ZERO))
        {
            cached_R_min = MAX_OF(R - max_abs_RGB_var,
                                  R*(1.0 - max_rel_RGB_var));
            cached_R_max = MIN_OF(R + max_abs_RGB_var,
                                  R*(1.0 + max_rel_RGB_var));

            cached_G_min = MAX_OF(G - max_abs_RGB_var,
                                  G*(1.0 - max_rel_RGB_var));
            cached_G_max = MIN_OF(G + max_abs_RGB_var,
                                  G*(1.0 + max_rel_RGB_var));

            cached_B_min = MAX_OF(B - max_abs_RGB_var,
                                  B*(1.0 - max_rel_RGB_var));
            cached_B_max = MIN_OF(B + max_abs_RGB_var,
                                  B*(1.0 + max_rel_RGB_var));
        }
        else if (max_abs_RGB_var >= FLT_ZERO)
        {
            cached_R_min = R - max_abs_RGB_var;
            cached_R_max = R + max_abs_RGB_var;
            cached_G_min = G - max_abs_RGB_var;
            cached_G_max = G + max_abs_RGB_var;
            cached_B_min = B - max_abs_RGB_var;
            cached_B_max = B + max_abs_RGB_var;
        }
        else if (max_rel_RGB_var >= FLT_ZERO)
        {
            cached_R_min = R*(1.0 - max_rel_RGB_var);
            cached_R_max = R*(1.0 + max_rel_RGB_var);
            cached_G_min = G*(1.0 - max_rel_RGB_var);
            cached_G_max = G*(1.0 + max_rel_RGB_var);
            cached_B_min = B*(1.0 - max_rel_RGB_var);
            cached_B_max = B*(1.0 + max_rel_RGB_var);
        }
        else
        {
            cached_R_min = FLT_NOT_SET;
            cached_R_max = FLT_NOT_SET;
            cached_G_min = FLT_NOT_SET;
            cached_G_max = FLT_NOT_SET;
            cached_B_min = FLT_NOT_SET;
            cached_B_max = FLT_NOT_SET;
        }

        if (    (max_abs_chrom_var >= FLT_ZERO)
             && (max_rel_chrom_var >= FLT_ZERO))
        {
            cached_r_chrom_min = MAX_OF(r_chrom - max_abs_chrom_var,
                                        r_chrom*(1.0 - max_rel_chrom_var));
            cached_r_chrom_max = MIN_OF(r_chrom + max_abs_chrom_var,
                                        r_chrom*(1.0 + max_rel_chrom_var));

            cached_g_chrom_min = MAX_OF(g_chrom - max_abs_chrom_var,
                                        g_chrom*(1.0 - max_rel_chrom_var));
            cached_g_chrom_max = MIN_OF(g_chrom + max_abs_chrom_var,
                                        g_chrom*(1.0 + max_rel_chrom_var));
        }
        else if (max_abs_chrom_var >= FLT_ZERO)
        {
            cached_r_chrom_min = r_chrom - max_abs_chrom_var;
            cached_r_chrom_max = r_chrom + max_abs_chrom_var;
            cached_g_chrom_min = g_chrom - max_abs_chrom_var;
            cached_g_chrom_max = g_chrom + max_abs_chrom_var;
        }
        else if (max_rel_chrom_var >= FLT_ZERO)
        {
            cached_r_chrom_min = r_chrom*(1.0 - max_rel_chrom_var);
            cached_r_chrom_max = r_chrom*(1.0 + max_rel_chrom_var);
            cached_g_chrom_min = g_chrom*(1.0 - max_rel_chrom_var);
            cached_g_chrom_max = g_chrom*(1.0 + max_rel_chrom_var);
        }
        else
        {
            cached_r_chrom_min = FLT_NOT_SET;
            cached_r_chrom_max = FLT_NOT_SET;
            cached_g_chrom_min = FLT_NOT_SET;
            cached_g_chrom_max = FLT_NOT_SET;
        }

        if (    (max_abs_sum_RGB_var >= FLT_ZERO)
             && (max_rel_sum_RGB_var >= FLT_ZERO))
        {
            cached_sum_RGB_min = MIN_OF(sum_RGB - max_abs_sum_RGB_var,
                                        sum_RGB*(1.0 - max_rel_sum_RGB_var));
            cached_sum_RGB_max = MAX_OF(sum_RGB + max_abs_sum_RGB_var,
                                        sum_RGB*(1.0 + max_rel_sum_RGB_var));
        }
        else if (max_abs_sum_RGB_var >= FLT_ZERO)
        {
            cached_sum_RGB_min = sum_RGB - max_abs_sum_RGB_var;
            cached_sum_RGB_max = sum_RGB + max_abs_sum_RGB_var;
        }
        else if (max_rel_sum_RGB_var >= FLT_ZERO)
        {
            cached_sum_RGB_min = sum_RGB*(1.0 - max_rel_sum_RGB_var);
            cached_sum_RGB_max = sum_RGB*(1.0 + max_rel_sum_RGB_var);
        }
        else
        {
            cached_sum_RGB_min = FLT_NOT_SET;
            cached_sum_RGB_max = FLT_NOT_SET;
        }
#else
        cached_R_min = MAX_OF(R - max_abs_RGB_var,
                              R*(1.0 - max_rel_RGB_var));
        cached_R_max = MIN_OF(R + max_abs_RGB_var,
                              R*(1.0 + max_rel_RGB_var));

        cached_G_min = MAX_OF(G - max_abs_RGB_var,
                              G*(1.0 - max_rel_RGB_var));
        cached_G_max = MIN_OF(G + max_abs_RGB_var,
                              G*(1.0 + max_rel_RGB_var));

        cached_B_min = MAX_OF(B - max_abs_RGB_var,
                              B*(1.0 - max_rel_RGB_var));
        cached_B_max = MIN_OF(B + max_abs_RGB_var,
                              B*(1.0 + max_rel_RGB_var));

        cached_r_chrom_min = MAX_OF(r_chrom - max_abs_chrom_var,
                                    r_chrom*(1.0 - max_rel_chrom_var));
        cached_r_chrom_max = MIN_OF(r_chrom + max_abs_chrom_var,
                                    r_chrom*(1.0 + max_rel_chrom_var));

        cached_g_chrom_min = MAX_OF(g_chrom - max_abs_chrom_var,
                                    g_chrom*(1.0 - max_rel_chrom_var));
        cached_g_chrom_max = MIN_OF(g_chrom + max_abs_chrom_var,
                                    g_chrom*(1.0 + max_rel_chrom_var));
        cached_sum_RGB_min = MAX_OF(sum_RGB - max_abs_sum_RGB_var,
                                    sum_RGB*(1.0 - max_rel_sum_RGB_var));
        cached_sum_RGB_max = MIN_OF(sum_RGB + max_abs_sum_RGB_var,
                                    sum_RGB*(1.0 + max_rel_sum_RGB_var));
#endif

        if (reseg != 0)
        {
            verbose_pso(150, "Re-");
        }
        verbose_pso(150, "Growing (%-3d, %-3d) ... ", i, j);

        seg_fn(i, j);

        verbose_pso(150, "%d pixels.\n", num_cached_pixels);

        if (num_cached_pixels < seg_min_resegment_size)
        {
            break;
        }
    }

    if (seg_fill_hole_level > 0)
    {
        fill_holes();
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int grow_segment(int i, int j)
{
    int   di, dj;
    int   i_offset;
    int   j_offset;
    float cur_R;
    float cur_G;
    float cur_B;
    float cur_r_chrom;
    float cur_g_chrom;
    float cur_sum_RGB;
    float cur_RGB_sqr;
    Pixel cur_image_pixel;


    cur_image_pixel = cached_ip->pixels[ i ][ j ];

    ASSERT(! (cached_seg_map[ i ][ j ]));
    ASSERT(cur_image_pixel.extra.invalid.pixel == VALID_PIXEL);

    cached_pixels[ num_cached_pixels ].i = i;
    cached_pixels[ num_cached_pixels ].j = j;

    cur_R = cur_image_pixel.r;
    cur_G = cur_image_pixel.g;
    cur_B = cur_image_pixel.b;

    cur_sum_RGB = cur_R + cur_G + cur_B + RGB_SUM_EPSILON;
    cur_r_chrom = cur_R / cur_sum_RGB;
    cur_g_chrom = cur_G / cur_sum_RGB;
    cur_RGB_sqr = (cur_R * cur_R) + (cur_G * cur_G) + (cur_B * cur_B);

    cached_R_sum += cur_R;
    cached_G_sum += cur_G;
    cached_B_sum += cur_B;
    cached_i_sum += i;
    cached_j_sum += j;

    num_cached_pixels++;

    if (i > cached_i_max) cached_i_max = i;
    if (i < cached_i_min) cached_i_min = i;
    if (j > cached_j_max) cached_j_max = j;
    if (j < cached_j_min) cached_j_min = j;

    cached_seg_map[ i ][ j ] = cached_segment_count;

    for (di = -1; di <= 1; di++)
    {
        for (dj = -1; dj <= 1; dj++)
        {
            if ((di != 0) || (dj != 0))
            {
                i_offset = i + di;
                j_offset = j + dj;

                if (    (i_offset >= 0)
                     && (j_offset >= 0)
                     && (i_offset < cached_num_rows)
                     && (j_offset < cached_num_cols)
#ifdef CONNECT_CORNER_OPTION
                     && (seg_connect_corners || ((di == 0) || (dj == 0)))
#else
#ifndef DEFAULT_IS_TO_CONNECT_CORNERS
                     && ((di == 0) || (dj == 0))
#endif
#endif
                     && ( ! cached_seg_map[ i_offset ][ j_offset ])
                     && (cached_ip->pixels[ i_offset ][ j_offset ].extra.invalid.pixel == VALID_PIXEL)
                   )
                {
                    int skip = FALSE;
                    float R = cached_ip->pixels[ i_offset ][ j_offset ].r;
                    float G = cached_ip->pixels[ i_offset ][ j_offset ].g;
                    float B = cached_ip->pixels[ i_offset ][ j_offset ].b;
                    float sum_RGB = R + G + B + RGB_SUM_EPSILON;
                    float r_chrom = R / sum_RGB;
                    float g_chrom = G / sum_RGB;
                    float d_r_chrom = ABS_OF(cur_r_chrom - r_chrom);
                    float d_g_chrom = ABS_OF(cur_g_chrom - g_chrom);
                    float d_sum_RGB = ABS_OF(cur_sum_RGB - sum_RGB);
                    float d_R = ABS_OF(cur_R - R);
                    float d_G = ABS_OF(cur_G - G);
                    float d_B = ABS_OF(cur_B - B);
                    float d_RGB_sqr = (d_R * d_R) + (d_G * d_G) + (d_B * d_B);
                    float d_rel_r_chrom = d_r_chrom / (r_chrom + CHROM_EPSILON);
                    float d_rel_g_chrom = d_g_chrom / (g_chrom + CHROM_EPSILON);
                    float d_rel_sum_RGB = d_sum_RGB / sum_RGB;
                    float d_rel_RGB_sqr = d_RGB_sqr / (cur_RGB_sqr + RGB_SUM_EPSILON);

#ifdef USE_NO_VALUE
                    if ((cached_R_min >= 0.0) && (R < cached_R_min))
                    {
                        skip = TRUE;
                    }
                    else if ((cached_R_max >= 0.0) && (R > cached_R_max))
                    {
                        skip = TRUE;
                    }
                    else if ((cached_G_min >= 0.0) && (G < cached_G_min))
                    {
                        skip = TRUE;
                    }
                    else if ((cached_G_max >= 0.0) && (G > cached_G_max))
                    {
                        skip = TRUE;
                    }
                    else if ((cached_B_min >= 0.0) && (B < cached_B_min))
                    {
                        skip = TRUE;
                    }
                    else if ((cached_B_max >= 0.0) && (B > cached_B_max))
                    {
                        skip = TRUE;
                    }
                    else if (    (cached_r_chrom_min >= 0.0)
                              && (r_chrom < cached_r_chrom_min)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (cached_r_chrom_max >= 0.0)
                              && (r_chrom > cached_r_chrom_max)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (cached_g_chrom_min >= 0.0)
                              && (g_chrom < cached_g_chrom_min)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (cached_g_chrom_max >= 0.0)
                              && (g_chrom > cached_g_chrom_max)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (cached_sum_RGB_min >= 0.0)
                              && (sum_RGB < cached_sum_RGB_min)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (cached_sum_RGB_max >= 0.0)
                              && (sum_RGB > cached_sum_RGB_max)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (max_abs_RGB_sqr_diff >= 0.0)
                              && (d_RGB_sqr > max_abs_RGB_sqr_diff)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (max_rel_RGB_sqr_diff >= 0.0)
                              && (d_rel_RGB_sqr > max_rel_RGB_sqr_diff)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (max_abs_sum_RGB_diff >= 0.0)
                              && (d_sum_RGB > max_abs_sum_RGB_diff)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (max_rel_sum_RGB_diff >= 0.0)
                              && (sum_RGB > min_sum_RGB_for_relative_sum_diff)
                              && (d_rel_sum_RGB > max_rel_sum_RGB_diff)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (max_abs_chrom_diff >= 0.0)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                              && (    (d_r_chrom > max_abs_chrom_diff)
                                   || (d_g_chrom > max_abs_chrom_diff)
                                 )
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (max_rel_chrom_diff >= 0.0)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                              && (    (d_rel_r_chrom > max_rel_chrom_diff)
                                   || (d_rel_g_chrom > max_rel_chrom_diff)
                                 )
                            )
                    {
                        skip = TRUE;
                    }
#else
                    if (R < cached_R_min)
                    {
                        skip = TRUE;
                    }
                    else if (R > cached_R_max)
                    {
                        skip = TRUE;
                    }
                    else if (G < cached_G_min)
                    {
                        skip = TRUE;
                    }
                    else if (G > cached_G_max)
                    {
                        skip = TRUE;
                    }
                    else if (B < cached_B_min)
                    {
                        skip = TRUE;
                    }
                    else if (B > cached_B_max)
                    {
                        skip = TRUE;
                    }
                    else if (    (r_chrom < cached_r_chrom_min)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (r_chrom > cached_r_chrom_max)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (g_chrom < cached_g_chrom_min)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (g_chrom > cached_g_chrom_max)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (sum_RGB < cached_sum_RGB_min)
                    {
                        skip = TRUE;
                    }
                    else if (sum_RGB > cached_sum_RGB_max)
                    {
                        skip = TRUE;
                    }
                    else if (d_RGB_sqr > max_abs_RGB_sqr_diff)
                    {
                        skip = TRUE;
                    }
                    else if (d_rel_RGB_sqr > max_rel_RGB_sqr_diff)
                    {
                        skip = TRUE;
                    }
                    else if (d_sum_RGB > max_abs_sum_RGB_diff)
                    {
                        skip = TRUE;
                    }
                    else if (    (d_rel_sum_RGB > max_rel_sum_RGB_diff)
                              && (sum_RGB > min_sum_RGB_for_relative_sum_diff)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (d_r_chrom > max_abs_chrom_diff)
                              || (d_g_chrom > max_abs_chrom_diff)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (d_rel_r_chrom > max_rel_chrom_diff)
                              || (d_rel_g_chrom > max_rel_chrom_diff)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
#endif

                    if ( ! skip )
                    {
                        ERE(grow_segment(i_offset, j_offset));
                    }
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int grow_segment_using_rg_var_and_lum_diff(int i, int j)
{
    int   di, dj;
    int   i_offset;
    int   j_offset;
    float cur_R;
    float cur_G;
    float cur_B;
    float cur_sum_RGB;
    Pixel cur_image_pixel;


    cur_image_pixel = cached_ip->pixels[ i ][ j ];

    ASSERT(! (cached_seg_map[ i ][ j ]));
    ASSERT(cur_image_pixel.extra.invalid.pixel == VALID_PIXEL);

    cached_pixels[ num_cached_pixels ].i = i;
    cached_pixels[ num_cached_pixels ].j = j;

    cur_R = cur_image_pixel.r;
    cur_G = cur_image_pixel.g;
    cur_B = cur_image_pixel.b;

    cur_sum_RGB = cur_R + cur_G + cur_B + RGB_SUM_EPSILON;

    cached_R_sum += cur_R;
    cached_G_sum += cur_G;
    cached_B_sum += cur_B;
    cached_i_sum += (double)i;
    cached_j_sum += (double)j;

    num_cached_pixels++;

    if (i > cached_i_max) cached_i_max = i;
    if (i < cached_i_min) cached_i_min = i;
    if (j > cached_j_max) cached_j_max = j;
    if (j < cached_j_min) cached_j_min = j;

    cached_seg_map[ i ][ j ] = cached_segment_count;

    for (di = -1; di <= 1; di++)
    {
        for (dj = -1; dj <= 1; dj++)
        {
            if ((di != 0) || (dj != 0))
            {
                i_offset = i + di;
                j_offset = j + dj;

                if (    (i_offset >= 0)
                     && (j_offset >= 0)
                     && (i_offset < cached_num_rows)
                     && (j_offset < cached_num_cols)
#ifdef CONNECT_CORNER_OPTION
                     && (seg_connect_corners || ((di == 0) || (dj == 0)))
#else
#ifndef DEFAULT_IS_TO_CONNECT_CORNERS
                     && ((di == 0) || (dj == 0))
#endif
#endif
                     && ( ! cached_seg_map[ i_offset ][ j_offset ])
                     && (cached_ip->pixels[ i_offset ][ j_offset ].extra.invalid.pixel == VALID_PIXEL)
                   )
                {
                    int skip = FALSE;

                    float R = cached_ip->pixels[ i_offset ][ j_offset ].r;
                    float G = cached_ip->pixels[ i_offset ][ j_offset ].g;
                    float B = cached_ip->pixels[ i_offset ][ j_offset ].b;
                    float sum_RGB = R + G + B + RGB_SUM_EPSILON;
                    float r_chrom = R / sum_RGB;
                    float g_chrom = G / sum_RGB;
                    float d_sum_RGB = ABS_OF(cur_sum_RGB - sum_RGB);
                    float d_rel_sum_RGB = d_sum_RGB / sum_RGB;

#ifdef USE_NO_VALUE
                    if (    (cached_r_chrom_min >= 0.0)
                         && (r_chrom < cached_r_chrom_min)
                         && (sum_RGB > min_sum_RGB_for_chrom_test)
                       )
                    {
                        skip = TRUE;
                    }
                    else if (    (cached_r_chrom_max >= 0.0)
                              && (r_chrom > cached_r_chrom_max)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (cached_g_chrom_min >= 0.0)
                              && (g_chrom < cached_g_chrom_min)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (cached_g_chrom_max >= 0.0)
                              && (g_chrom > cached_g_chrom_max)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (max_abs_sum_RGB_diff >= 0.0)
                              && (d_sum_RGB > max_abs_sum_RGB_diff)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (max_rel_sum_RGB_diff >= 0.0)
                              && (sum_RGB > min_sum_RGB_for_relative_sum_diff)
                              && (d_rel_sum_RGB > max_rel_sum_RGB_diff)
                            )
                    {
                        skip = TRUE;
                    }
#else
                    if (    (r_chrom < cached_r_chrom_min)
                         && (sum_RGB > min_sum_RGB_for_chrom_test)
                       )
                    {
                        skip = TRUE;
                    }
                    else if (    (r_chrom > cached_r_chrom_max)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (g_chrom < cached_g_chrom_min)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (g_chrom > cached_g_chrom_max)
                              && (sum_RGB > min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (d_sum_RGB > max_abs_sum_RGB_diff)
                    {
                        skip = TRUE;
                    }
                    else if (    (d_rel_sum_RGB > max_rel_sum_RGB_diff)
                              && (sum_RGB > min_sum_RGB_for_relative_sum_diff)
                            )
                    {
                        skip = TRUE;
                    }
#endif

                    if ( ! skip )
                    {
                        ERE(grow_segment_using_rg_var_and_lum_diff(i_offset,
                                                                   j_offset));
                    }
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
// Note: We currently include holes which are invalid pixels.
*/
static void fill_holes(void)
{
    int i, j, di, dj;
    int surround_count;
    Pixel pixel;


    pixel.extra.invalid.r = VALID_PIXEL;
    pixel.extra.invalid.g = VALID_PIXEL;
    pixel.extra.invalid.b = VALID_PIXEL;
    pixel.extra.invalid.pixel = VALID_PIXEL;

    for (i = cached_i_min + 1; i < cached_i_max - 1; i++)
    {
        for (j = cached_j_min + 1; j < cached_j_max - 1; j++)
        {
            if (cached_seg_map[ i ][ j ] != cached_segment_count)
            {
                surround_count = 0;

                for (di=-1; di<=1; di++)
                {
                    for (dj=-1; dj<=1; dj++)
                    {
                        if ((di != 0) || (dj != 0))
                        {
                            int i_offset = i + di;
                            int j_offset = j + dj;

                            if (cached_seg_map[ i_offset ][ j_offset ] == cached_segment_count)
                            {
                                surround_count++;
                            }
                        }
                    }
                }

                if (surround_count > 4)
                {
                    float R = 0.0;
                    float G = 0.0;
                    float B = 0.0;

                    surround_count = 0;

                    for (di=-1; di<=1; di++)
                    {
                        for (dj=-1; dj<=1; dj++)
                        {
                            if ((di != 0) || (dj != 0))
                            {
                                int i_offset = i + di;
                                int j_offset = j + dj;

                                if (cached_seg_map[ i_offset ][ j_offset ] == cached_segment_count)
                                {
                                    R += cached_ip->pixels[ i_offset ][ j_offset ].r;
                                    G += cached_ip->pixels[ i_offset ][ j_offset ].g;
                                    B += cached_ip->pixels[ i_offset ][ j_offset ].b;

                                    surround_count++;
                                }
                            }
                        }
                    }

                    cached_seg_map[ i][j ] = cached_segment_count;

                    R /= surround_count;
                    pixel.r = R;

                    G /= surround_count;
                    pixel.g = G;

                    B /= surround_count;
                    pixel.b = B;

                    cached_ip->pixels[ i ][ j ] = pixel;

                    cached_pixels[ num_cached_pixels ].i = i;
                    cached_pixels[ num_cached_pixels ].j = j;

                    num_cached_pixels++;

                    cached_R_sum += R;
                    cached_G_sum += G;
                    cached_B_sum += B;

                    cached_i_sum += (double)i;
                    cached_j_sum += (double)j;
                }
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_boundary_pixels(Segmentation_t1* image_segment_info_ptr)
{
    int   num_rows     = image_segment_info_ptr->num_rows;
    int   num_cols     = image_segment_info_ptr->num_cols;
    int   num_segments = image_segment_info_ptr->num_segments;
    int   k, i, j, di, dj;
    int   seg_num;
    int** seg_map;


    update_seg_map(image_segment_info_ptr);
    seg_map = image_segment_info_ptr->seg_map;

    for (seg_num = 1; seg_num <= num_segments; seg_num++)
    {
        Segment_t1* cur_seg_ptr = image_segment_info_ptr->segments[ seg_num - 1 ];
        int            num_pixels  = cur_seg_ptr->num_pixels;
        int            count = 0;

        for (k = 0; k<num_pixels; k++)
        {
            i = cur_seg_ptr->pixels[ k ].i;
            j = cur_seg_ptr->pixels[ k ].j;

            if (    (i == 0)
                 || (j == 0)
                 || (i == num_rows - 1)
                 || (j == num_cols - 1)
               )
            {
                 cached_pixels[ count ] = cur_seg_ptr->pixels[ k ];
                 count++;
            }
            else
            {
                for (di=-1; di<=1; di++)
                {
                    for (dj=-1; dj<=1; dj++)
                    {
                        int i_offset = i + di;
                        int j_offset = j + dj;

                        if (seg_map[ i_offset ][ j_offset ] != seg_num)
                        {
                             cached_pixels[ count ] = cur_seg_ptr->pixels[ k ];
                             count++;
                        }
                    }
                }
            }
        }

        kjb_free(cur_seg_ptr->boundary_pixels);
        NRE(cur_seg_ptr->boundary_pixels = N_TYPE_MALLOC(Pixel_info, count));

        for (k = 0; k<count; k++)
        {
            cur_seg_ptr->boundary_pixels[ k ] = cached_pixels[ k ];
        }
        cur_seg_ptr->num_boundary_pixels = count;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef IMPLEMENT_MERGING

/*
// This is not finished yet!
*/
static int merge_segments(Segmentation_t1* image_segment_info_ptr)
{
    static int max_num_segments  = 0;
    int        num_rows          = image_segment_info_ptr->num_rows;
    int        num_cols          = image_segment_info_ptr->num_cols;
    int        num_segments      = image_segment_info_ptr->num_segments;
    int**      seg_map           = image_segment_info_ptr->seg_map;
    Segment_t1**  image_segment_ptr = image_segment_info_ptr->segments;
    int        i, j;
    int        count;
    long       cpu_time = get_cpu_time();
    int        seg_num;
    int        nb_num;


    if (num_segments < 2) return num_segments;

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (num_segments > max_num_segments)
    {
        verbose_pso(10, "Bumping up storage for merging.\n");

        free_2D_int_array(merge_counts);
        kjb_free(segment_indices_after_merging);

        NRE(merge_counts = allocate_2D_int_array(num_segments, num_segments));

        ERE(get_target_matrix(&merge_r_diff_mp, num_segments, num_segments));
        ERE(get_target_matrix(&merge_g_diff_mp, num_segments, num_segments));
        ERE(get_target_matrix(&merge_sum_diff_mp, num_segments, num_segments));

        NRE(segment_indices_after_merging = INT_MALLOC(num_segments));

        max_num_segments = num_segments;
    }

    /* Initilize merge chart to zeros. We only use one of the upper right
    // triangle of the chart, but it is easiest to initialize the whole thin. */

    for (i=0; i<num_segments; i++)
    {
        segment_indices_after_merging[ i ] = i;

        for (j=0; j<num_segments; j++)
        {
            merge_counts[ i ][ j ] = 0;
            merge_r_diff_mp->elements[ i ][ j ] = 0.0;
            merge_g_diff_mp->elements[ i ][ j ] = 0.0;
            merge_sum_diff_mp->elements[ i ][ j ] = 0.0;
        }
    }

    /*
    // Fill marked chart with all boundary points.
    */
    for (i=0; i<num_segments; i++)
    {
        int num_boundary_pixels = image_segment_ptr[ i ]->num_boundary_pixels;
        Pixel_info* boundary_pixels = image_segment_ptr[ i ]->boundary_pixels;

        seg_num = i + 1;
        ASSERT(seg_num == image_segment_ptr[ i ]->segment_number);

        for (j=0; j<num_boundary_pixels; j++)
        {
            int i_b = boundary_pixels[ j ].i;
            int j_b = boundary_pixels[ j ].j;

            seg_map[ i_b ][ j_b ] = seg_num;
        }
    }

    /*
    // Fill merge chart.  We increment bins to count the number of adjacent
    // points. If an adjacent point difference is greater than the threshold,
    // then we decrement the chart location by a fairy large number times a
    // function of the ratio of the difference to the tolerance.  Thus we can
    // establish an allowable tolerance of noise. Normally we simply need all
    // differences to be sub-tolerance, and a certain number of adjacent points
    */
    for (i=0; i<num_segments; i++)
    {
        int num_boundary_pixels = image_segment_ptr[ i ]->num_boundary_pixels;
        Pixel_info* boundary_pixels = image_segment_ptr[ i ]->boundary_pixels;

        seg_num = i + 1;
        ASSERT(seg_num == image_segment_ptr[ i ]->segment_number);

        verbose_pso(10, "Region %d has %d boundary pixels.\n", seg_num,
                    num_boundary_pixels);

        for (j=0; j<num_boundary_pixels; j++)
        {
            int i_b = boundary_pixels[ j ].i;
            int j_b = boundary_pixels[ j ].j;
            int di, dj;

#ifdef CONNECTION_MAX_STEP_OPTION
            for (di = -connection_max_step; di <= connection_max_step; di++)
#else
            for (di = -1; di <= 1; di++)
#endif
            {
#ifdef CONNECTION_MAX_STEP_OPTION
                for (dj = -connection_max_step; dj <= connection_max_step; dj++)
#else
                for (dj = -1; dj <= 1; dj++)
#endif
                {
                    int i_offset = i_b + di;
                    int j_offset = j_b + dj;

                    if (    (i_offset >= 0)
                         && (j_offset >= 0)
                         && (i_offset < num_rows)
                         && (j_offset < num_cols)
#ifdef CONNECT_CORNER_OPTION
                         && (seg_connect_corners || ((di == 0) || (dj == 0)))
#else
#ifndef DEFAULT_IS_TO_CONNECT_CORNERS
                         && ((di == 0) || (dj == 0))
#endif
#endif
                         && (seg_map[ i_offset ][ j_offset ] != seg_num)
                         && ((nb_num = seg_map[ i_offset ][ j_offset ]) != 0)
                       )
                    {
                        float b_R, b_G, b_B, b_r, b_g, b_sum;
                        float n_R, n_G, n_B, n_r, n_g, n_sum;
                        double r_diff, g_diff, sum_diff;
                        int b_i = boundary_pixels[ j ].i;
                        int b_j = boundary_pixels[ j ].j;

                        b_R = cached_ip->pixels[ b_i ][ b_j ].r;
                        b_G = cached_ip->pixels[ b_i ][ b_j ].g;
                        b_B = cached_ip->pixels[ b_i ][ b_j ].b;

                        b_sum = b_R + b_G + b_B + RGB_SUM_EPSILON;
                        b_r = b_R / b_sum;
                        b_g = b_G / b_sum;

                        n_R = cached_ip->pixels[ i_offset ][ j_offset ].r;
                        n_G = cached_ip->pixels[ i_offset ][ j_offset ].g;
                        n_B = cached_ip->pixels[ i_offset ][ j_offset ].b;
                        n_sum = n_R + n_G + n_B + RGB_SUM_EPSILON;
                        n_r = n_R / n_sum;
                        n_g = n_G / n_sum;

                        r_diff = b_r - n_r;
                        g_diff = b_g - n_g;
                        sum_diff = b_sum - n_sum;

                        merge_r_diff_mp->elements[ i ][ nb_num - 1 ] += r_diff*r_diff;
                        merge_g_diff_mp->elements[ i ][ nb_num - 1 ] += g_diff*g_diff;
                        merge_sum_diff_mp->elements[ i ][ nb_num - 1 ] += sum_diff*sum_diff;

                        (merge_counts[ i ][ nb_num - 1 ])++;
                    }
                }
            }
        }
    }

    for (i=0; i<num_segments; i++)
    {
        for (j = i+1; j<num_segments; j++)
        {
            count = merge_counts[ i ][ j ];

            if (count > merge_min_num_pixels)
            {
                double r_diff = merge_r_diff_mp->elements[ i ][ j ];
                double r_test = sqrt(r_diff / (double)count);
                double g_diff = merge_g_diff_mp->elements[ i ][ j ];
                double g_test = sqrt(g_diff / (double)count);
                double sum_diff = merge_sum_diff_mp->elements[ i ][ j ];
                double sum_test = sqrt(sum_diff / (double)count);

                if (    (r_test < merge_rg_threshold)
                     && (g_test < merge_rg_threshold)
                     && (sum_test < merge_sum_RGB_abs_threshold)
                   )
                {
                    verbose_pso(10, "Merging regions %d and %d ", i+1, j+1);
                    verbose_pso(10, "(connectivity count is %d).\n",
                                count);

                    ERE(merge_segment_pair(i, j, image_segment_info_ptr));
                }
            }
        }
    }

    count = 0;

    for (i=0; i<num_segments; i++)
    {
        if (image_segment_info_ptr->segments[ i ] != NULL)
        {
            image_segment_info_ptr->segments[ count ] =
                                        image_segment_info_ptr->segments[ i ];
            image_segment_info_ptr->segments[count]->segment_number = count + 1;
            count++;
        }
    }

    dbi(count);
    image_segment_info_ptr->num_segments = count;

    /*
    // Compact segment list.
    */

    verbose_pso(5, "Merge took %ldms.\n", get_cpu_time() - cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int merge_segment_pair
(
    int           i,
    int           j,
    Segmentation_t1* image_segment_info_ptr
)
{
    int      count       = 0;
    int      k, ni, nj;
    int      temp;
    Segment_t1* seg_ptr;
    Segment_t1* nb_ptr;
    Segment_t1* new_seg_ptr;

    /*
    // We have with a list of segment pointers and segment numbers, indexed by
    // segments. As a segment gets merged, its pointer becomes null, and its
    // number becomes the segment number it was merged with.
    //
    // Thus, to merge X with Y, we find Y by chasing the indices arround, until
    // we get a segment number equal to its index. Call this Z. Then the number
    // for Z gets set to X, the pointer gets set to NULL, and the segment gets
    // added to X's stuff.
    */

#ifdef TEST
    for (nj = 0; nj < MIN_OF(25, image_segment_info_ptr->num_segments); nj++)
    {
        verbose_pso(20, "%2d ", segment_indices_after_merging[ nj ] + 1);
    }
    verbose_pso(20, "\n");
#endif

    while ((ni = segment_indices_after_merging[ i ]) != i)
    {
        i = ni;

        count++;

        if (count > 10000)
        {
            set_bug("Likely infinite loop in merge_segment_pair.");
            return ERROR;
        }
    }

    count = 0;

    while ((nj = segment_indices_after_merging[ j ]) != j)
    {
        j = nj;

        count++;

        if (count > 10000)
        {
            set_bug("Likely infinite loop in merge_segment_pair.");
            return ERROR;
        }
    }

    verbose_pso(10, "Merging relabeled regions %d and %d.\n", i+1, j+1);

    if (i == j)
    {
        verbose_pso(10, "Regions are already merge. Skipping.\n");
        return NO_ERROR;
    }

    if (i > j)
    {
        temp = i;
        i = j;
        j = temp;
    }

    seg_ptr = image_segment_info_ptr->segments[ i ];
    nb_ptr = image_segment_info_ptr->segments[ j ];

    num_cached_pixels = seg_ptr->num_pixels + nb_ptr->num_pixels;

    cached_R_sum = seg_ptr->R_ave * seg_ptr->num_pixels;
    cached_R_sum += nb_ptr->R_ave * nb_ptr->num_pixels;

    cached_G_sum = seg_ptr->G_ave * seg_ptr->num_pixels;
    cached_G_sum += nb_ptr->G_ave * nb_ptr->num_pixels;

    cached_B_sum = seg_ptr->B_ave * seg_ptr->num_pixels;
    cached_B_sum += nb_ptr->B_ave * nb_ptr->num_pixels;

    cached_i_sum = seg_ptr->i_CM * seg_ptr->num_pixels;
    cached_i_sum += nb_ptr->i_CM * nb_ptr->num_pixels;

    cached_j_sum = seg_ptr->j_CM * seg_ptr->num_pixels;
    cached_j_sum += nb_ptr->j_CM * nb_ptr->num_pixels;

    cached_i_min = MIN_OF(seg_ptr->i_min, nb_ptr->i_min);
    cached_j_min = MIN_OF(seg_ptr->j_min, nb_ptr->j_min);
    cached_i_max = MAX_OF(seg_ptr->i_max, nb_ptr->i_max);
    cached_j_max = MAX_OF(seg_ptr->j_max, nb_ptr->j_max);

    count = 0;

    for (k = 0; k < seg_ptr->num_pixels; k++)
    {
        cached_pixels[ count ] = seg_ptr->pixels[ k ];
        count++;
    }

    for (k = 0; k < nb_ptr->num_pixels; k++)
    {
        cached_pixels[ count ] = nb_ptr->pixels[ k ];
        count++;
    }

    segment_indices_after_merging[ j ] = i;

    NRE(new_seg_ptr = create_image_segment(i + 1));

    free_image_segment(nb_ptr);
    free_image_segment(seg_ptr);

    image_segment_info_ptr->segments[ j ] = NULL;
    image_segment_info_ptr->segments[ i ] = new_seg_ptr;

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_outside_boundary(Segmentation_t1* image_segment_info_ptr)
{
    Segment_t1** segments     = image_segment_info_ptr->segments;
    int       num_segments = image_segment_info_ptr->num_segments;
    int       num_rows     = image_segment_info_ptr->num_rows;
    int       num_cols     = image_segment_info_ptr->num_cols;
    int**     mag_seg_map;
    int       mag_num_rows = 2 * num_rows;
    int       mag_num_cols = 2 * num_cols;
    int       j, k;
    long      cpu_time = get_cpu_time();
    int       seg_num;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    ERE(get_zero_int_matrix(&cached_magnified_seg_map, mag_num_rows,
                            mag_num_cols));
    mag_seg_map = cached_magnified_seg_map->elements;

    /*
    // First pass: Mark all boundary points.
    */
    for (seg_num = 1; seg_num <= num_segments; seg_num++)
    {
        Segment_t1* cur_seg_ptr         = segments[ seg_num - 1 ];
        int            num_boundary_pixels = cur_seg_ptr->num_boundary_pixels;
        Pixel_info*    boundary_pixels     = cur_seg_ptr->boundary_pixels;

        for (j=0; j<num_boundary_pixels; j++)
        {
            int i_b = boundary_pixels[ j ].i;
            int j_b = boundary_pixels[ j ].j;

            mag_seg_map[ 2 * i_b ][ 2 * j_b ] = seg_num;
            mag_seg_map[ 2 * i_b ][ 2 * j_b + 1] = seg_num;
            mag_seg_map[ 2 * i_b + 1][ 2 * j_b ] = seg_num;
            mag_seg_map[ 2 * i_b + 1][ 2 * j_b + 1] = seg_num;
        }
    }

    /*
    // Second pass: For each segment, approach top to bottem through CM.
    // When we hit it, follow the boundary to get the outside boundary.
    */
    for (seg_num = 1; seg_num <= num_segments; seg_num++)
    {
        Segment_t1* cur_seg_ptr = segments[ seg_num - 1 ];
        int            start_j     = (int)(2.0 * cur_seg_ptr->j_CM);
        int            ii, jj;
        int            start_i     = NOT_SET;
        int            valid_boundary      = TRUE;
        int            count = 0;
        Boundary_approach approach = SEG_DOWN;
        int end_i, end_j;
        int ni, si, ei, nj, sj, ej;
        int prev_ii, prev_jj;


        for (ii = 0; ii < mag_num_rows; ii++)
        {
            if (mag_seg_map[ ii ][ start_j ] == seg_num)
            {
                start_i = ii;
                break;
            }
        }

        if (start_i == NOT_SET)
        {
            verbose_pso(20, "Unable to find start of outside boundary for segment %d.\n",
                        seg_num);
        }
        else
        {
            ii = start_i;
            jj = start_j;

            end_i = start_i;
            end_j = start_j;

            /*
            // Go in the opposite way that we plan to go for the boundary do
            // find out when to stop. Simply stopping when we get to where we
            // started does not work because we sometings have to pass the
            // start point.
            */

            /* Go down */ /* dbm("END: Down");  */

            si = MAX_OF(0, ii - 1);
            nj = jj + 1;

            if (nj >= 0)
            {
                ei = MIN_OF(mag_num_rows - 1, ii + 1);

                ASSERT(si <= ei);
                for (ni = si; ni <= ei; ni++)
                {
                    ASSERT(    (ni >= 0) && (nj >= 0)
                            && (ni < mag_num_rows)
                            && (nj < mag_num_cols));

                    verbose_pso(200, "E==>(%3d, %3d)   ", ni, nj);
                    if (mag_seg_map[ ni ][ nj ] == seg_num)
                    {
                        verbose_pso(200, "found\n");

                        end_i = ni;
                        end_j = nj;
                        goto end_pixel_found;
                    }
                    verbose_pso(200, "\n");
                }
            }

            /* Pixel opposite */ /* dbm("END: Pixel opposite"); */

            ni = ii + 1;
            nj = jj;

            if (ni < mag_num_rows)
            {
                ASSERT(    (ni >= 0) && (nj >= 0)
                        && (ni < mag_num_rows)
                        && (nj < mag_num_cols));

                verbose_pso(200, "E==>(%3d, %3d)   ", ni, nj);
                if (mag_seg_map[ ni ][ nj ] == seg_num)
                {
                    verbose_pso(200, "found\n");

                    end_i = ni;
                    end_j = nj;
                    goto end_pixel_found;
                }
                verbose_pso(200, "\n");
            }

            /* Go up  */ /* dbm("END: Up");  */

            si = MIN_OF(mag_num_rows - 1, ii + 1);
            nj = jj - 1;

            if (nj < mag_num_cols)
            {
                ei = MAX_OF(0, ii - 1);

                ASSERT(si >= ei);
                for (ni = si; ni >= ei; ni--)
                {
                    ASSERT(    (ni >= 0) && (nj >= 0)
                            && (ni < mag_num_rows)
                            && (nj < mag_num_cols));

                    verbose_pso(200, "E==>(%3d, %3d)   ", ni, nj);
                    if (mag_seg_map[ ni ][ nj ] == seg_num)
                    {
                        verbose_pso(200, "found\n");

                        end_i = ni;
                        end_j = nj;
                        goto end_pixel_found;
                    }
                    verbose_pso(200, "\n");
                }
            }

end_pixel_found:

            /* dbi(end_i);  */
            /* dbi(end_j);  */

            ii = start_i;
            jj = start_j;

            do
            {
                /* dbi(ii); */
                /* dbi(jj); */

                if (approach == SEG_DOWN)
                {

                    /* Go down */ /* dbm("Down");  */

                    si = MAX_OF(0, ii - 1);
                    nj = jj - 1;

                    if (nj >= 0)
                    {
                        ei = MIN_OF(mag_num_rows - 1, ii + 1);

                        ASSERT(si <= ei);
                        for (ni = si; ni <= ei; ni++)
                        {
                            ASSERT(    (ni >= 0) && (nj >= 0)
                                    && (ni < mag_num_rows)
                                    && (nj < mag_num_cols));

                            verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(200, "found\n");
                                if (ni == ii - 1) approach = SEG_LEFT;
                                goto add_pixel;
                            }
                            verbose_pso(200, "\n");
                        }
                    }

                    /* Pixel opposite */ /* dbm("Pixel opposite"); */

                    ni = ii + 1;
                    nj = jj;

                    if (ni < mag_num_rows)
                    {
                        ASSERT(    (ni >= 0) && (nj >= 0)
                                && (ni < mag_num_rows)
                                && (nj < mag_num_cols));

                        verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                        if (mag_seg_map[ ni ][ nj ] == seg_num)
                        {
                            approach = SEG_RIGHT;
                            verbose_pso(200, "found\n");
                            goto add_pixel;
                        }
                        verbose_pso(200, "\n");
                    }

                    /* Go up  */ /* dbm("Up");  */

                    si = MIN_OF(mag_num_rows - 1, ii + 1);
                    nj = jj + 1;

                    if (nj < mag_num_cols)
                    {
                        ei = MAX_OF(0, ii - 1);

                        ASSERT(si >= ei);
                        for (ni = si; ni >= ei; ni--)
                        {
                            ASSERT(    (ni >= 0) && (nj >= 0)
                                    && (ni < mag_num_rows)
                                    && (nj < mag_num_cols));

                            verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(200, "found\n");

                                if (ni == ii + 1)
                                {
                                    approach = SEG_RIGHT;
                                }
                                else
                                {
                                    approach = SEG_UP;
                                }

                                goto add_pixel;
                            }
                            verbose_pso(200, "\n");
                        }
                    }
                }
                else if (approach == SEG_UP)
                {
                    /* Go up */ /* dbm("Up");  */

                    si = MIN_OF(mag_num_rows - 1, ii + 1);
                    nj = jj + 1;

                    if (nj < mag_num_cols)
                    {
                        ei = MAX_OF(0, ii - 1);

                        ASSERT(si >= ei);
                        for (ni = si; ni >= ei; ni--)
                        {
                            ASSERT(    (ni >= 0) && (nj >= 0)
                                    && (ni < mag_num_rows)
                                    && (nj < mag_num_cols));

                            verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(200, "found\n");
                                if (ni == ii + 1) approach = SEG_RIGHT;
                                goto add_pixel;
                            }
                            verbose_pso(200, "\n");
                        }
                    }

                    /* Opposite Pixel */ /* dbm("Opposite Pixel"); */

                    ni = ii - 1;
                    nj = jj;

                    if (ni >= 0)
                    {
                        ASSERT(    (ni >= 0) && (nj >= 0)
                                && (ni < mag_num_rows)
                                && (nj < mag_num_cols));

                        verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                        if (mag_seg_map[ ni ][ nj ] == seg_num)
                        {
                            verbose_pso(200, "found\n");
                            approach = SEG_LEFT;
                            goto add_pixel;
                        }
                        verbose_pso(200, "\n");
                    }

                    /* Go down  */ /* dbm("Down");  */

                    si = MAX_OF(0, ii - 1);
                    nj = jj - 1;

                    if (nj >= 0)
                    {
                        ei = MIN_OF(mag_num_rows - 1, ii + 1);

                        ASSERT(si <= ei);
                        for (ni = si; ni <= ei; ni++)
                        {
                            ASSERT(    (ni >= 0) && (nj >= 0)
                                    && (ni < mag_num_rows)
                                    && (nj < mag_num_cols));

                            verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(200, "found\n");

                                if (ni == ii - 1)
                                {
                                    approach = SEG_LEFT;
                                }
                                else
                                {
                                    approach = SEG_DOWN;
                                }

                                goto add_pixel;
                            }
                            verbose_pso(200, "\n");
                        }
                    }
                }
                else if (approach == SEG_RIGHT)
                {
                    /* Go right */ /* dbm("Right");  */

                    sj = MAX_OF(0, jj - 1);
                    ni = ii + 1;

                    if (ni < mag_num_rows)
                    {
                        ej = MIN_OF(mag_num_cols - 1, jj + 1);

                        ASSERT(sj <= ej);
                        for (nj = sj; nj <= ej; nj++)
                        {
                            ASSERT(    (nj >= 0) && (ni >= 0)
                                    && (nj < mag_num_cols)
                                    && (ni < mag_num_rows));

                            verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(200, "found\n");
                                if (nj == jj - 1) approach = SEG_DOWN;
                                goto add_pixel;
                            }
                            verbose_pso(200, "\n");
                        }
                    }

                    /* Pixel opposite */ /* dbm("Pixel opposite"); */

                    nj = jj + 1;
                    ni = ii;

                    if (nj < mag_num_cols)
                    {
                        ASSERT(    (nj >= 0) && (ni >= 0)
                                && (nj < mag_num_cols)
                                && (ni < mag_num_rows));

                        verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                        if (mag_seg_map[ ni ][ nj ] == seg_num)
                        {
                            verbose_pso(200, "found\n");
                            approach = SEG_UP;
                            goto add_pixel;
                        }
                        verbose_pso(200, "\n");
                    }

                    /* Go left  */ /* dbm("Left");  */

                    sj = MIN_OF(mag_num_cols - 1, jj + 1);
                    ni = ii - 1;

                    if (ni >= 0)
                    {
                        ej = MAX_OF(0, jj - 1);

                        ASSERT(sj >= ej);
                        for (nj = sj; nj >= ej; nj--)
                        {
                            ASSERT(    (nj >= 0) && (ni >= 0)
                                    && (nj < mag_num_cols)
                                    && (ni < mag_num_rows));

                            verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(200, "found\n");

                                if (nj == jj + 1)
                                {
                                    approach = SEG_UP;
                                }
                                else
                                {
                                    approach = SEG_LEFT;
                                }

                                goto add_pixel;
                            }
                            verbose_pso(200, "\n");
                        }
                    }
                }
                else if (approach == SEG_LEFT)
                {
                    /* Go left */ /* dbm("Left");  */

                    sj = MIN_OF(mag_num_cols - 1, jj + 1);
                    ni = ii - 1;

                    if (ni >= 0)
                    {
                        ej = MAX_OF(0, jj - 1);

                        ASSERT(sj >= ej);
                        for (nj = sj; nj >= ej; nj--)
                        {
                            ASSERT(    (nj >= 0) && (ni >= 0)
                                    && (nj < mag_num_cols)
                                    && (ni < mag_num_rows));

                            verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(200, "found\n");
                                if (nj == jj + 1) approach = SEG_UP;
                                goto add_pixel;
                            }
                            verbose_pso(200, "\n");
                        }
                    }

                    /* Opposite Pixel */ /* dbm("Opposite Pixel"); */

                    nj = jj - 1;
                    ni = ii;

                    if (nj >= 0)
                    {
                        ASSERT(    (nj >= 0) && (ni >= 0)
                                && (nj < mag_num_cols)
                                && (ni < mag_num_rows));

                        verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                        if (mag_seg_map[ ni ][ nj ] == seg_num)
                        {
                            verbose_pso(200, "found\n");
                            approach = SEG_DOWN;
                            goto add_pixel;
                        }
                        verbose_pso(200, "\n");
                    }

                    /* Go right  */ /* dbm("Right");  */

                    sj = MAX_OF(0, jj - 1);
                    ni = ii + 1;

                    if (ni < mag_num_rows)
                    {
                        ej = MIN_OF(mag_num_cols - 1, jj + 1);

                        ASSERT(sj <= ej);
                        for (nj = sj; nj <= ej; nj++)
                        {
                            ASSERT(    (nj >= 0) && (ni >= 0)
                                    && (nj < mag_num_cols)
                                    && (ni < mag_num_rows));

                            verbose_pso(200, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(200, "found\n");

                                if (nj == jj - 1)
                                {
                                    approach = SEG_DOWN;
                                }
                                else
                                {
                                    approach = SEG_RIGHT;
                                }

                                goto add_pixel;
                            }
                            verbose_pso(200, "\n");
                        }
                    }

                }

                verbose_pso(20, "Unable to follow outside boundary ");
                verbose_pso(20, "of segment %d.\n", seg_num);
                valid_boundary = FALSE;

                break;

add_pixel:

                if (count >= 4 * cur_seg_ptr->num_boundary_pixels)
                {
                    verbose_pso(20, "Max length of outside boundary exceeded ");
                    verbose_pso(20, "for segment %d.\n", seg_num);
                    valid_boundary = FALSE;

                    break;
                }

                /* dbi(ni); */
                /* dbi(nj);  */

                /* Add to cached pixels. */

                ASSERT(count < mag_num_rows * mag_num_cols);
                ASSERT(count >= 0);

                cached_pixels[ count ].i = ni;
                cached_pixels[ count ].j = nj;

                count++;

                prev_ii = ii;
                prev_jj = jj;

                ii = ni;
                jj = nj;
            }
            while ((ii != start_i) || (jj != start_j) || (prev_ii != end_i) || (prev_jj != end_j));

            if (valid_boundary)
            {
                ERE(get_target_matrix(&(cur_seg_ptr->outside_boundary_mp),
                                      count, 2));

                for (k = 0; k<count; k++)
                {
                    double x = (double)cached_pixels[ k ].i / 2.0;
                    int  b_i = kjb_rint(x);
                    double y = (double)cached_pixels[ k ].j / 2.0;
                    int  b_j = kjb_rint(y);

                    cur_seg_ptr->outside_boundary_mp->elements[ k ][ 0 ] = b_i;
                    cur_seg_ptr->outside_boundary_mp->elements[ k ][ 1 ] = b_j;
                }
            }
        }
    }

    verbose_pso(5, "Finding outside boundary took %ldms.\n",
                get_cpu_time() - cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_neighbours(Segmentation_t1* image_segment_info_ptr)
{
    static int  prev_num_segments = 0;
    int**       seg_map;
    int         num_segments = image_segment_info_ptr->num_segments;
    int         num_rows     = image_segment_info_ptr->num_rows;
    int         num_cols     = image_segment_info_ptr->num_cols;
    Segment_t1** image_segment_ptr;
    int             seg_num, j, k;
    long            cpu_time = get_cpu_time();


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (num_segments > prev_num_segments)
    {
        verbose_pso(10, "Bumping up storage for neighbours.\n");

        kjb_free(cached_neighbours);
        NRE(cached_neighbours = INT_MALLOC(num_segments));
        prev_num_segments = num_segments;
    }

    zero_seg_map(image_segment_info_ptr);

    seg_map = image_segment_info_ptr->seg_map;
    image_segment_ptr = image_segment_info_ptr->segments;

    /*
    // First pass: Mark all boundary points.
    */
    for (seg_num = 1; seg_num <= num_segments; seg_num++)
    {
        int num_boundary_pixels = image_segment_ptr[ seg_num - 1 ]->num_boundary_pixels;
        Pixel_info* boundary_pixels = image_segment_ptr[ seg_num - 1 ]->boundary_pixels;

        for (j=0; j<num_boundary_pixels; j++)
        {
            int i_b = boundary_pixels[ j ].i;
            int j_b = boundary_pixels[ j ].j;

            seg_map[ i_b ][ j_b ] = seg_num; /* Segment_t1 number */
        }
    }

    /*
    // Second pass: For each boundary point, see if it is near another segment.
    */
    for (seg_num=1; seg_num <= num_segments; seg_num++)
    {
        int num_boundary_pixels = image_segment_ptr[ seg_num - 1 ]->num_boundary_pixels;
        Pixel_info* boundary_pixels = image_segment_ptr[ seg_num - 1 ]->boundary_pixels;
        int*        cur_neighbour_ptr   = cached_neighbours;
        int         num_neighbours      = 0;

        for (j=0; j<num_boundary_pixels; j++)
        {
            int i_b = boundary_pixels[ j ].i;
            int j_b = boundary_pixels[ j ].j;
            int di, dj;
            int new_nb;

#ifdef CONNECTION_MAX_STEP_OPTION
            for (di = -connection_max_step; di <= connection_max_step; di++)
#else
            for (di = -1; di <= 1; di++)
#endif
            {
#ifdef CONNECTION_MAX_STEP_OPTION
                for (dj = -connection_max_step; dj <= connection_max_step; dj++)
#else
                for (dj = -1; dj <= 1; dj++)
#endif
                {
                    int i_offset = i_b + di;
                    int j_offset = j_b + dj;

                    if (    (i_offset >= 0)
                         && (j_offset >= 0)
                         && (i_offset < num_rows)
                         && (j_offset < num_cols)
                         && (seg_map[ i_offset ][ j_offset ] != seg_num)
                         && ((new_nb = seg_map[ i_offset ][ j_offset ]) != 0)
                       )
                    {
                        int skip = FALSE;

                        for (k=0; k<num_neighbours; k++)
                        {
                            if (new_nb == cached_neighbours[ k ])
                            {
                                skip = TRUE;
                                break;
                            }
                        }

                        if (! skip)
                        {
                            *cur_neighbour_ptr = new_nb;
                            cur_neighbour_ptr++;
                            num_neighbours++;
                        }
                    }
                }
            }
        }

        if (num_neighbours > 0)
        {
            verbose_pso(120, "seg %3d : ", seg_num);

            NRE(image_segment_ptr[ seg_num - 1 ]->neighbours = INT_MALLOC(num_neighbours));

            for (k = 0; k < num_neighbours; k++)
            {
                image_segment_ptr[ seg_num - 1 ]->neighbours[ k ]= cached_neighbours[ k ];

                verbose_pso(120, "%d, ", cached_neighbours[ k ]);
            }

            image_segment_ptr[ seg_num - 1 ]->num_neighbours = num_neighbours;

            verbose_pso(120, "\n");
        }
    }

    verbose_pso(5, "Finding neigbours took %ldms.\n",
                get_cpu_time() - cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_interior_points(Segmentation_t1* image_segment_info_ptr)
{
    Segment_t1** segments     = image_segment_info_ptr->segments;
    int             num_segments = image_segment_info_ptr->num_segments;
    int**           seg_map;
    int             i, j;
    long            cpu_time     = get_cpu_time();
    int             seg_num;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif
    zero_seg_map(image_segment_info_ptr);
    seg_map = image_segment_info_ptr->seg_map;

    /*
    // First pass: Mark all region points.
    */
    for (seg_num = 1; seg_num <= num_segments; seg_num++)
    {
        Segment_t1* cur_seg_ptr = segments[ seg_num - 1 ];
        int            num_pixels  = cur_seg_ptr->num_pixels;
        Pixel_info*    pixels      = cur_seg_ptr->pixels;

        for (j=0; j<num_pixels; j++)
        {
            int i_b = pixels[ j ].i;
            int j_b = pixels[ j ].j;

            seg_map[ i_b ][ j_b ] = seg_num;
        }
    }

    /*
    // Second pass: For each segment, approach top to bottom through CM.
    // When we hit it, follow the boundary to get the outside boundary.
    */
    for (seg_num = 1; seg_num <= num_segments; seg_num++)
    {
        Segment_t1* cur_seg_ptr = segments[ seg_num - 1 ];
        int            j_inside    = (int)cur_seg_ptr->j_CM;
        int            start_i     = NOT_SET;
        int            end_i;


        cur_seg_ptr->j_inside = j_inside;

        for (i = 0; i < cached_num_rows; i++)
        {
            if (seg_map[ i ][ j_inside ] == seg_num)
            {
                start_i = i;
                break;
            }
        }

        if (start_i == NOT_SET)
        {
            verbose_pso(20, "Unable to find interior point for segment %d.\n",
                        seg_num);
            cur_seg_ptr->i_inside = (int)cur_seg_ptr->i_CM;
        }
        else
        {
            end_i = cached_num_rows - 1;

            for (i = start_i; i < cached_num_rows; i++)
            {
                if (seg_map[ i ][ j_inside ] != seg_num)
                {
                    end_i = i;
                    break;
                }
            }

            cur_seg_ptr->i_inside = (end_i + start_i) / 2;
        }
    }

    verbose_pso(5, "Finding inside points took %ldms.\n",
                get_cpu_time() - cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void zero_seg_map(Segmentation_t1* image_segment_info_ptr)
{
    int num_rows = image_segment_info_ptr->num_rows;
    int num_cols = image_segment_info_ptr->num_cols;
    int** seg_map = image_segment_info_ptr->seg_map;
    int i, j;


    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            seg_map[ i ][ j ] = 0;
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void update_seg_map(Segmentation_t1* image_segment_info_ptr)
{
    int**          seg_map      = image_segment_info_ptr->seg_map;
    int            num_segments = image_segment_info_ptr->num_segments;
    int            i, j, k;
    int            seg_num;
    Segment_t1* cur_seg_ptr;
    int            num_pixels;


    zero_seg_map(image_segment_info_ptr);

    for (seg_num = 1; seg_num <= num_segments; seg_num++)
    {
        cur_seg_ptr = image_segment_info_ptr->segments[ seg_num - 1 ];
        num_pixels  = cur_seg_ptr->num_pixels;

        for (k = 0; k<num_pixels; k++)
        {
            i = cur_seg_ptr->pixels[ k ].i;
            j = cur_seg_ptr->pixels[ k ].j;

            seg_map[ i ][ j ] = seg_num;
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_target_image_segment_info
(
    Segmentation_t1** image_seg_info_ptr_ptr,
    int            num_rows,
    int            num_cols
)
{
    Segmentation_t1* image_seg_info_ptr = *image_seg_info_ptr_ptr;


    if (image_seg_info_ptr == NULL)
    {
        NRE(image_seg_info_ptr = create_image_segment_info());
        *image_seg_info_ptr_ptr = image_seg_info_ptr;
    }
    else
    {
        int i;
        int num_segments = image_seg_info_ptr->num_segments;

        for (i=0; i<num_segments; i++)
        {
            free_image_segment(image_seg_info_ptr->segments[ i ]);
        }

        kjb_free(image_seg_info_ptr->segments);
        image_seg_info_ptr->segments = NULL;

        if (    (image_seg_info_ptr->num_rows != num_rows)
             || (image_seg_info_ptr->num_cols != num_cols)
           )
        {
            UNTESTED_CODE();

            verbose_pso(10, "Changing segmentation size from (%d, %d) to (%d, %d).\n",
                        image_seg_info_ptr->num_rows,
                        image_seg_info_ptr->num_cols,
                        num_rows, num_cols);

            free_2D_int_array(image_seg_info_ptr->seg_map);
            image_seg_info_ptr->seg_map = NULL;
        }
    }

    if (image_seg_info_ptr->seg_map == NULL)
    {
        image_seg_info_ptr->seg_map = allocate_2D_int_array(num_rows, num_cols);

        if (image_seg_info_ptr->seg_map == NULL)
        {
            kjb_free(image_seg_info_ptr);
            return ERROR;
        }
    }

    image_seg_info_ptr->num_rows = num_rows;
    image_seg_info_ptr->num_cols = num_cols;
    image_seg_info_ptr->num_segments = 0;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Segmentation_t1* create_image_segment_info(void)
{
    Segmentation_t1* image_segment_info_ptr;

    NRN(image_segment_info_ptr = TYPE_MALLOC(Segmentation_t1));

    image_segment_info_ptr->segments = NULL;
    image_segment_info_ptr->seg_map = NULL;

    return image_segment_info_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void t1_free_segmentation(Segmentation_t1* image_segment_info_ptr)
{

    if (image_segment_info_ptr != NULL)
    {
        int num_segments = image_segment_info_ptr->num_segments;
        int i;

        for (i=0; i<num_segments; i++)
        {
            free_image_segment(image_segment_info_ptr->segments[ i ]);
        }

        kjb_free(image_segment_info_ptr->segments);
        free_2D_int_array(image_segment_info_ptr->seg_map);
        kjb_free(image_segment_info_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Segment_t1* create_image_segment(int segment_number)
{
    Segment_t1* seg_ptr;
    int      k;
    double   R_ave;
    double   G_ave;
    double   B_ave;
    double   sum_RGB_ave;


    NRN(seg_ptr = TYPE_MALLOC(Segment_t1));

    seg_ptr->segment_number = segment_number;

    R_ave = cached_R_sum / num_cached_pixels;
    G_ave = cached_G_sum / num_cached_pixels;
    B_ave = cached_B_sum / num_cached_pixels;

    seg_ptr->R_ave = R_ave;
    seg_ptr->G_ave = G_ave;
    seg_ptr->B_ave = B_ave;

    sum_RGB_ave = R_ave + G_ave + B_ave;
    sum_RGB_ave += RGB_SUM_EPSILON;

    seg_ptr->r_chrom_ave = R_ave / sum_RGB_ave;
    seg_ptr->g_chrom_ave = B_ave / sum_RGB_ave;

    seg_ptr->i_CM = cached_i_sum / num_cached_pixels;
    seg_ptr->j_CM = cached_j_sum / num_cached_pixels;

    seg_ptr->i_min = cached_i_min;
    seg_ptr->i_max = cached_i_max;
    seg_ptr->j_min = cached_j_min;
    seg_ptr->j_max = cached_j_max;

    seg_ptr->pixels = N_TYPE_MALLOC(Pixel_info, num_cached_pixels);

    if (seg_ptr->pixels == NULL)
    {
        kjb_free(seg_ptr);
        return NULL;
    }

    for (k=0; k<num_cached_pixels; k++)
    {
        seg_ptr->pixels[ k ] = cached_pixels[ k ];
    }

    seg_ptr->num_pixels = num_cached_pixels;

    seg_ptr->num_boundary_pixels = 0;
    seg_ptr->boundary_pixels = NULL;

    seg_ptr->outside_boundary_mp = NULL;

    seg_ptr->num_neighbours = 0;
    seg_ptr->neighbours = NULL;

    seg_ptr->i_inside = NOT_SET;
    seg_ptr->j_inside = NOT_SET;

    seg_ptr->first_moment = DBL_NOT_SET;
    seg_ptr->second_moment = DBL_NOT_SET;

    seg_ptr->boundary_len = DBL_NOT_SET;
    seg_ptr->outside_boundary_len = DBL_NOT_SET;

    return seg_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
// Helper function for free_queue().
*/
static void free_image_segment_storage(void* segment_ptr)
{

    free_image_segment((Segment_t1*)segment_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void free_image_segment(Segment_t1* segment_ptr)
{

    if (segment_ptr != NULL)
    {
        kjb_free(segment_ptr->pixels);
        kjb_free(segment_ptr->boundary_pixels);
        free_matrix(segment_ptr->outside_boundary_mp);
        kjb_free(segment_ptr->neighbours);
    }

    kjb_free(segment_ptr);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_pixel_cache(int new_cache_size)
{
    static int pixel_cache_size = 0;

    if (new_cache_size > pixel_cache_size)
    {
        kjb_free(cached_pixels);
        NRE(cached_pixels = N_TYPE_MALLOC(Pixel_info, new_cache_size));
        pixel_cache_size = new_cache_size;
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


    kjb_free(cached_pixels);
    kjb_free(cached_neighbours);

#ifdef IMPLEMENT_MERGING
    free_2D_int_array(merge_counts);
    free_matrix(merge_r_diff_mp);
    free_matrix(merge_g_diff_mp);
    free_matrix(merge_sum_diff_mp);
    kjb_free(segment_indices_after_merging);
#endif

    free_int_matrix(cached_magnified_seg_map);

}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

