
/* $Id: t2_segment.c 7987 2010-12-15 09:32:33Z kobus $ */

/*
     Copyright (c) 1994-2008 by Kobus Barnard (author).

     Personal and educational use of this code is granted, provided
     that this header is kept intact, and that the authorship is not
     misrepresented. Commercial use is not permitted.
*/

#include "t2/t2_gen.h"     /* Only safe as first include in a ".c" file. */
#include "t2/t2_segment.h"

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

typedef struct Cached_pixel
{
    Pixel pixel;
    int   i;
    int   j;
    int   seg_num;
}
Cached_pixel;

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
static int seg_min_initial_segment_size = 4;
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
#ifdef TEST
static int count_remark = 0;
#endif
static Cached_pixel* cached_pixels = NULL;
static int* cached_pixel_counts = NULL;
static int* cached_initial_segment_numbers = NULL;
static int* cached_good_segment_numbers = NULL;
static float* cached_R_means = NULL;
static float* cached_G_means = NULL;
static float* cached_B_means = NULL;
static float* cached_sum_RGB_means = NULL;
static float* cached_r_chrom_means = NULL;
static float* cached_g_chrom_means = NULL;
static Int_matrix*  cached_magnified_boundary_points = NULL;
static Int_matrix*  cached_magnified_seg_map = NULL;
static double cached_R_sum;
static double cached_G_sum;
static double cached_B_sum;
static double cached_r_chrom_sum;
static double cached_g_chrom_sum;
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

static int get_target_image_segment_info
(
    Segmentation_t2** image_seg_info_ptr_ptr,
    int            num_rows,
    int            num_cols
);

static Segmentation_t2* create_image_segment_info(void);

static Segment_t2* create_image_segment
(
    int segment_number,
    int num_pixels
);

static void free_image_segment(Segment_t2* segment_ptr);

static int initialize_caches(int num_rows, int num_cols);

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_allocated_static_data(void);
    static void prepare_memory_cleanup(void);
#endif

static void get_initial_segmentation(const KJB_image* ip, int** seg_map);

static void fill_holes(const KJB_image* ip, int** seg_map);

static void find_boundary_pixels
(
    const KJB_image* ip,
    int**            seg_map,
    int*             num_pixels_ptr,
    Cached_pixel*    pixel_buff
);

static int merge_segments
(
    const KJB_image* ip,
    int**            seg_map,
    int              num_boundary_pixels,
    Cached_pixel*    boundary_pixels,
    int*             seg_count_ptr,
    int*             pixel_counts,
    float*           R_means,
    float*           G_means,
    float*           B_means,
    float*           r_chrom_means,
    float*           g_chrom_means,
    float*           sum_RGB_means
);

static int merge_segment_pair
(
    int    i,
    int    j,
    int*   pixel_counts,
    float* R_means,
    float* G_means,
    float* B_means,
    float* r_chrom_means,
    float* g_chrom_means,
    float* sum_RGB_means
);

static int get_segment_pixels
(
    const KJB_image* ip,
    Segmentation_t2*    segment_info_ptr,
    int              num_segments,
    int*             pixel_counts,
    float*           R_means,
    float*           G_means,
    float*           B_means,
    float*           r_chrom_means,
    float*           g_chrom_means,
    float*           sum_RGB_means
);

static void update_segment_map(Segmentation_t2* segment_info_ptr);

static int get_boundaries_of_segments
(
    Segmentation_t2* image_segment_info_ptr,
    int           num_boundary_pixels,
    Cached_pixel* pixel_buff
);

static int find_interior_points(Segmentation_t2* image_segment_info_ptr);

static int find_outside_boundary
(
    Segmentation_t2* image_segment_info_ptr
);

static int find_neighbours(Segmentation_t2* image_segment_info_ptr);

static int get_good_segment_number(int seg_num);
static int chase_initial_segment_numbers(int seg_num);
static void get_initial_segmentation_helper(int i, int j);
static int grow_segment(int i, int j);
static int grow_segment_using_rg_var_and_lum_diff(int i, int j);
static void grow_segment_4(int i, int j);
static void grow_segment_using_rg_var_and_lum_diff_4(int i, int j);
static void grow_segment_8(int i, int j);
static void grow_segment_using_rg_var_and_lum_diff_8(int i, int j);
static void remark_segment(int i, int j, int new_seg_num);
static void zero_seg_map(const KJB_image* ip, int** seg_map);

#ifdef TEST
static int verify_segmentation(Segmentation_t2* image_segment_info_ptr);

static int count_connected_pixels
(
    Segmentation_t2* image_segment_info_ptr,
    int           i,
    int           j
);
#endif

/* -------------------------------------------------------------------------- */

static Method_option segmentation_methods[ ] =
{
    { "general",         "g",       (int(*)())grow_segment},
    { "rg-var-lum-diff", "rg-lum",  (int(*)())grow_segment_using_rg_var_and_lum_diff}
};


static const int num_segmentation_methods =
                                sizeof(segmentation_methods) /
                                          sizeof(segmentation_methods[ 0 ]);
static int         segmentation_method                  = 0;
static const char* segmentation_method_short_str= "t2-seg-method";
static const char* segmentation_method_long_str = "t2-segmentation-method";

/* -------------------------------------------------------------------------- */

int t2_set_segmentation_options(const char* option, const char* value)
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
                                "T2 segmentation method",
                                value, &segmentation_method));
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t2-segmentation-fill-hole-level"))
         || (match_pattern(lc_option, "t2-seg-fill-hole-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-fill-hole-level = %d\n",
                    seg_fill_hole_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation holes are %s .\n",
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
         || (match_pattern(lc_option, "t2-segmentation-min-segment-size"))
         || (match_pattern(lc_option, "t2-seg-min-segment-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-min-segment-size = %d\n",
                    seg_min_segment_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation min segment size is %d.\n",
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
         || (match_pattern(lc_option, "t2-segmentation-resegment-size"))
         || (match_pattern(lc_option, "t2-seg-resegment-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-resegment-size = %d\n",
                    seg_min_resegment_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation resegment size is %d.\n",
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
         || (match_pattern(lc_option, "t2-segmentation-connection-max-step"))
         || (match_pattern(lc_option, "t2-seg-connection-max-step"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-connection-max-step = %d\n", connection_max_step));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation connection max step is %d.\n",
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
         || (match_pattern(lc_option, "t2-segmentation-resegment-level"))
         || (match_pattern(lc_option, "t2-seg-resegment-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-resegment-level = %d\n",
                    seg_resegment_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation resegment level is %d.\n",
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
         || (match_pattern(lc_option, "t2-seg-merge-rgb-noise-estimate"))
         || (match_pattern(lc_option, "t2-seg-merge-rgb-noise"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-merge-rgb-noise-estimate = %.2f\n",
                    merge_RGB_noise_estimate));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation RGB noise estimate for merge threshold is %.2f.\n",
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
         || (match_pattern(lc_option, "t2-seg-merge-rg-threshold"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-merge-rg-threshold = %.2f\n",
                    merge_rg_threshold));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation merge rg threshold is %.2f.\n",
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
         || (match_pattern(lc_option, "t2-seg-merge-sum-abs-threshold"))
         || (match_pattern(lc_option, "t2-seg-merge-rgb-sum-abs-threshold"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-merge-rgb-sum-abs-threshold = %.2f\n",
                    merge_sum_RGB_abs_threshold));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation merge RGB sum threshold is %.2f.\n",
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
         || (match_pattern(lc_option, "t2-seg-merge-sum-rel-threshold"))
         || (match_pattern(lc_option, "t2-seg-merge-rgb-sum-rel-threshold"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-merge-rgb-sum-rel-threshold = %.2f\n",
                    merge_sum_RGB_rel_threshold));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation merge RGB sum threshold is %.2f.\n",
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
         || (match_pattern(lc_option, "t2-seg-merge-min-num-pixels"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-merge-min-num-pixels = %d\n",
                    merge_min_num_pixels));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation merge min number of adjacent pixels is %d.\n",
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
         || (match_pattern(lc_option, "t2-segmentation-merge-level"))
         || (match_pattern(lc_option, "t2-seg-merge-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-merge-level = %d\n",
                    seg_merge_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation merge level is %d.\n",
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
         || (match_pattern(lc_option, "t2-segmentation-connect-corners"))
         || (match_pattern(lc_option, "t2-seg-connect-corners"))
       )
    {
        int temp_boolean;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-connect-corners = %s\n",
                    seg_connect_corners ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T2 segmentation connects corners %s used.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-rgb-var"))
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
                ERE(pso("t2-seg-max-rel-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-rel-rgb-var = %.3f\n",
                        (double)max_rel_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_RGB_var < 0.0)
            {
                ERE(pso("Maximum relative RGB variance is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative RGB variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-rgb-var"))
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
                ERE(pso("t2-seg-max-abs-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-abs-rgb-var = %.3f\n",
                        (double)max_abs_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_RGB_var < 0.0)
            {
                ERE(pso("Maximum absolute RGB variance is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute RGB variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-sum-rgb-var"))
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
                ERE(pso("t2-seg-max-rel-sum-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-rel-sum-rgb-var = %.3f\n",
                        (double)max_rel_sum_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_sum_RGB_var < 0.0)
            {
                ERE(pso("Maximum relative sum RGB variance is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative sum RGB variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-sum-rgb-var"))
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
                ERE(pso("t2-seg-max-abs-sum-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-abs-sum-rgb-var = %.3f\n",
                        (double)max_abs_sum_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_sum_RGB_var < 0.0)
            {
                ERE(pso("Maximum absolute sum RGB variance is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute sum RGB variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-chrom-var"))
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
                ERE(pso("t2-seg-max-rel-chrom-var = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-rel-chrom-var = %.3f\n",
                        (double)max_rel_chrom_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_chrom_var < 0.0)
            {
                ERE(pso("Maximum relative chrom variance is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative chrom variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-chrom-var"))
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
                ERE(pso("t2-seg-max-abs-chrom-var = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-abs-chrom-var = %.3f\n",
                        (double)max_abs_chrom_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_chrom_var < 0.0)
            {
                ERE(pso("Maximum absolute chrom variance is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute chrom variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-rgb-sqr-diff"))
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
                ERE(pso("t2-seg-max-rel-rgb-sqr-diff = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-rel-rgb-sqr-diff = %.3f\n",
                        (double)max_rel_RGB_sqr_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_RGB_sqr_diff < 0.0)
            {
                ERE(pso("Maximum relative sum RGB sqr difference is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative sum RGB sqr difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-rgb-sqr-diff"))
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
                ERE(pso("t2-seg-max-abs-rgb-sqr-diff = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-abs-rgb-sqr-diff = %.3f\n",
                        (double)max_abs_RGB_sqr_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_RGB_sqr_diff < 0.0)
            {
                ERE(pso("Maximum absolute sum RGB sqr difference is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute sum RGB sqr difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-sum-rgb-diff"))
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
                ERE(pso("t2-seg-max-rel-sum-rgb-diff = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-rel-sum-rgb-diff = %.3f\n",
                        (double)max_rel_sum_RGB_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_sum_RGB_diff < 0.0)
            {
                ERE(pso("Maximum relative sum RGB difference is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative sum RGB difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-sum-rgb-diff"))
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
                ERE(pso("t2-seg-max-abs-sum-rgb-diff = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-abs-sum-rgb-diff = %.3f\n",
                        (double)max_abs_sum_RGB_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_sum_RGB_diff < 0.0)
            {
                ERE(pso("Maximum absolute sum RGB difference is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute sum RGB difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-chrom-diff"))
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
                ERE(pso("t2-seg-max-rel-chrom-diff = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-rel-chrom-diff = %.3f\n",
                        (double)max_rel_chrom_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_rel_chrom_diff < 0.0)
            {
                ERE(pso("Maximum relative chrom difference is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative chrom difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-chrom-diff"))
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
                ERE(pso("t2-seg-max-abs-chrom-diff = off\n"));
            }
            else
            {
                ERE(pso("t2-seg-max-abs-chrom-diff = %.3f\n",
                        (double)max_abs_chrom_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (max_abs_chrom_diff < 0.0)
            {
                ERE(pso("Maximum absolute chrom difference is not used for T2 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute chrom difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-rgb-var"))
       )
    {
        extern float max_rel_RGB_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-rel-rgb-var = %.3f\n",
                    (double)max_rel_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative RGB variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-rgb-var"))
       )
    {
        extern float max_abs_RGB_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-abs-rgb-var = %.3f\n", (double)max_abs_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute RGB variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-sum-rgb-var"))
       )
    {
        extern float max_rel_sum_RGB_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-rel-sum-rgb-var = %.3f\n",
                    (double)max_rel_sum_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative sum RGB variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-sum-rgb-var"))
       )
    {
        extern float max_abs_sum_RGB_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-abs-sum-rgb-var = %.3f\n",
                    (double)max_abs_sum_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute sum RGB variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-chrom-var"))
       )
    {
        extern float max_rel_chrom_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-rel-chrom-var = %.3f\n",
                    (double)max_rel_chrom_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative chrom variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-chrom-var"))
       )
    {
        extern float max_abs_chrom_var;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-abs-chrom-var = %.3f\n",
                    (double)max_abs_chrom_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute chrom variance for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-rgb-sqr-diff"))
       )
    {
        extern float max_rel_RGB_sqr_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-rel-rgb-sqr-diff = %.3f\n",
                    (double)max_rel_RGB_sqr_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative sum RGB sqr difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-rgb-sqr-diff"))
       )
    {
        extern float max_abs_RGB_sqr_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-abs-rgb-sqr-diff = %.3f\n",
                    (double)max_abs_RGB_sqr_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute sum RGB sqr difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-sum-rgb-diff"))
       )
    {
        extern float max_rel_sum_RGB_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-rel-sum-rgb-diff = %.3f\n",
                    (double)max_rel_sum_RGB_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative sum RGB difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-sum-rgb-diff"))
       )
    {
        extern float max_abs_sum_RGB_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-abs-sum-rgb-diff = %.3f\n",
                    (double)max_abs_sum_RGB_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute sum RGB difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-rel-chrom-diff"))
       )
    {
        extern float max_rel_chrom_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-rel-chrom-diff = %.3f\n",
                    (double)max_rel_chrom_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative chrom difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-max-abs-chrom-diff"))
       )
    {
        extern float max_abs_chrom_diff;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-max-abs-chrom-diff = %.3f\n",
                    (double)max_abs_chrom_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute chrom difference for T2 segmenting is %.3f.\n",
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
         || (match_pattern(lc_option, "t2-seg-min-rgb-sum-for-chrom-test"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-min-rgb-sum-for-chrom-test = %.3f\n",
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
         || (match_pattern(lc_option, "t2-seg-min-rgb-sum-for-relative-sum-diff"))
         || (match_pattern(lc_option, "t2-seg-min-rgb-sum-for-relative-diff"))
         || (match_pattern(lc_option, "t2-seg-min-rgb-sum-for-rel-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-min-rgb-sum-for-relative-sum-diff = %.3f\n",
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
         || (match_pattern(lc_option, "t2-segmentation-verify-segmentation"))
         || (match_pattern(lc_option, "t2-seg-verify-segmentation"))
       )
    {
        int temp_boolean;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t2-seg-verify-segmentation = %s\n",
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
 *                            t2_segment_image
 *
 * Segments the image into regions
 *
 * This routine segments an image into regions. The segmentation is done under
 * the control of a large number of options normally exposed to the user, and
 * also accessable using kjb_set(). The routine makes minor changes to the image
 * to ensure consistency between the segmentation and the image. If you do not
 * want the image to be changed, you MUST make a copy. The routine fills a
 * structure which includes:
 *     The number of segments
 *     An image size integer array with the segment number at each (i,j)
 *     An array of segments (Type Segment_t2*).
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

int t2_segment_image
(
    const KJB_image* ip,
    Segmentation_t2**   segment_info_ptr_ptr
)
{
    Segmentation_t2* segment_info_ptr;
    int           i;
    int           num_rows = ip->num_rows;
    int           num_cols = ip->num_cols;
    long          initial_cpu_time = get_cpu_time();
    int           merge;


    ERE(get_target_image_segment_info(segment_info_ptr_ptr, num_rows,num_cols));
    segment_info_ptr = *segment_info_ptr_ptr;

    ERE(initialize_caches(num_rows, num_cols));

    get_initial_segmentation(ip, segment_info_ptr->seg_map);

    for (i=0; i <= cached_segment_count; i++)
    {
        cached_initial_segment_numbers[ i ] = i;
    }

    fill_holes(ip, segment_info_ptr->seg_map);

    find_boundary_pixels(ip, segment_info_ptr->seg_map, &num_cached_pixels,
                         cached_pixels);

#ifdef TEST
    dbi(num_cached_pixels);
    dbx(cached_pixels);
    for (i=0; i<num_cached_pixels; i++)
    {
        ASSERT(cached_pixels[ i ].seg_num > 0);
    }
#endif

#ifdef IMPLEMENT_MERGING
    for (merge = 0;
         (merge < seg_merge_level) && (cached_segment_count > 1);
         merge++
        )
    {
        ERE(merge_segments(ip, segment_info_ptr->seg_map,
                               num_cached_pixels, cached_pixels,
                               &cached_segment_count,
                               cached_pixel_counts,
                               cached_R_means, cached_G_means, cached_B_means,
                               cached_r_chrom_means, cached_g_chrom_means,
                               cached_sum_RGB_means));

        fill_holes(ip, segment_info_ptr->seg_map);

        find_boundary_pixels(ip, segment_info_ptr->seg_map,
                                 &num_cached_pixels,
                                 cached_pixels);
    }
#endif

    /*
    // Home stretch. We are not longer modifying things, just getting the
    // segmentation information into the appropriate form.
    */
    ERE(get_segment_pixels(ip, segment_info_ptr, cached_segment_count,
                           cached_pixel_counts, cached_R_means,
                           cached_G_means, cached_B_means,
                           cached_r_chrom_means, cached_g_chrom_means,
                           cached_sum_RGB_means));

    ERE(get_boundaries_of_segments(segment_info_ptr, num_cached_pixels,
                                   cached_pixels));

    update_segment_map(segment_info_ptr);

    /*
    // At this point, the correct segment number is in the segment map. We no
    // longer use get_good_segment_number().
    */
    find_interior_points(segment_info_ptr);
    find_outside_boundary(segment_info_ptr);
    find_neighbours(segment_info_ptr);

#ifdef TEST
    if (seg_verify_segmentation)
    {
        verify_segmentation(segment_info_ptr);
    }
#endif

    verbose_pso(2, "Complete segmentation has %d segments and took %ldms.\n",
                segment_info_ptr->num_segments,
                get_cpu_time() - initial_cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void get_initial_segmentation(const KJB_image* ip, int** seg_map)
{
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int i, j;
    long initial_cpu_time = get_cpu_time();


    zero_seg_map(ip, seg_map);

    /* Normally bad practice, but we want to improve performace! */
    cached_ip = ip;
    cached_seg_map = seg_map;
    cached_num_rows = num_rows;
    cached_num_cols = num_cols;

    cached_segment_count = 0;

    /*
    // Visit each pixel. For each one that is not marked, create a segment, and
    // segment the image by growing the region. (Note that as the region grows,
    // those pixels get marked).
    */
    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            if (seg_map[ i ][ j ] == 0)
            {
                cached_segment_count++;
                get_initial_segmentation_helper(i, j);

                if (num_cached_pixels < seg_min_initial_segment_size)
                {
                    /* Segment_t2 is not big enough. Back off this segment, and set
                    // the map locations to the negative of the number of
                    // pixels.
                    */
#ifdef TEST
                    count_remark = 0;
#endif
                    remark_segment(i, j, -num_cached_pixels);
#ifdef TEST
                    ASSERT(count_remark == num_cached_pixels);
#endif
                    cached_segment_count--;
                }
                else
                {
                    int  num_pixels = num_cached_pixels;
                    int  s = cached_segment_count - 1;
                    double ave_sum_RGB;

                    cached_pixel_counts[ s ] = num_pixels;

                    cached_R_means[ s ] = cached_R_sum / num_pixels;
                    cached_G_means[ s ] = cached_G_sum / num_pixels;
                    cached_B_means[ s ] = cached_B_sum / num_pixels;

                    cached_r_chrom_means[ s ] = cached_r_chrom_sum / num_pixels;
                    cached_g_chrom_means[ s ] = cached_g_chrom_sum / num_pixels;

                    ave_sum_RGB = cached_R_sum + cached_G_sum + cached_B_sum;

                    cached_sum_RGB_means[ s ] = ave_sum_RGB / num_pixels;
                }
            }
        }
    }

#ifdef TEST
    {
        int num_pixels_in_segments = 0;
        int num_unsegmented_pixels = 0 ;

        for (i=0; i < cached_segment_count; i++)
        {
            num_pixels_in_segments += cached_pixel_counts[ i ];
        }

        verbose_pso(10, "Image has %d pixels in initial segments.\n",
                    num_pixels_in_segments);

        for (i=0; i<num_rows; i++)
        {
            for (j=0; j<num_cols; j++)
            {
                ASSERT(cached_seg_map[ i ][ j ] != 0);

                if (cached_seg_map[ i ][ j ] < 0)
                {
                    num_unsegmented_pixels++;
                }
            }
        }

        verbose_pso(10, "Image has %d unsegmented pixels.\n",
                    num_unsegmented_pixels);

        verbose_pso(10, "Image size is %d x %d = %d.\n",
                    num_rows, num_cols, num_rows * num_cols);

        ASSERT(num_pixels_in_segments + num_unsegmented_pixels == num_rows * num_cols);
    }
#endif

    verbose_pso(3, "Initial segmentation has %d segments and took %ldms.\n",
                cached_segment_count, get_cpu_time() - initial_cpu_time);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void fill_holes(const KJB_image* ip, int** seg_map)
{
    int i, j;
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    long initial_cpu_time = get_cpu_time();
    int         seg_num;
    Pixel pixel;
    int         iteration;


    pixel.extra.invalid.r = VALID_PIXEL;
    pixel.extra.invalid.g = VALID_PIXEL;
    pixel.extra.invalid.b = VALID_PIXEL;
    pixel.extra.invalid.pixel = VALID_PIXEL;

    for (iteration = 0; iteration < seg_fill_hole_level; iteration++)
    {
        for (i=1; i<num_rows - 1; i++)
        {
            for (j=1; j<num_cols - 1; j++)
            {
                seg_num = seg_map[ i ][ j ];

                ASSERT(seg_num != 0);

                if (seg_num < 0)
                {
                    int surround_seg_num = NOT_SET;
                    int s = seg_map[ i ][ j - 1 ];
                    int t = seg_map[ i - 1 ][ j ];

                    s = chase_initial_segment_numbers(s);
                    t = chase_initial_segment_numbers(t);

                    if (    (s > 0)
                         && (chase_initial_segment_numbers(seg_map[ i ][ j + 1 ]) == s)
                         && (    (    (chase_initial_segment_numbers(seg_map[ i + 1 ][ j + 1 ]) == s)
                                   && (chase_initial_segment_numbers(seg_map[ i + 1 ][ j     ]) == s)
                                   && (chase_initial_segment_numbers(seg_map[ i + 1 ][ j - 1 ]) == s)
                                 )
                              ||
                                 (    (chase_initial_segment_numbers(seg_map[ i - 1 ][ j + 1 ]) == s)
                                   && (chase_initial_segment_numbers(seg_map[ i - 1 ][ j     ]) == s)
                                   && (chase_initial_segment_numbers(seg_map[ i - 1 ][ j - 1 ]) == s)
                                 )
                             )
                       )
                    {
                        surround_seg_num = s;
                    }
                    else if (    (t > 0)
                              && (chase_initial_segment_numbers(seg_map[ i + 1 ][ j ]) == t)
                              && (    (    (chase_initial_segment_numbers(seg_map[ i + 1 ][ j + 1 ]) == t)
                                        && (chase_initial_segment_numbers(seg_map[ i     ][ j + 1 ]) == t)
                                        && (chase_initial_segment_numbers(seg_map[ i - 1 ][ j + 1 ]) == t)
                                      )
                                   ||
                                      (    (chase_initial_segment_numbers(seg_map[ i + 1 ][ j - 1 ]) == t)
                                        && (chase_initial_segment_numbers(seg_map[ i     ][ j - 1 ]) == t)
                                        && (chase_initial_segment_numbers(seg_map[ i - 1 ][ j - 1 ]) == t)
                                     )
                                 )
                            )
                    {
                        surround_seg_num = t;
                    }

                    if (surround_seg_num > 0)
                    {
                        int surround_count = 0;
                        int di, dj;
                        float R = 0.0;
                        float G = 0.0;
                        float B = 0.0;

                        for (di=-1; di<=1; di++)
                        {
                            for (dj=-1; dj<=1; dj++)
                            {
                                if ((di != 0) || (dj != 0))
                                {
                                    int m = i + di;
                                    int n = j + dj;

                                    if (chase_initial_segment_numbers(seg_map[m][n]) == surround_seg_num)
                                    {
                                        R += ip->pixels[ m ][ n ].r;
                                        G += ip->pixels[ m ][ n ].g;
                                        B += ip->pixels[ m ][ n ].b;
                                        surround_count++;
                                    }
                                }
                            }
                        }

                        ASSERT(surround_count >= 5);

                        seg_map[ i][j ] = surround_seg_num;

                        R /= surround_count;
                        pixel.r = R;

                        G /= surround_count;
                        pixel.g = G;

                        B /= surround_count;
                        pixel.b = B;

                        ip->pixels[ i ][ j ] = pixel;

                        cached_pixel_counts[ surround_seg_num - 1]++;
                    }
                }
            }
        }
    }

    verbose_pso(5, "Filling holes (%d iterations) took %ldms.\n",
                seg_fill_hole_level, get_cpu_time() - initial_cpu_time);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void find_boundary_pixels
(
    const KJB_image* ip,
    int**            seg_map,
    int*             num_pixels_ptr,
    Cached_pixel*    pixel_buff
)
{
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int i, j;
    long initial_cpu_time = get_cpu_time();


    dbi(*num_pixels_ptr);
    *num_pixels_ptr = 0;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            int seg_num = chase_initial_segment_numbers(seg_map[ i ][ j ]);

            if (seg_num > 0)
            {
                if (    (i == 0) || (j == 0)
                     || (i == num_rows - 1) || (j == num_cols - 1)
                     || (chase_initial_segment_numbers(seg_map[ i + 1 ][ j ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i - 1 ][ j ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i ][ j + 1 ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i ][ j - 1 ]) != seg_num)
                     /* CHECK: Do we count inside corner pixels? */
                     || (chase_initial_segment_numbers(seg_map[ i + 1 ][ j + 1 ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i - 1 ][ j + 1 ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i + 1 ][ j - 1 ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i - 1 ][ j - 1 ]) != seg_num)
                   )
                {
                    pixel_buff->pixel  = cached_ip->pixels[ i ][ j ];
                    pixel_buff->i = i;
                    pixel_buff->j = j;
                    pixel_buff->seg_num = seg_num;

                    pixel_buff++;
                    (*num_pixels_ptr)++;
                }
            }
        }
    }

    verbose_pso(5, "Finding boundary pixels took %ldms.\n",
                get_cpu_time() - initial_cpu_time);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef IMPLEMENT_MERGING

/*
// This is not finished yet!
*/
static int merge_segments
(
    const KJB_image* ip,
    int**            seg_map,
    int              num_boundary_pixels,
    Cached_pixel*    boundary_pixels,
    int*             seg_count_ptr,
    int*             pixel_counts,
    float*           R_means,
    float*           G_means,
    float*           B_means,
    float*           r_chrom_means,
    float*           g_chrom_means,
    float*           sum_RGB_means
)
{
    static int      max_num_segments  = 0;
    int num_rows     = ip->num_rows;
    int num_cols     = ip->num_cols;
    int num_segments = *seg_count_ptr;
    int i, j;
    int count;
    long initial_cpu_time = get_cpu_time();
    long cpu_time_1;
    long cpu_time_2;
    long cpu_time_3;
    long cpu_time_4;
    int  nb_num;


    UNTESTED_CODE();
    dbw();

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (num_segments > max_num_segments)
    {
        verbose_pso(10, "Bumping up storage for merging.\n");

        free_2D_int_array(merge_counts);

        kjb_free(segment_indices_after_merging);
        NRE(segment_indices_after_merging = INT_MALLOC(num_segments));

        NRE(merge_counts = allocate_2D_int_array(num_segments, num_segments));

        ERE(get_target_matrix(&merge_r_diff_mp, num_segments, num_segments));

        ERE(get_target_matrix(&merge_g_diff_mp, num_segments, num_segments));

        ERE(get_target_matrix(&merge_sum_diff_mp, num_segments, num_segments));

        max_num_segments = num_segments;
    }

    cpu_time_1 = get_cpu_time();
    verbose_pso(10, "Merge part one took %ldms.\n",
                cpu_time_1 - initial_cpu_time);

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

    cpu_time_2 = get_cpu_time();
    verbose_pso(10, "Merge part two took %ldms.\n",
                cpu_time_2 - cpu_time_1);
    /*
    // Update merge info based on boundary pairs. Note that things are
    // overcounted roughly 2 to 3 times. Thus if two regions are joined through
    // 10 pixels, they may have a merge count of 30.
    */
    dbi(num_boundary_pixels);
    dbx(boundary_pixels);

    for (i=0; i<num_boundary_pixels; i++)
    {
        int seg_num = boundary_pixels[ i ].seg_num;
        int b_i = boundary_pixels[ i ].i;
        int b_j = boundary_pixels[ i ].j;
        float b_R = ip->pixels[ b_i ][ b_j ].r;
        float b_G = ip->pixels[ b_i ][ b_j ].g;
        float b_B = ip->pixels[ b_i ][ b_j ].b;
        float b_sum = b_R + b_G + b_B + RGB_SUM_EPSILON;
        float b_r = b_R / b_sum;
        float b_g = b_G / b_sum;
        int di, dj;

        /*
        // FIX (?)
        //
        // We may want a separate value and option to control whether the region
        // is big enough to bother with. For now, merge_min_num_pixels will do.
        */
        if (pixel_counts[ seg_num - 1 ] > merge_min_num_pixels)
        {
            for (di = -1; di <= 1; di++)
            {
                for (dj = -1; dj <= 1; dj++)
                {
                    int i_offset = b_i + di;
                    int j_offset = b_j + dj;

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
                         && ((nb_num = seg_map[ i_offset ][ j_offset ]) > 0)
                         && (nb_num != seg_num)
                       )
                    {
                        float n_R, n_G, n_B, n_r, n_g, n_sum;
                        double r_diff, g_diff, sum_diff;

                        n_R = cached_ip->pixels[ i_offset ][ j_offset ].r;
                        n_G = cached_ip->pixels[ i_offset ][ j_offset ].g;
                        n_B = cached_ip->pixels[ i_offset ][ j_offset ].b;
                        n_sum = n_R + n_G + n_B + RGB_SUM_EPSILON;
                        n_r = n_R / n_sum;
                        n_g = n_G / n_sum;

                        r_diff = b_r - n_r;
                        g_diff = b_g - n_g;
                        sum_diff = b_sum - n_sum;

                        merge_r_diff_mp->elements[ seg_num - 1 ][ nb_num - 1 ] += r_diff*r_diff;
                        merge_g_diff_mp->elements[ seg_num - 1 ][ nb_num - 1 ] += g_diff*g_diff;
                        merge_sum_diff_mp->elements[ seg_num - 1 ][ nb_num - 1 ] += sum_diff*sum_diff;

                        (merge_counts[ seg_num - 1 ][ nb_num - 1 ])++;
                    }
                }
            }
        }
    }

    cpu_time_3 = get_cpu_time();
    verbose_pso(10, "Merge part three took %ldms.\n",
                cpu_time_3 - cpu_time_2);

    /* Look at merge charts for regions that should be merged. */

    for (i=0; i<num_segments; i++)
    {
        for (j = i+1; j<num_segments; j++)
        {
            count = merge_counts[ i ][ j ];

            /*
            // Due to over-counting, we actually want three times as many counts
            // as the user thinks they want. (This could use a bit of
            // adjustment.)
            */
            if (count > 3 * merge_min_num_pixels)
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

                    ERE(merge_segment_pair(i, j, pixel_counts,
                                               R_means, G_means, B_means,
                                               r_chrom_means, g_chrom_means,
                                               sum_RGB_means));
                    (seg_count_ptr)--;
                }
            }
        }
    }

    cpu_time_4 = get_cpu_time();
    verbose_pso(10, "Merge part four took %ldms.\n",
                cpu_time_4 - cpu_time_3);

    verbose_pso(5, "Merge took %ldms.\n", get_cpu_time() - initial_cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int merge_segment_pair
(
    int    i,
    int    j,
    int*   pixel_counts,
    float* R_means,
    float* G_means,
    float* B_means,
    float* r_chrom_means,
    float* g_chrom_means,
    float* sum_RGB_means
)
{
    int   temp;
    float weight_i;
    float weight_j;
    float weight;



    ERE(i = chase_initial_segment_numbers(i + 1) - 1);
    ERE(j = chase_initial_segment_numbers(j + 1) - 1);

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

    verbose_pso(10, "Merging %d (%d pixels) into %d (%d pixels).\n", j+1,
                pixel_counts[ j ], i+1, pixel_counts[ i ]);

    weight_i = pixel_counts[ i ];
    weight_j = pixel_counts[ j ];
    weight = weight_i + weight_j;

    R_means[ i ] *= weight_i;
    R_means[ i ] += R_means[ j ] * weight_j;
    R_means[ i ] /= weight;
    R_means[ j ] = FLT_NOT_SET;

    G_means[ i ] *= weight_i;
    G_means[ i ] += G_means[ j ] * weight_j;
    G_means[ i ] /= weight;
    G_means[ j ] = FLT_NOT_SET;

    B_means[ i ] *= weight_i;
    B_means[ i ] += B_means[ j ] * weight_j;
    B_means[ i ] /= weight;
    B_means[ j ] = FLT_NOT_SET;

    r_chrom_means[ i ] *= weight_i;
    r_chrom_means[ i ] += r_chrom_means[ j ] * weight_j;
    r_chrom_means[ i ] /= weight;
    r_chrom_means[ j ] = FLT_NOT_SET;

    g_chrom_means[ i ] *= weight_i;
    g_chrom_means[ i ] += g_chrom_means[ j ] * weight_j;
    g_chrom_means[ i ] /= weight;
    g_chrom_means[ j ] = FLT_NOT_SET;

    sum_RGB_means[ i ] *= weight_i;
    sum_RGB_means[ i ] += sum_RGB_means[ j ] * weight_j;
    sum_RGB_means[ i ] /= weight;
    sum_RGB_means[ j ] = FLT_NOT_SET;

    pixel_counts[ i ] += pixel_counts[ j ];
    pixel_counts[ j ] = 0;

    cached_initial_segment_numbers[ j + 1 ] = i + 1;

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_segment_pixels
(
    const KJB_image* ip,
    Segmentation_t2*    segment_info_ptr,
    int              num_segments,
    int*             pixel_counts,
    float*           R_means,
    float*           G_means,
    float*           B_means,
    float*           r_chrom_means,
    float*           g_chrom_means,
    float*           sum_RGB_means
)
{
    int   num_rows = ip->num_rows;
    int   num_cols = ip->num_cols;
    int** seg_map  = segment_info_ptr->seg_map;
    int   i, j;
    int   count;
    int   num_good_segments = 0;
    int   initial_seg_num;
    int   num_pixels;
    int   seg_num;
    Segment_t2* seg_ptr;
    int p;
    int* row_ptr;
    long initial_cpu_time = get_cpu_time();


    for (i=0; i <= num_segments; i++)
    {
        cached_good_segment_numbers[ i ] = 0;

        num_pixels = pixel_counts[ i ];

        if (num_pixels >= seg_min_segment_size)
        {
            num_good_segments++;
        }
    }

    verbose_pso(3, "Image has %d good segments.\n", num_good_segments);

    if (num_good_segments == 0) return NO_ERROR;

    NRE(segment_info_ptr->segments =
                            N_TYPE_MALLOC(Segment_t2*, num_good_segments));
    segment_info_ptr->num_segments = num_good_segments;

    /* We want a safe free for the caller if we return without finishing. */
    for (i=0; i<num_good_segments; i++)
    {
        segment_info_ptr->segments[ i ] = NULL;
    }

    count = 0;

    for (i=0; i<num_segments; i++)
    {
#ifdef IT_SEEMS_BUGGY_TO_ME
        ERE(initial_seg_num = chase_initial_segment_numbers(i + 1));
#else
        initial_seg_num = i;
#endif

        if (initial_seg_num > 0)
        {
            num_pixels = pixel_counts[ initial_seg_num - 1 ];

            if (num_pixels >= seg_min_segment_size)
            {
                seg_num = count + 1;

                NRE(segment_info_ptr->segments[ count ] =
                                  create_image_segment(seg_num,
                                                           num_pixels));
                seg_ptr = segment_info_ptr->segments[ count ];

                seg_ptr->R_ave = R_means[ i ];
                seg_ptr->G_ave = G_means[ i ];
                seg_ptr->B_ave = B_means[ i ];
                seg_ptr->r_chrom_ave = r_chrom_means[ i ];
                seg_ptr->g_chrom_ave = g_chrom_means[ i ];
                seg_ptr->sum_RGB_ave = sum_RGB_means[ i ];

                cached_good_segment_numbers[ initial_seg_num ] = seg_num;


                verbose_pso(20, "Initial segment %d has %d pixels. ",
                            initial_seg_num, num_pixels);
                verbose_pso(20, "Making it good segment %d.\n", seg_num);

                count++;
            }
        }
    }

    for (i=0; i<num_rows; i++)
    {
        row_ptr = seg_map[ i ];

        for (j=0; j<num_cols; j++)
        {
            ERE(seg_num = get_good_segment_number(*row_ptr++));

            ASSERT(seg_num >= 0);

            if (seg_num > 0)
            {
                seg_ptr = segment_info_ptr->segments[ seg_num-1 ];

                p = seg_ptr->pixel_count;

                seg_ptr->pixels[ p ].i = i;
                seg_ptr->pixels[ p ].j = j;

                /*
                // If one of the bounds is less than zero, then they all should
                // be less than zero.
                */
                if (seg_ptr->i_min < 0)
                {
                    ASSERT(seg_ptr->i_CM == 0);
                    ASSERT(seg_ptr->j_CM == 0);

                    ASSERT(seg_ptr->j_min < 0);
                    ASSERT(seg_ptr->j_max < 0);
                    ASSERT(seg_ptr->i_max < 0);

                    seg_ptr->i_min = i;
                    seg_ptr->i_max = i;
                    seg_ptr->j_min = j;
                    seg_ptr->j_max = j;
                }
                else
                {
                    if (i < seg_ptr->i_min)
                    {
                        seg_ptr->i_min = i;
                    }
                    else if (i > seg_ptr->i_max)
                    {
                        seg_ptr->i_max = i;
                    }

                    if (j < seg_ptr->j_min)
                    {
                        seg_ptr->j_min = j;
                    }
                    else if (j > seg_ptr->j_max)
                    {
                        seg_ptr->j_max = j;
                    }
                }

                seg_ptr->i_CM += i;
                seg_ptr->j_CM += j;

                (seg_ptr->pixel_count)++;

                ASSERT(seg_ptr->pixel_count <= seg_ptr->num_pixels);

                ASSERT(seg_ptr->i_CM >= 0);
                ASSERT(seg_ptr->j_CM >= 0);
                ASSERT(seg_ptr->i_CM <= num_rows * seg_ptr->pixel_count);
                ASSERT(seg_ptr->j_CM <= num_cols * seg_ptr->pixel_count);
            }
        }
    }

    for (i=0; i<num_good_segments; i++)
    {
        seg_ptr = segment_info_ptr->segments[ i ];

        seg_ptr->i_CM /= (seg_ptr->num_pixels);
        seg_ptr->j_CM /= (seg_ptr->num_pixels);

        ASSERT(seg_ptr->pixel_count == seg_ptr->num_pixels);
    }

    verbose_pso(5, "Collecting segment pixels took %ldms.\n",
                get_cpu_time() - initial_cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void update_segment_map(Segmentation_t2* segment_info_ptr)
{
    int** seg_map  = segment_info_ptr->seg_map;
    int   num_rows = segment_info_ptr->num_rows;
    int   num_cols = segment_info_ptr->num_cols;
    int   i, j;
    long  initial_cpu_time = get_cpu_time();
    int   seg_num;
    int*  row_ptr;


    for (i=0; i<num_rows; i++)
    {
        row_ptr = seg_map[ i ];

        for (j=0; j<num_cols; j++)
        {
            seg_num = *row_ptr;

            ASSERT(seg_num != 0);

            if (seg_num >= 0)
            {
                *row_ptr = get_good_segment_number(seg_num);
            }
            row_ptr++;
        }
    }

    verbose_pso(2, "Updating the segment map took %ldms.\n",
                get_cpu_time() - initial_cpu_time);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_boundaries_of_segments
(
    Segmentation_t2* segment_info_ptr,
    int           num_boundary_pixels,
    Cached_pixel* pixel_buff
)
{

    int i, p;
    int seg_num;
    int num_segments = segment_info_ptr->num_segments;
    Segment_t2* seg_ptr;
    long initial_cpu_time = get_cpu_time();


    for (i=0; i<num_boundary_pixels; i++)
    {
        ERE(seg_num = get_good_segment_number(pixel_buff[ i ].seg_num));

        ASSERT(seg_num >= 0);

        if (seg_num > 0)
        {
            seg_ptr = segment_info_ptr->segments[ seg_num - 1 ];
            (seg_ptr->num_boundary_pixels)++;
        }
    }

    for (i=0; i<num_segments; i++)
    {
        seg_ptr = segment_info_ptr->segments[ i ];

        NRE(seg_ptr->boundary_pixels =
                    N_TYPE_MALLOC(Pixel_info, seg_ptr->num_boundary_pixels));
    }

    for (i=0; i<num_boundary_pixels; i++)
    {
        ERE(seg_num = get_good_segment_number(pixel_buff[ i ].seg_num));

        ASSERT(seg_num >= 0);

        if (seg_num > 0)
        {
            seg_ptr = segment_info_ptr->segments[ seg_num - 1 ];
            p = seg_ptr->boundary_pixel_count;

            seg_ptr->boundary_pixels[ p ].i = cached_pixels[ i ].i;
            seg_ptr->boundary_pixels[ p ].j = cached_pixels[ i ].j;

            (seg_ptr->boundary_pixel_count)++;

            ASSERT(seg_ptr->boundary_pixel_count <= seg_ptr->num_boundary_pixels);
        }
    }

    verbose_pso(5, "Finding boundary pixels of segments took %ldms.\n",
                get_cpu_time() - initial_cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_interior_points(Segmentation_t2* image_segment_info_ptr)
{
    Segment_t2** segments     = image_segment_info_ptr->segments;
    int             num_segments = image_segment_info_ptr->num_segments;
    int**           seg_map      = image_segment_info_ptr->seg_map;
    int             i;
    long            cpu_time     = get_cpu_time();
    int             seg_num;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    /*
    // For each segment, approach top to bottem through CM.  When we hit it,
    // follow the boundary to get the outside boundary.
    */
    for (seg_num = 1; seg_num <= num_segments; seg_num++)
    {
        Segment_t2* seg_ptr = segments[ seg_num - 1 ];
        int            j_inside    = seg_ptr->j_CM;
        int            start_i     = NOT_SET;
        int            end_i;


        seg_ptr->j_inside = j_inside;

        ASSERT(seg_ptr->segment_number == seg_num);
        ASSERT(seg_ptr->i_min >= 0);
        ASSERT(seg_ptr->j_min >= 0);
        ASSERT(seg_ptr->i_max < image_segment_info_ptr->num_rows);
        ASSERT(seg_ptr->j_max < image_segment_info_ptr->num_cols);
        ASSERT(seg_ptr->i_CM >= seg_ptr->i_min);
        ASSERT(seg_ptr->j_CM >= seg_ptr->j_min);
        ASSERT(seg_ptr->i_CM <= seg_ptr->i_max);
        ASSERT(seg_ptr->j_CM <= seg_ptr->j_max);

#ifdef TEST
        verbose_pso(30, "Segment_t2 %d i is in (%d, %d), j is in (%d, %d) ",
                    seg_num,
                    seg_ptr->i_min, seg_ptr->i_max,
                    seg_ptr->j_min, seg_ptr->j_max);
        verbose_pso(30, "and CM is (%.2f, %.2f).\n",
                    seg_ptr->i_CM, seg_ptr->j_CM);
#endif

        for (i = seg_ptr->i_min; i <= seg_ptr->i_max; i++)
        {
            if (seg_map[ i ][ j_inside ] == seg_num)
            {
                start_i = i;
                break;
            }
        }

        if (start_i == NOT_SET)
        {
            verbose_pso(1, "Unable to find interior point for segment %d.\n",
                        seg_num);
            seg_ptr->i_inside = seg_ptr->i_CM;
        }
        else
        {
            int best_start = start_i;
            int best_length = 0;

            i = start_i;

            while (i < seg_ptr->i_max)
            {
                int length = 0;
                int cur_start;

                while (    (seg_map[ i ][ j_inside ] != seg_num)
                        && (i < seg_ptr->i_max)
                      )
                {
                    i++;
                }

                if (i == seg_ptr->i_max)
                {
                    break;
                }

                cur_start = i;

                while (    (seg_map[ i ][ j_inside ] == seg_num)
                        && (i < seg_ptr->i_max)
                      )
                {
                    i++;
                }

                length = i - cur_start;

                if (i == seg_ptr->i_max)
                {
                    length++;
                }

                if (length > best_length)
                {
                    best_length = length;
                    best_start  = cur_start;
                }
            }

            end_i = seg_ptr->i_max;

            ASSERT(seg_map[ best_start ][ j_inside ] == seg_num);

            start_i = best_start;

            for (i = start_i; i <= seg_ptr->i_max; i++)
            {
                if (seg_map[ i ][ j_inside ] != seg_num)
                {
                    end_i = i - 1;
                    break;
                }
            }

            seg_ptr->i_inside = (end_i + start_i) / 2;

            ASSERT(seg_map[ seg_ptr->i_inside ][ seg_ptr->j_inside ]==seg_num);
        }
    }

    verbose_pso(5, "Finding inside points took %ldms.\n",
                get_cpu_time() - cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_outside_boundary
(
    Segmentation_t2* image_segment_info_ptr
)
{
    static int prev_max_num_boundary_points = 0;
    Segment_t2**  segments     = image_segment_info_ptr->segments;
    int        num_segments = image_segment_info_ptr->num_segments;
    int        num_rows     = image_segment_info_ptr->num_rows;
    int        num_cols     = image_segment_info_ptr->num_cols;
    int**      mag_seg_map;
    int        mag_num_rows = 2 * num_rows;
    int        mag_num_cols = 2 * num_cols;
    int        j, k;
    long       cpu_time = get_cpu_time();
    int        seg_num;


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
        Segment_t2* cur_seg_ptr         = segments[ seg_num - 1 ];
        int            num_boundary_pixels = cur_seg_ptr->num_boundary_pixels;
        Pixel_info*    boundary_pixels     = cur_seg_ptr->boundary_pixels;

        verbose_pso(50, "Segment_t2 %d has %d boundary pixels.\n",
                    seg_num, num_boundary_pixels);

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
        Segment_t2* cur_seg_ptr = segments[ seg_num - 1 ];
        int            start_j     = 2 * cur_seg_ptr->j_CM;
        int            ii, jj;
        int            start_i     = NOT_SET;
        int            valid_boundary      = TRUE;
        int            count = 0;
        Boundary_approach approach = SEG_DOWN;
        int ni, si, ei, nj, sj, ej;
        int prev_ii, prev_jj;
        int end_i, end_j;


        if (cur_seg_ptr->num_boundary_pixels > prev_max_num_boundary_points)
        {
            ERE(get_target_int_matrix(&cached_magnified_boundary_points,
                                      4 * cur_seg_ptr->num_boundary_pixels, 2));
            prev_max_num_boundary_points =  cur_seg_ptr->num_boundary_pixels;
        }

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
            verbose_pso(1, "Unable to find start of outside boundary for segment %d.\n",
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
            //
            // IMPORTANT NOTE:
            //    At one point it looked like the move to a magnified view made
            //    the above strategy obsolete, but this is NOT the case. (It
            //    could be the case for 4 connectedness, but it is definately
            //    NOT the case for 8 connectedness).
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

                verbose_pso(1, "Unable to follow outside boundary ");
                verbose_pso(1, "of segment %d.\n", seg_num);
                valid_boundary = FALSE;

                break;

add_pixel:

                if (count >= 4 * cur_seg_ptr->num_boundary_pixels)
                {
                    verbose_pso(1, "Max length of outside boundary exceeded ");
                    verbose_pso(1, "for segment %d.\n", seg_num);
                    valid_boundary = FALSE;

                    break;
                }

                /* dbi(ni); */
                /* dbi(nj);  */

                /* Add to cached pixels. */

                ASSERT(count < mag_num_rows * mag_num_cols);
                ASSERT(count >= 0);

                cached_magnified_boundary_points->elements[ count ][ 0 ] = ni;
                cached_magnified_boundary_points->elements[ count ][ 1 ] = nj;

                count++;


                prev_ii = ii;
                prev_jj = jj;

                ii = ni;
                jj = nj;
            }
            while (    (ii != start_i) || (jj != start_j)
                    || (prev_ii != end_i) || (prev_jj != end_j)
                  );

#define CHECK_FOR_DUPLICATES

            if (valid_boundary)
            {
#ifdef CHECK_FOR_DUPLICATES
                int num_unique = 0;
                int prev_ten_b_i = NOT_SET;
                int prev_ten_b_j = NOT_SET;
#endif
                double** row_ptr;

                ERE(get_target_matrix(&(cur_seg_ptr->outside_boundary_mp),
                                      count, 2));

                row_ptr = cur_seg_ptr->outside_boundary_mp->elements;

                /*
                // The outer boundary is a non-pixel based list of points. If a
                // region's boundary passes through (i,j), then the outer part
                // of the boundary may include any of (i,j), (i+1,j), (i, j+1)
                // and (i+1, j+1) in double values. However, we instead use
                // (i+.1, j+.1), (i+.9, j+.1), (i+.1, j+.9), and (i+.9, j+.9)
                // in order to retain the information about which pixel the
                // outer boundary was from. [ Obviously the offset of .1 is
                // arbitrary, and could be made smaller if need be. ].
                */
                for (k = 0; k<count; k++)
                {
                    int mag_i = cached_magnified_boundary_points->elements[k][0];
                    int mag_j = cached_magnified_boundary_points->elements[k][1];
                    int ten_b_i = 1 + 5 * mag_i + 3 * (mag_i % 2);
                    int ten_b_j = 1 + 5 * mag_j + 3 * (mag_j % 2);

#ifdef CHECK_FOR_DUPLICATES
                    if (    (ten_b_i != prev_ten_b_i)
                         || (ten_b_j != prev_ten_b_j)
                       )
                    {
                        (*row_ptr)[ 0 ] = (double)ten_b_i / 10.0;
                        (*row_ptr)[ 1 ] = (double)ten_b_j / 10.0;

                        row_ptr++;

                        num_unique++;

                        prev_ten_b_j = ten_b_j;
                        prev_ten_b_i = ten_b_i;

                    }
#else
                    (*row_ptr)[ 0 ] = (double)ten_b_i / 10.0;
                    (*row_ptr)[ 1 ] = (double)ten_b_j / 10.0;

                    row_ptr++;
#endif
                }

#ifdef CHECK_FOR_DUPLICATES
                cur_seg_ptr->outside_boundary_mp->num_rows = num_unique;

                if (count - num_unique > 0)
                {
                    verbose_pso(50, "Outside boundary of region %d had %d duplicates.\n",
                                seg_num, count - num_unique);
                }
#endif

                verbose_pso(20, "Segment_t2 %d has %d outside boundary points.\n",
                            seg_num, count);
            }
            else
            {
                verbose_pso(1, "Unable to get an outside boundary for segment %d.\n",
                            seg_num);
            }
        }
    }

    verbose_pso(5, "Finding outside boundary took %ldms.\n",
                get_cpu_time() - cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_neighbours(Segmentation_t2* image_segment_info_ptr)
{
    static int prev_num_segments = 0;
    int**      seg_map      = image_segment_info_ptr->seg_map;
    int        num_segments = image_segment_info_ptr->num_segments;
    int        num_rows     = image_segment_info_ptr->num_rows;
    int        num_cols     = image_segment_info_ptr->num_cols;
    Segment_t2**  image_segment_ptr = image_segment_info_ptr->segments;
    int        seg_num;
    int        j, k;
    long       cpu_time = get_cpu_time();


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

    /*
    // For each boundary point, see if it is near another segment.
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
                         && ((new_nb = seg_map[ i_offset ][ j_offset ]) > 0)
                         && (new_nb != seg_num)
                       )
                    {
                        int skip = FALSE;

                        if (new_nb == ERROR) return ERROR;

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
#ifdef TEST
            verbose_pso(50, "seg %3d : ", seg_num);
#endif

            NRE(image_segment_ptr[ seg_num - 1 ]->neighbours = INT_MALLOC(num_neighbours));

            for (k = 0; k < num_neighbours; k++)
            {
                image_segment_ptr[ seg_num - 1 ]->neighbours[ k ]= cached_neighbours[ k ];

#ifdef TEST
                verbose_pso(50, "%d, ", cached_neighbours[ k ]);
#endif
            }

            image_segment_ptr[ seg_num - 1 ]->num_neighbours = num_neighbours;

#ifdef TEST
            verbose_pso(50, "\n");
#endif
        }
    }

    verbose_pso(5, "Finding neigbours took %ldms.\n",
                get_cpu_time() - cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
static int verify_segmentation(Segmentation_t2* image_segment_info_ptr)
{
    Segment_t2** segments     = image_segment_info_ptr->segments;
    int             num_segments = image_segment_info_ptr->num_segments;
    int**           seg_map      = image_segment_info_ptr->seg_map;
    int num_rows = image_segment_info_ptr->num_rows;
    int num_cols = image_segment_info_ptr->num_cols;
    int             i, j;
    long            cpu_time     = get_cpu_time();
    int             seg_num;
    int  total_num_good_pixels = 0;
    int  num_positive_seg_nums = 0;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    for (seg_num = 1; seg_num <= num_segments; seg_num++)
    {
        Segment_t2* seg_ptr  = segments[ seg_num - 1 ];
        int         j_inside = seg_ptr->j_inside;
        int         i_inside = seg_ptr->i_inside;


        ASSERT(seg_ptr->segment_number == seg_num);
        ASSERT(seg_ptr->i_min >= 0);
        ASSERT(seg_ptr->j_min >= 0);
        ASSERT(seg_ptr->i_max < image_segment_info_ptr->num_rows);
        ASSERT(seg_ptr->j_max < image_segment_info_ptr->num_cols);
        ASSERT(seg_ptr->i_CM >= seg_ptr->i_min);
        ASSERT(seg_ptr->j_CM >= seg_ptr->j_min);
        ASSERT(seg_ptr->i_CM <= seg_ptr->i_max);
        ASSERT(seg_ptr->j_CM <= seg_ptr->j_max);
        ASSERT(seg_ptr->i_inside >= seg_ptr->i_min);
        ASSERT(seg_ptr->j_inside >= seg_ptr->j_min);
        ASSERT(seg_ptr->i_inside <= seg_ptr->i_max);
        ASSERT(seg_ptr->j_inside <= seg_ptr->j_max);

        ASSERT(seg_ptr->num_pixels >= seg_min_segment_size);
        ASSERT(seg_ptr->num_pixels = count_connected_pixels(
                                                        image_segment_info_ptr,
                                                        i_inside, j_inside));

        for (j=0; j<seg_ptr->num_pixels; j++)
        {
            int k;
            int ii = seg_ptr->pixels[ j ].i;
            int jj = seg_ptr->pixels[ j ].j;
            int image_boundary_connect_count = 0;
            int non_corner_connect_count = 0;
            int corner_connect_count = 0;
            int non_connect_count = 0;
            int is_boundary_pixel = FALSE;
            int found_in_boundary = FALSE;
            int di, dj;

            ASSERT(seg_map[ ii ][ jj ] == seg_num);

            for (di=-1; di<=1; di++)
            {
                for (dj=-1; dj<=1; dj++)
                {
                    if ((di != 0) || (dj != 0))
                    {
                        int m = ii + di;
                        int n = jj + dj;

                        if (    (m < 0) || (n < 0)
                             || (m >= num_rows) || (n >= num_cols)
                           )
                        {
                            image_boundary_connect_count++;
                        }
                        else if (seg_map[ m ][ n ] != seg_num)
                        {
                            non_connect_count++;
                        }
                        else if ((di == 0) || (dj == 0))
                        {
                            non_corner_connect_count++;
                        }
                        else
                        {
                            corner_connect_count++;
                        }
                    }
                }
            }

#ifdef MIGHT_BE_OBSOLETE
            /* My currend thinking is that boundary pixelness transcends 4 or 8
            // connectedness. */

#ifdef CONNECT_CORNER_OPTION
            if (seg_connect_corners)
            {
                if (non_corner_connect_count + corner_connect_count < 8)
                {
                    is_boundary_pixel = TRUE;
                }
            }
            else
            {
                if (non_corner_connect_count < 4)
                {
                    is_boundary_pixel = TRUE;
                }
            }
#else
#ifdef DEFAULT_IS_TO_CONNECT_CORNERS
            if (non_corner_connect_count + corner_connect_count < 8)
            {
                is_boundary_pixel = TRUE;
            }
#else
#endif
            if (non_corner_connect_count < 4)
            {
                is_boundary_pixel = TRUE;
            }
#endif
#else
            if (non_corner_connect_count + corner_connect_count < 8)
            {
                is_boundary_pixel = TRUE;
            }
#endif

            for (k=0; k<seg_ptr->num_boundary_pixels; k++)
            {
                if (    (ii == seg_ptr->boundary_pixels[ k ].i)
                     && (jj == seg_ptr->boundary_pixels[ k ].j)
                   )
                {
                    found_in_boundary = TRUE;
                    break;
                }
            }


            ASSERT(found_in_boundary == is_boundary_pixel);
        }

        /*
        // If we have more than one pixel in a region, then every boundary
        // pixel must be connected to at least one pixel in the region (as
        // it is part of the region, and also must be connected to something
        // else.
        */
        if (seg_ptr->num_pixels > 1)
        {
            for (j=0; j<seg_ptr->num_boundary_pixels; j++)
            {
                int ii = seg_ptr->boundary_pixels[ j ].i;
                int jj = seg_ptr->boundary_pixels[ j ].j;
                int image_boundary_connect_count = 0;
                int non_corner_connect_count = 0;
                int corner_connect_count = 0;
                int non_connect_count = 0;
                int di, dj;

                ASSERT(seg_map[ ii ][ jj ] == seg_num);

                    for (di=-1; di<=1; di++)
                    {
                        for (dj=-1; dj<=1; dj++)
                        {
                            if ((di != 0) || (dj != 0))
                            {
                                int m = ii + di;
                                int n = jj + dj;

                                if (    (m < 0) || (n < 0)
                                     || (m >= num_rows) || (n >= num_cols)
                                   )
                                {
                                    image_boundary_connect_count++;
                                }
                                else if (seg_map[ m ][ n ] != seg_num)
                                {
                                    non_connect_count++;
                                }
                                else if ((di == 0) || (dj == 0))
                                {
                                    non_corner_connect_count++;
                                }
                                else
                                {
                                    corner_connect_count++;
                                }
                            }
                        }
                    }


#ifdef CONNECT_CORNER_OPTION
                    if (seg_connect_corners)
                    {
                        ASSERT(image_boundary_connect_count + corner_connect_count + non_corner_connect_count > 0);
                    }
                    else
                    {
                        ASSERT(image_boundary_connect_count + non_corner_connect_count > 0);
                    }
#else
#ifdef DEFAULT_IS_TO_CONNECT_CORNERS
                    ASSERT(image_boundary_connect_count + corner_connect_count + non_corner_connect_count > 0);
#else
                    ASSERT(image_boundary_connect_count + non_corner_connect_count > 0);
#endif
#endif
                }
        }

        total_num_good_pixels += seg_ptr->num_pixels;
    }

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            if (seg_map[ i ][ j ] > 0)
            {
                num_positive_seg_nums++;
            }
        }
    }

    ASSERT(total_num_good_pixels == num_positive_seg_nums);

    verbose_pso(5, "Verifying segmentation took %ldms.\n",
                get_cpu_time() - cpu_time);

    return NO_ERROR;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST

static int count_connected_pixels
(
    Segmentation_t2* image_segment_info_ptr,
    int           i,
    int           j
)
{
    int** seg_map     = image_segment_info_ptr->seg_map;
    int   num_rows    = image_segment_info_ptr->num_rows;
    int   num_cols    = image_segment_info_ptr->num_cols;
    int   cur_seg     = seg_map[ i ][ j ];
    int   count;
    int   count_2;
    int   check_sum   = 0;
    int   check_sum_2 = 0;
    int   ii, jj;


    ASSERT(cached_seg_map == seg_map);
    ASSERT(cached_num_rows == num_rows);
    ASSERT(cached_num_cols == num_cols);

    for (ii=0; ii<num_rows; ii++)
    {
        for (jj=0; jj<num_cols; jj++)
        {
            check_sum += seg_map[ ii ][ jj ];
        }
    }

    count_remark = 0;

    remark_segment(i, j, -10000000);
    count = count_remark;
    count_remark = 0;
    remark_segment(i, j, cur_seg);
    count_2 = count_remark;
    count_remark = 0;

    ASSERT(count == count_2);

    for (ii=0; ii<num_rows; ii++)
    {
        for (jj=0; jj<num_cols; jj++)
        {
            check_sum_2 += seg_map[ ii ][ jj ];
        }
    }

    ASSERT(check_sum == check_sum_2);

    return count;

}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void get_initial_segmentation_helper(int i, int j)
{
    float R, G, B;
    float sum_RGB;
    float r_chrom;
    float g_chrom;
    int   num_reseg;
    int   reseg;

    num_reseg = 1;

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
        num_reseg += seg_resegment_level;
    }

    for (reseg = 0; reseg < num_reseg; reseg++)
    {
        if (reseg != 0)
        {
            int k;

            R = cached_R_sum / num_cached_pixels;
            G = cached_G_sum / num_cached_pixels;
            B = cached_B_sum / num_cached_pixels;
            r_chrom = cached_r_chrom_sum / num_cached_pixels;
            g_chrom = cached_g_chrom_sum / num_cached_pixels;
            sum_RGB = R + G + B + RGB_SUM_EPSILON;

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

        cached_r_chrom_sum = 0.0;
        cached_g_chrom_sum = 0.0;
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

#ifdef TEST
        if (reseg != 0)
        {
            verbose_pso(150, "Re-");
        }
        verbose_pso(150, "Growing (%-3d, %-3d) ... ", i, j);
#endif

        (void)grow_segment(i, j);

#ifdef TEST
        verbose_pso(150, "%d pixels.\n", num_cached_pixels);
#endif

        if (num_cached_pixels < seg_min_resegment_size)
        {
            break;
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef AS_IT_WAS

static void grow_segment(int i, int j)
{
#ifdef CONNECT_CORNER_OPTION
    extern int seg_connect_corners;
#endif
    extern int   cached_segment_count;
    extern int   num_cached_pixels;
    extern double cached_R_sum;
    extern double cached_G_sum;
    extern double cached_B_sum;
    extern double cached_g_chrom_sum;
    extern double cached_r_chrom_sum;
    extern int**              cached_seg_map;
    extern const KJB_image* cached_ip;
    extern float              cached_R_min;
    extern float              cached_R_max;
    extern float              cached_G_min;
    extern float              cached_B_max;
    extern float              cached_B_min;
    extern float              cached_G_max;
    extern float              cached_r_chrom_min;
    extern float              cached_r_chrom_max;
    extern float              cached_g_chrom_min;
    extern float              cached_g_chrom_max;
    extern float              cached_sum_RGB_min;
    extern float              cached_sum_RGB_max;
    extern float              max_abs_chrom_diff;
    extern float              max_rel_chrom_diff;
    extern float              max_abs_sum_RGB_diff;
    extern float              max_rel_sum_RGB_diff;
    extern float              max_abs_RGB_sqr_diff;
    extern float              max_rel_RGB_sqr_diff;
    int                       di, dj;
    int                       i_offset;
    int                       j_offset;
    float                     cur_R;
    float                     cur_G;
    float                     cur_B;
    float                     cur_r_chrom;
    float                     cur_g_chrom;
    float                     cur_sum_RGB;
    float                     cur_RGB_sqr;
    Pixel               cur_image_pixel;


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

    cached_r_chrom_sum += cur_r_chrom;
    cached_g_chrom_sum += cur_g_chrom;
    cached_R_sum += cur_R;
    cached_G_sum += cur_G;
    cached_B_sum += cur_B;

    num_cached_pixels++;

    ASSERT(num_cached_pixels <= cached_num_rows * cached_num_cols);

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
                        grow_segment(i_offset, j_offset);
                    }
                }
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int grow_segment_using_rg_var_and_lum_diff(int i, int j)
{
#ifdef CONNECT_CORNER_OPTION
    extern int seg_connect_corners;
#endif
    extern int   cached_segment_count;
    extern int   num_cached_pixels;
    extern double cached_R_sum;
    extern double cached_G_sum;
    extern double cached_B_sum;
    extern double cached_g_chrom_sum;
    extern double cached_r_chrom_sum;
    extern int   cached_num_cols;
    extern int   cached_num_rows;
    extern int**              cached_seg_map;
    extern const KJB_image* cached_ip;
    extern float              cached_r_chrom_min;
    extern float              cached_r_chrom_max;
    extern float              cached_g_chrom_min;
    extern float              cached_g_chrom_max;
    extern float              max_abs_sum_RGB_diff;
    extern float              max_rel_sum_RGB_diff;
    int                       di, dj;
    int                       i_offset;
    int                       j_offset;
    float                     cur_R;
    float                     cur_G;
    float                     cur_B;
    float                     cur_sum_RGB;
    float                     cur_r_chrom;
    float                     cur_g_chrom;
    Pixel               cur_image_pixel;


    UNTESTED_CODE();

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

    cached_R_sum += cur_R;
    cached_G_sum += cur_G;
    cached_B_sum += cur_B;
    cached_r_chrom_sum += cur_r_chrom;
    cached_g_chrom_sum += cur_g_chrom;

    num_cached_pixels++;

    cached_seg_map[ i ][ j ] = cached_segment_count;

    /* The code here be kept consistent with that in grow_segment. */

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
                        (void)grow_segment_using_rg_var_and_lum_diff(i_offset,
                                                                     j_offset);
                    }
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#else

static int grow_segment(int i, int j)
{

#ifdef CONNECT_CORNER_OPTION
    if (seg_connect_corners)
    {
        grow_segment_8(i, j);
    }
    else
    {
        grow_segment_4(i, j);
    }
#else
#ifdef DEFAULT_IS_TO_CONNECT_CORNERS
    grow_segment_8(i, j);
#endif
    grow_segment_4(i, j);
#endif
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int grow_segment_using_rg_var_and_lum_diff(int i, int j)
{

#ifdef CONNECT_CORNER_OPTION
    if (seg_connect_corners)
    {
        grow_segment_using_rg_var_and_lum_diff_8(i, j);
    }
    else
    {
        grow_segment_using_rg_var_and_lum_diff_4(i, j);
    }
#else
#ifdef DEFAULT_IS_TO_CONNECT_CORNERS
    grow_segment_using_rg_var_and_lum_diff_8(i, j);
#endif
    grow_segment_using_rg_var_and_lum_diff_4(i, j);
#endif

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_segment_4(int i, int j)
{
    int   count;
    int   d;
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

    cached_r_chrom_sum += cur_r_chrom;
    cached_g_chrom_sum += cur_g_chrom;
    cached_R_sum += cur_R;
    cached_G_sum += cur_G;
    cached_B_sum += cur_B;

    num_cached_pixels++;

    ASSERT(num_cached_pixels <= cached_num_rows * cached_num_cols);

    cached_seg_map[ i ][ j ] = cached_segment_count;

    for (count = 0; count < 2; count++)
    {
        for (d = -1; d <= 1; d += 2)
        {
            if (count == 0)
            {
                i_offset = i;
                j_offset = j + d;
            }
            else
            {
                i_offset = i + d;
                j_offset = j;
            }

            if (    (i_offset >= 0)
                 && (j_offset >= 0)
                 && (i_offset < cached_num_rows)
                 && (j_offset < cached_num_cols)
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
                    (void)grow_segment(i_offset, j_offset);
                }
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_segment_using_rg_var_and_lum_diff_4(int i, int j)
{
    int   count;
    int   d;
    int   i_offset;
    int   j_offset;
    float cur_R;
    float cur_G;
    float cur_B;
    float cur_sum_RGB;
    float cur_r_chrom;
    float cur_g_chrom;
    Pixel cur_image_pixel;


    UNTESTED_CODE();

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

    cached_R_sum += cur_R;
    cached_G_sum += cur_G;
    cached_B_sum += cur_B;
    cached_r_chrom_sum += cur_r_chrom;
    cached_g_chrom_sum += cur_g_chrom;

    num_cached_pixels++;

    cached_seg_map[ i ][ j ] = cached_segment_count;

    /* The code here be kept consistent with that in grow_segment. */

    for (count = 0; count < 2; count++)
    {
        for (d = -1; d <= 1; d += 2)
        {
            if (count == 0)
            {
                i_offset = i;
                j_offset = j + d;
            }
            else
            {
                i_offset = i + d;
                j_offset = j;
            }

            if (    (i_offset >= 0)
                 && (j_offset >= 0)
                 && (i_offset < cached_num_rows)
                 && (j_offset < cached_num_cols)
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
                    (void)grow_segment_using_rg_var_and_lum_diff(i_offset,
                                                                 j_offset);
                }
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_segment_8(int i, int j)
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

    cached_r_chrom_sum += cur_r_chrom;
    cached_g_chrom_sum += cur_g_chrom;
    cached_R_sum += cur_R;
    cached_G_sum += cur_G;
    cached_B_sum += cur_B;

    num_cached_pixels++;

    ASSERT(num_cached_pixels <= cached_num_rows * cached_num_cols);

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
                        grow_segment(i_offset, j_offset);
                    }
                }
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_segment_using_rg_var_and_lum_diff_8(int i, int j)
{
    int   di, dj;
    int   i_offset;
    int   j_offset;
    float cur_R;
    float cur_G;
    float cur_B;
    float cur_sum_RGB;
    float cur_r_chrom;
    float cur_g_chrom;
    Pixel cur_image_pixel;


    UNTESTED_CODE();

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

    cached_R_sum += cur_R;
    cached_G_sum += cur_G;
    cached_B_sum += cur_B;
    cached_r_chrom_sum += cur_r_chrom;
    cached_g_chrom_sum += cur_g_chrom;

    num_cached_pixels++;

    cached_seg_map[ i ][ j ] = cached_segment_count;

    /* The code here be kept consistent with that in grow_segment. */

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
                        (void)grow_segment_using_rg_var_and_lum_diff(i_offset,
                                                                     j_offset);
                    }
                }
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#endif

/*
// Currently, we only use this for small segments. If we use it for large
// segments, then we would want to make new_seg_num global.
*/
static void remark_segment(int i, int j, int new_seg_num)
{
    int cur_seg = cached_seg_map[ i ][ j ];
    int di, dj;


    cached_seg_map[ i ][ j ] = new_seg_num;

#ifdef TEST
    count_remark++;
#endif


    /* The code here must mimic that in grow_segment. */
    for (di = -1; di <= 1; di++)
    {
        for (dj = -1; dj <= 1; dj++)
        {
            if ((di != 0) || (dj != 0))
            {
                int i_offset = i + di;
                int j_offset = j + dj;

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
                     && (cached_seg_map[ i_offset ][ j_offset ] == cur_seg)
                   )
                {
                    remark_segment(i_offset, j_offset, new_seg_num);
                }
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_good_segment_number(int seg_num)
{

    ERE(seg_num = chase_initial_segment_numbers(seg_num));

    return cached_good_segment_numbers[ seg_num];
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int chase_initial_segment_numbers(int seg_num)
{
    int count = 0;
    int new_num;


    seg_num = MAX_OF(seg_num, 0);

    while ((new_num = cached_initial_segment_numbers[ seg_num ]) != seg_num)
    {
        seg_num = new_num;

        count++;

        if (count > 10000)
        {
            set_bug("Likely infinite loop in chase_initial_segment_numbers.");
            return ERROR;
        }
    }

    return new_num;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void zero_seg_map(const KJB_image* ip, int** seg_map)
{
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int i, j;
#ifdef TEST
    int error_count = 0;
#endif


    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            if (ip->pixels[ i ][ j ].extra.invalid.pixel)
            {
                seg_map[ i ][ j ] = ERROR;
#ifdef TEST
                error_count++;
#endif
            }
            else
            {
                seg_map[ i ][ j ] = 0;
            }
        }
    }

#ifdef TEST
    verbose_pso(2, "Zeroing image with %d error pixels.\n", error_count);
#endif


}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_caches(int num_rows, int num_cols)
{
    static int prev_num_rows         = 0;
    static int prev_num_cols         = 0;
    static int prev_max_num_segments;
    int        prev_num_pixels       = prev_num_rows * prev_num_cols;
    int        num_pixels            = num_rows * num_cols;
    int        max_num_segments;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (num_pixels > prev_num_pixels)
    {
        verbose_pso(2, "Bumping one dimensional caches from %,d to %,d.\n",
                    prev_num_pixels, num_pixels);

        kjb_free(cached_pixels);
        NRE(cached_pixels = N_TYPE_MALLOC(Cached_pixel, 4 * num_pixels));
    }

    max_num_segments = num_pixels / MAX_OF(1, seg_min_initial_segment_size);

    if (max_num_segments > prev_max_num_segments)
    {
        verbose_pso(2, "Bumping segment oriented caches from %,d to %,d.\n",
                    prev_max_num_segments, max_num_segments);

        kjb_free(cached_pixel_counts);
        kjb_free(cached_initial_segment_numbers);
        kjb_free(cached_good_segment_numbers);
        kjb_free(cached_R_means);
        kjb_free(cached_G_means);
        kjb_free(cached_B_means);
        kjb_free(cached_r_chrom_means);
        kjb_free(cached_g_chrom_means);
        kjb_free(cached_sum_RGB_means);

        NRE(cached_pixel_counts = INT_MALLOC(max_num_segments));
        NRE(cached_initial_segment_numbers = INT_MALLOC(1 + max_num_segments));
        NRE(cached_good_segment_numbers = INT_MALLOC(1 + max_num_segments));
        NRE(cached_R_means = FLT_MALLOC(max_num_segments));
        NRE(cached_G_means = FLT_MALLOC(max_num_segments));
        NRE(cached_B_means = FLT_MALLOC(max_num_segments));
        NRE(cached_r_chrom_means = FLT_MALLOC(max_num_segments));
        NRE(cached_g_chrom_means = FLT_MALLOC(max_num_segments));
        NRE(cached_sum_RGB_means = FLT_MALLOC(max_num_segments));
    }

    prev_max_num_segments = max_num_segments;
    prev_num_rows = num_rows;
    prev_num_cols = num_cols;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_target_image_segment_info
(
    Segmentation_t2** image_seg_info_ptr_ptr,
    int            num_rows,
    int            num_cols
)
{
    Segmentation_t2* image_seg_info_ptr = *image_seg_info_ptr_ptr;


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

static Segmentation_t2* create_image_segment_info(void)
{
    Segmentation_t2* image_segment_info_ptr;

    NRN(image_segment_info_ptr = TYPE_MALLOC(Segmentation_t2));

    image_segment_info_ptr->segments = NULL;
    image_segment_info_ptr->seg_map = NULL;

    return image_segment_info_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Segment_t2* create_image_segment
(
    int segment_number,
    int num_pixels
)
{
    Segment_t2*     seg_ptr;


    NRN(seg_ptr = TYPE_MALLOC(Segment_t2));

    seg_ptr->segment_number = segment_number;

    seg_ptr->R_ave = DBL_NOT_SET;
    seg_ptr->G_ave = DBL_NOT_SET;
    seg_ptr->B_ave = DBL_NOT_SET;

    seg_ptr->r_chrom_ave = DBL_NOT_SET;
    seg_ptr->g_chrom_ave = DBL_NOT_SET;
    seg_ptr->sum_RGB_ave = DBL_NOT_SET;

    seg_ptr->pixels = N_TYPE_MALLOC(Pixel_info, num_pixels);

    if (seg_ptr->pixels == NULL)
    {
        kjb_free(seg_ptr);
        return NULL;
    }

    seg_ptr->num_pixels = num_pixels;
    seg_ptr->pixel_count = 0;

    seg_ptr->num_boundary_pixels = 0;
    seg_ptr->boundary_pixel_count = 0;
    seg_ptr->boundary_pixels = NULL;

    seg_ptr->outside_boundary_mp = NULL;

    seg_ptr->num_neighbours = 0;
    seg_ptr->neighbours = NULL;

    seg_ptr->i_min = NOT_SET;
    seg_ptr->i_max = NOT_SET;

    seg_ptr->j_min = NOT_SET;
    seg_ptr->j_max = NOT_SET;

    seg_ptr->i_inside = NOT_SET;
    seg_ptr->j_inside = NOT_SET;

    seg_ptr->i_CM = 0.0;
    seg_ptr->j_CM = 0.0;

    seg_ptr->first_moment = DBL_NOT_SET;
    seg_ptr->second_moment = DBL_NOT_SET;

    seg_ptr->boundary_len = DBL_NOT_SET;
    seg_ptr->outside_boundary_len = DBL_NOT_SET;

    return seg_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void t2_free_segmentation(Segmentation_t2* image_segment_info_ptr)
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

static void free_image_segment(Segment_t2* segment_ptr)
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


    kjb_free(cached_neighbours);

#ifdef IMPLEMENT_MERGING
    free_2D_int_array(merge_counts);
    free_matrix(merge_r_diff_mp);
    free_matrix(merge_g_diff_mp);
    free_matrix(merge_sum_diff_mp);
    kjb_free(segment_indices_after_merging);
#endif

    kjb_free(cached_pixels);
    kjb_free(cached_pixel_counts);
    kjb_free(cached_initial_segment_numbers);
    kjb_free(cached_good_segment_numbers);
    kjb_free(cached_R_means);
    kjb_free(cached_G_means);
    kjb_free(cached_B_means);
    kjb_free(cached_r_chrom_means);
    kjb_free(cached_g_chrom_means);
    kjb_free(cached_sum_RGB_means);
    free_int_matrix(cached_magnified_boundary_points);

    free_int_matrix(cached_magnified_seg_map);

}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

