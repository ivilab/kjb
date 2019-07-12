
/* $Id: t3_segment.c 21596 2017-07-30 23:33:36Z kobus $ */

/*
     Copyright (c) 1994-2008 by Kobus Barnard (author).

     Personal and educational use of this code is granted, provided
     that this header is kept intact, and that the authorship is not
     misrepresented. Commercial use is not permitted.
*/

#include "t3/t3_gen.h"     /* Only safe as first include in a ".c" file. */
#include "t3/t3_segment.h"

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
// Expand after merging requires an updated segment map of sorts. Basically, we
// need to back off all regions which are not big enough after merging. This
// requires breaking up get_segment_pixels().
//
#define DO_NEGATIVE_SEGMENTS
#define COLLECT_PIXELS_DURING_INITIAL_SEG
#define FIND_BEST_EXPANSION_HOME
#define EXPAND_RANDOMLY
*/

#define GROW_RANDOMLY
#define USE_RANDOM_SEEDING
/*
*/
#define USE_NO_VALUE
#define CONNECT_CORNER_OPTION


/*
*/

#define EXPAND_AFTER_MERGING
#define MERGE_SMALL_PAIRS
/*
#define REMARK_SMALL_REGIONS_AS_ZERO
#define WEIGHT_SMOOTHING
*/

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
    int         i;
    int         j;
    int         seg_num;
}
Cached_pixel;

/* -------------------------------------------------------------------------- */

#define D1 {-1, 0}
#define D2 { 1, 0}
#define D3 {0, -1}
#define D4 {0, 1}

/* -------------------------------------------------------------------------- */

static int set_segmentation_options_2
(
    const char* option,
    const char* value
);

#ifdef DO_NEGATIVE_SEGMENTS
static int do_negative_segments
(
    const KJB_image* ip,
    Segmentation_t3*    segmentation_ptr
);

static void get_negative_segmentation(const KJB_image* ip, int** seg_map);

static void grow_unconstrained_segment(int i, int j);
#endif

static int get_initial_segmentation(const KJB_image* ip, int** seg_map);

static int neighbourhood_is_smooth
(
    const KJB_image* ip,
    int              i,
    int              j,
    int              size
);


#ifdef NOT_USED_YET
static int segment_is_sliver
(
    int**         seg_map,
    int           seg,
    int           num_pixels,
    Cached_pixel* pixels
);
#endif

static void fill_holes(const KJB_image* ip, int** seg_map);

static void erode_segments(const KJB_image* ip, int** seg_map);

static void initial_expand_edges(const KJB_image* ip, int** seg_map);

#ifdef EXPAND_AFTER_MERGING
static void post_merge_expand_edges
(
    const KJB_image* ip,
    int**            seg_map,
    int              count
);
#endif

static void expand_edges
(
    const KJB_image* ip,
    int**            seg_map,
    int              expand_edge_level,
    int              num_edge_expansions
);

static void find_boundary_pixels
(
    const KJB_image* ip,
    int**            seg_map,
    int*             num_pixels_ptr,
    Cached_pixel*    pixel_buff
);

#ifdef HOW_IT_WAS_FOR_A_LONG_TIME
static int merge_segments
(
    const KJB_image* ip,
    int**            seg_map,
    int              num_boundary_pixels,
    Cached_pixel*    boundary_pixels,
    int              num_segments,
    int*             pixel_counts,
    float*           R_means,
    float*           G_means,
    float*           B_means,
    float*           r_chrom_means,
    float*           g_chrom_means,
    float*           sum_RGB_means
);
#endif

static int merge_segments_1
(
    const KJB_image* ip,
    int**            seg_map,
    int              num_boundary_pixels,
    Cached_pixel*    boundary_pixels,
    int              num_segments,
    int*             pixel_counts,
    float*           R_means,
    float*           G_means,
    float*           B_means,
    float*           r_chrom_means,
    float*           g_chrom_means,
    float*           sum_RGB_means
);

static int merge_segments_2
(
    const KJB_image* ip,
    int**            seg_map,
    int              num_boundary_pixels,
    Cached_pixel*    boundary_pixels,
    int              num_segments,
    int*             pixel_counts,
    float*           R_means,
    float*           G_means,
    float*           B_means,
    float*           r_chrom_means,
    float*           g_chrom_means,
    float*           sum_RGB_means
);

static int merge_small_segment_pairs
(
    const KJB_image* ip,
    int**            seg_map,
    int              num_boundary_pixels,
    Cached_pixel*    boundary_pixels,
    int              num_segments,
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
    Segmentation_t3*    segmentation_ptr,
    int              num_segments,
    int*             pixel_counts,
    float*           R_means,
    float*           G_means,
    float*           B_means,
    float*           r_chrom_means,
    float*           g_chrom_means,
    float*           sum_RGB_means
);

static void update_segment_map(Segmentation_t3* segmentation_ptr);

static int get_boundaries_of_segments
(
    Segmentation_t3* segmentation_ptr,
    int           num_boundary_pixels,
    Cached_pixel* pixel_buff
);

static int find_interior_points(Segmentation_t3* segmentation_ptr);

static int find_outside_boundary(Segmentation_t3* segmentation_ptr);

static int find_neighbours(Segmentation_t3* segmentation_ptr);

#ifdef COMPUTE_EDGE_STRENGTHS
static int get_edge_strengths
(
    const KJB_image* ip,
    Segmentation_t3*    segmentation_ptr
);
#endif

static int  get_good_segment_number(int seg_num);
static int  chase_initial_segment_numbers(int seg_num);
static void get_initial_segmentation_helper(int i, int j);
static int  grow_segment(int i, int j);
static void grow_segment_4(int i, int j);
static void grow_segment_8(int i, int j);
static int  grow_segment_using_rg_and_lum_var(int i, int j);
static void grow_segment_using_rg_and_lum_var_4(int i, int j);
static void grow_segment_using_rg_and_lum_var_8(int i, int j);

static void remark_segment(int i, int j, int seg_num);

static void zero_seg_map(const KJB_image* ip, int** seg_map);

#ifdef TEST
static int verify_segmentation(Segmentation_t3* segmentation_ptr);

static int count_connected_pixels
(
    Segmentation_t3* segmentation_ptr,
    int           i,
    int           j
);
#endif

static int get_target_segmentation
(
    Segmentation_t3** segmentation_ptr_ptr,
    int            num_rows,
    int            num_cols
);

static Segmentation_t3* create_segmentation(void);

static Segment_t3* create_segment(int segment_number, int num_pixels);

static int copy_segment
(
    Segment_t3**      target_segment_ptr_ptr,
    const Segment_t3* source_segment_ptr
);

static void free_segment(Segment_t3* segment_ptr);

static int initialize_caches
(
    int num_rows,
    int num_cols,
    int num_segments
);

static int update_segment_pair_storage
(
    int         num_segments,
    const char* mess_str
);

static int update_segment_merge_storage
(
    int         num_segments,
    const char* mess_str
);

static int use_initial_segmentation
(
    const KJB_image* ip,
    int**            initial_map,
    int**            seg_map
);

static void use_initial_segmentation_helper(int i, int j);

static void grow_segment_using_initial_map(int i, int j);

static void grow_segment_using_initial_map_4(int i, int j);

static void grow_segment_using_initial_map_8(int i, int j);


#ifdef TRACK_MEMORY_ALLOCATION
    static void free_allocated_static_data(void);
    static void prepare_memory_cleanup(void);
#endif

/* -------------------------------------------------------------------------- */

#ifdef GROW_RANDOMLY
static int fs_direction_permutations_4[ 24 ][ 4 ][ 2 ] =
{
   {D1, D2, D3, D4}, {D2, D1, D3, D4}, {D3, D1, D2, D4}, {D4, D1, D2, D3},
   {D1, D2, D4, D3}, {D2, D1, D4, D3}, {D3, D1, D4, D2}, {D4, D1, D3, D2},
   {D1, D3, D4, D2}, {D2, D3, D4, D1}, {D3, D2, D4, D1}, {D4, D2, D3, D1},
   {D1, D3, D2, D4}, {D2, D3, D1, D4}, {D3, D2, D1, D4}, {D4, D2, D1, D3},
   {D1, D4, D3, D2}, {D2, D4, D3, D1}, {D3, D4, D2, D1}, {D4, D3, D2, D1},
   {D1, D4, D2, D3}, {D2, D4, D1, D3}, {D3, D4, D1, D2}, {D4, D3, D1, D2}
};
#endif

static int fs_max_num_segments  = 0;

#ifdef TEST
static int fs_seg_verify_segmentation = FALSE;
#endif

static int fs_seg_fill_hole_level       = 20;

static int fs_seg_initial_expand_edge_level     = 0;
static int fs_seg_initial_num_edge_expansions   = 0;
static int fs_seg_expand_edge_level     = 0;
static int fs_seg_num_edge_expansions   = 0;
static int fs_seg_num_segment_erosions  = 0;

static int fs_connection_max_step       = 2;
static int fs_min_num_connections       = 20;
static int fs_seg_resegment_level       = 1;

#ifdef CONNECT_CORNER_OPTION
#ifdef DEFAULT_IS_TO_CONNECT_CORNERS
static int fs_seg_connect_corners       = TRUE;
#else
static int fs_seg_connect_corners       = FALSE;
#endif
#endif
static int fs_seg_min_initial_segment_size = 10;
static int fs_seg_min_segment_size      = 50;
static int fs_seg_min_resegment_size    = 10;

static int fs_seg_min_seed_size = 0;
static int fs_seg_max_seed_size = 3;
static int fs_seg_max_smooth_size = 1;

/*
// We set the variance of each channel, but for differences we use the sum of
// squares.
//
// Note that most defaults are very large, effectively making it so that that
// particular threshold is not used.
*/
#ifdef USE_NO_VALUE
static float fs_max_rel_RGB_var = FLT_NOT_SET;
static float fs_max_abs_RGB_var = FLT_NOT_SET;

static float fs_max_rel_sum_RGB_var = 0.30;
static float fs_max_abs_sum_RGB_var = 50;

static float fs_max_rel_chrom_var = FLT_NOT_SET;
static float fs_max_abs_chrom_var = 0.02;

static float fs_max_rel_RGB_sqr_diff = FLT_NOT_SET;
static float fs_max_abs_RGB_sqr_diff = FLT_NOT_SET;

static float fs_max_rel_sum_RGB_diff = FLT_NOT_SET;
static float fs_max_abs_sum_RGB_diff = FLT_NOT_SET;

static float fs_max_rel_chrom_diff = FLT_NOT_SET;
static float fs_max_abs_chrom_diff = FLT_NOT_SET;
#else
static float fs_max_rel_RGB_var = 0.30;
static float fs_max_abs_RGB_var = 50;

static float fs_max_rel_sum_RGB_var = 500.0;
static float fs_max_abs_sum_RGB_var = 50000;

static float fs_max_rel_chrom_var = 100.0;
static float fs_max_abs_chrom_var = 0.02;

static float fs_max_rel_RGB_sqr_diff = 10000;
static float fs_max_abs_RGB_sqr_diff = 1000000;

static float fs_max_rel_sum_RGB_diff = 10;
static float fs_max_abs_sum_RGB_diff = 1000;

static float fs_max_rel_chrom_diff = 100.0;
static float fs_max_abs_chrom_diff = 20.0;
#endif

static float fs_min_sum_RGB_for_chrom_test = 50.0;
static float fs_min_sum_RGB_for_relative_sum_diff = 50.0;

static int  fs_seg_merge_level              = 0;
static int  fs_merge_max_step               = 2;
static int  fs_merge_small_region_size      = 50;
static int  fs_merge_min_num_pixels         = 20;
static int  fs_merge_min_num_connections    = 20;
static double fs_merge_sum_RGB_drift          = 0.30;
static double fs_merge_rg_drift               = 0.30;
static double fs_merge_rg_rms_threshold       = DBL_NOT_SET;
static double fs_merge_sum_RGB_rel_threshold  = .30;
static double fs_merge_max_norm_diff = 0.5;

#ifdef NOT_CURRENTLY_USED
static double fs_merge_sum_RGB_abs_threshold = 30;
static double fs_merge_RGB_diff_ave_threshold = 3;
#endif

static int fs_cached_segment_count = 0;
static int fs_num_cached_pixels;
#ifdef TEST
static int fs_count_remark = 0;
#endif
static Cached_pixel* fs_cached_pixels = NULL;
static int* fs_cached_pixel_counts = NULL;
static int* fs_cached_initial_segment_numbers = NULL;
static int* fs_cached_good_segment_numbers = NULL;
static float* fs_cached_R_means = NULL;
static float* fs_cached_G_means = NULL;
static float* fs_cached_B_means = NULL;
static float* fs_cached_sum_RGB_means = NULL;
static float* fs_cached_r_chrom_means = NULL;
static float* fs_cached_g_chrom_means = NULL;
#ifdef MAINTAIN_SS
static float* fs_cached_sum_RGB_SS = NULL;
static float* fs_cached_r_chrom_SS = NULL;
static float* fs_cached_g_chrom_SS = NULL;
#endif

static Int_matrix*  fs_cached_magnified_boundary_points = NULL;
static Int_matrix*  fs_cached_magnified_seg_map = NULL;
static double fs_cached_R_sum;
static double fs_cached_G_sum;
static double fs_cached_B_sum;
static double fs_cached_r_chrom_sum;
static double fs_cached_g_chrom_sum;
#ifdef MAINTAIN_SS
static double fs_cached_r_chrom_sqr_sum;
static double fs_cached_g_chrom_sqr_sum;
static double fs_cached_sum_RGB_sqr_sum;
#endif

static int   fs_cached_num_rows      = 0;
static int   fs_cached_num_cols      = 0;
static int*  fs_cached_neighbours    = NULL;
static int** fs_data_pair_counts     = NULL;
static int** fs_boundary_pair_counts = NULL;
static int** fs_properly_connected_pairs = NULL;
static int** fs_merge_candidates     = NULL;

static Matrix* fs_merge_r_sum_mp       = NULL;
static Matrix* fs_merge_g_sum_mp       = NULL;
static Matrix* fs_merge_sum_RGM_sum_mp       = NULL;

/* We play it a bit fast and loose to reduce stack use during recursion. */
static KJB_image* fs_cached_ip = NULL;
static int** fs_cached_seg_map;            /* Do NOT free this! */
static float fs_cached_R_min;
static float fs_cached_R_max;
static float fs_cached_G_min;
static float fs_cached_G_max;
static float fs_cached_B_min;
static float fs_cached_B_max;
static float fs_cached_r_chrom_min;
static float fs_cached_r_chrom_max;
static float fs_cached_g_chrom_min;
static float fs_cached_g_chrom_max;
static float fs_cached_sum_RGB_min;
static float fs_cached_sum_RGB_max;

static int**  fs_cached_initial_map = NULL;

static Method_option fs_segmentation_methods[ ] =
{
    { "general",         "g",  (int(*)())grow_segment },
    { "rg-lum-var", "rg-lum",  (int(*)())grow_segment_using_rg_and_lum_var }
};

static const int fs_num_segmentation_methods =
                                sizeof(fs_segmentation_methods) /
                                          sizeof(fs_segmentation_methods[ 0 ]);
static int         fs_segmentation_method                  = 0;
static const char* fs_segmentation_method_short_str= "t3-seg-method";
static const char* fs_segmentation_method_long_str = "t3-segmentation-method";


/* -------------------------------------------------------------------------- */

int t3_set_segmentation_options(const char* option, const char* value)
{
    int result      = NOT_FOUND;
    int temp_result;
    int print_title = FALSE;


    if (    (is_pattern(option))
         && (value != NULL)
         && ((value[ 0 ] == '\0') || (value[ 0 ] == '?'))
       )
    {
        /* Dry run */

        ERE(temp_result = set_segmentation_options_2(option, (char*)NULL));
        if (temp_result == NO_ERROR) result = NO_ERROR;

        if (result == NOT_FOUND) return result;

        print_title = TRUE;

        ERE(pso("\n"));
        ERE(set_high_light(stdout));
        ERE(pso("T3 Segmentation options"));
        ERE(unset_high_light(stdout));
        ERE(pso("\n"));
    }

    result = set_segmentation_options_2(option, value);

    if (print_title)
    {
        ERE(pso("\n"));
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int set_segmentation_options_2
(
    const char* option,
    const char* value
)
{
    char   lc_option[ 100 ];
    int    temp_int;
    double temp_real;
    float  temp_float;
    int    result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, fs_segmentation_method_short_str)
          || match_pattern(lc_option, fs_segmentation_method_long_str)
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(parse_method_option(fs_segmentation_methods, fs_num_segmentation_methods,
                                fs_segmentation_method_long_str,
                                "T3 segmentation method",
                                value, &fs_segmentation_method));
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-fill-hole-level"))
         || (match_pattern(lc_option, "t3-seg-fill-hole-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-fill-hole-level = %d\n",
                    fs_seg_fill_hole_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation hole fill level is %d .\n",
                    fs_seg_fill_hole_level));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_seg_fill_hole_level = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-initial-expand-edge-level"))
         || (match_pattern(lc_option, "t3-seg-initial-expand-edge-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-initial-expand-edge-level = %d\n",
                    fs_seg_initial_expand_edge_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation edges are initially expanded at level %d .\n",
                    fs_seg_initial_expand_edge_level));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_seg_initial_expand_edge_level = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-initial-num-edge-expansions"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-initial-num-edge-expansions = %d\n",
                    fs_seg_initial_num_edge_expansions));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation edges are initially expanded %d times.\n",
                    fs_seg_initial_num_edge_expansions));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_seg_initial_num_edge_expansions = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-expand-edge-level"))
         || (match_pattern(lc_option, "t3-seg-expand-edge-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-expand-edge-level = %d\n",
                    fs_seg_expand_edge_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation edges are expanded at level %d .\n",
                    fs_seg_expand_edge_level));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_seg_expand_edge_level = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-num-edge-expansions"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-num-edge-expansions = %d\n",
                    fs_seg_num_edge_expansions));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation edges are expanded %d times.\n",
                    fs_seg_num_edge_expansions));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_seg_num_edge_expansions = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-num-segment-erosions"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-num-segment-erosions = %d\n",
                    fs_seg_num_segment_erosions));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 segments are eroded %d times.\n",
                    fs_seg_num_segment_erosions));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_seg_num_segment_erosions = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-min-initial-segment-size"))
         || (match_pattern(lc_option, "t3-seg-min-initial-segment-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-min-initial-segment-size = %d\n",
                    fs_seg_min_initial_segment_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation min initial segment size is %d.\n",
                    fs_seg_min_initial_segment_size));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) fs_seg_min_initial_segment_size = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-min-seed-size"))
         || (match_pattern(lc_option, "t3-seg-min-seed-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-min-seed-size = %d\n",
                    fs_seg_min_seed_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation min seed size is %d.\n",
                    fs_seg_min_seed_size));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_seg_min_seed_size = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-max-seed-size"))
         || (match_pattern(lc_option, "t3-seg-max-seed-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-seed-size = %d\n",
                    fs_seg_max_seed_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation max seed size is %d.\n",
                    fs_seg_max_seed_size));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_seg_max_seed_size = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-max-smooth-size"))
         || (match_pattern(lc_option, "t3-seg-max-smooth-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-smooth-size = %d\n",
                    fs_seg_max_smooth_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation max smooth size is %d.\n",
                    fs_seg_max_smooth_size));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_seg_max_smooth_size = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-min-segment-size"))
         || (match_pattern(lc_option, "t3-seg-min-segment-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-min-segment-size = %d\n",
                    fs_seg_min_segment_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation min segment size is %d.\n",
                    fs_seg_min_segment_size));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) fs_seg_min_segment_size = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-resegment-size"))
         || (match_pattern(lc_option, "t3-seg-resegment-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-resegment-size = %d\n",
                    fs_seg_min_resegment_size));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation resegment size is %d.\n",
                    fs_seg_min_resegment_size));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) fs_seg_min_resegment_size = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-min-num-connections"))
         || (match_pattern(lc_option, "t3-seg-min-num-connections"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-min-num-connections = %d\n", fs_min_num_connections));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation min num connections is %d.\n",
                    fs_min_num_connections));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) fs_min_num_connections = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-connection-max-step"))
         || (match_pattern(lc_option, "t3-seg-connection-max-step"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-connection-max-step = %d\n", fs_connection_max_step));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation connection max step is %d.\n",
                    fs_connection_max_step));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) fs_connection_max_step = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-resegment-level"))
         || (match_pattern(lc_option, "t3-seg-resegment-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-resegment-level = %d\n",
                    fs_seg_resegment_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation resegment level is %d.\n",
                    fs_seg_resegment_level));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_seg_resegment_level = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-merge-max-step"))
         || (match_pattern(lc_option, "t3-seg-merge-max-step"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-merge-max-step = %d\n", fs_merge_max_step));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation merge max step is %d.\n",
                    fs_merge_max_step));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            if (temp_int > 0) fs_merge_max_step = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-merge-max-norm-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-merge-max-norm-diff = %.2f\n",
                    fs_merge_max_norm_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation max norm difference for merge is %.2f.\n",
                    fs_merge_max_norm_diff));
        }
        else if (is_no_value_word(value))
        {
            fs_merge_max_norm_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_merge_max_norm_diff = temp_real;
        }

        result = NO_ERROR;
    }

#ifdef NOT_CURRENTLY_USED
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-merge-rgb-diff-ave-threshold"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-merge-rgb-diff-ave-threshold = %.2f\n",
                    fs_merge_RGB_diff_ave_threshold));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation RGB noise estimate for merge threshold is %.2f.\n",
                    fs_merge_RGB_diff_ave_threshold));
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_merge_RGB_diff_ave_threshold = temp_real;
        }

        result = NO_ERROR;
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-merge-rg-drift"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-merge-rg-drift = %.2f\n", fs_merge_rg_drift));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation merge rg drift is %.2f.\n", fs_merge_rg_drift));
        }
        else if (is_no_value_word(value))
        {
            fs_merge_rg_drift = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_merge_rg_drift = temp_real;
        }

        result = NO_ERROR;
    }


    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-merge-rgb-sum-drift"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-merge-rgb-sum-drift = %.2f\n", fs_merge_sum_RGB_drift));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation merge RGB sum drift is %.2f.\n",
                    fs_merge_sum_RGB_drift));
        }
        else if (is_no_value_word(value))
        {
            fs_merge_sum_RGB_drift = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_merge_sum_RGB_drift = temp_real;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-merge-rg-threshold"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-merge-rg-threshold = %.2f\n", fs_merge_rg_rms_threshold));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation merge rg threshold is %.2f.\n",
                    fs_merge_rg_rms_threshold));
        }
        else if (is_no_value_word(value))
        {
            fs_merge_rg_rms_threshold = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_merge_rg_rms_threshold = temp_real;
        }

        result = NO_ERROR;
    }


    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-merge-sum-rel-threshold"))
         || (match_pattern(lc_option, "t3-seg-merge-rgb-sum-rel-threshold"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-merge-rgb-sum-rel-threshold = %.2f\n",
                    fs_merge_sum_RGB_rel_threshold));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation merge RGB sum threshold is %.2f.\n",
                    fs_merge_sum_RGB_rel_threshold));
        }
        else if (is_no_value_word(value))
        {
            fs_merge_sum_RGB_rel_threshold = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_merge_sum_RGB_rel_threshold = temp_real;
        }

        result = NO_ERROR;
    }

#ifdef NOT_CURRENTLY_USED
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-merge-sum-abs-threshold"))
         || (match_pattern(lc_option, "t3-seg-merge-rgb-sum-abs-threshold"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-merge-rgb-sum-abs-threshold = %.2f\n",
                    fs_merge_sum_RGB_abs_threshold));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation merge RGB sum threshold is %.2f.\n",
                    fs_merge_sum_RGB_abs_threshold));
        }
        else if (is_no_value_word(value))
        {
            fs_merge_sum_RGB_abs_threshold = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1d(value, &temp_real));
            fs_merge_sum_RGB_abs_threshold = temp_real;
        }

        result = NO_ERROR;
    }
#endif
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-merge-min-num-pixels"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-merge-min-num-pixels = %d\n",
                    fs_merge_min_num_pixels));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation merge min number of adjacent pixels is %d.\n",
                    fs_merge_min_num_pixels));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_merge_min_num_pixels = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-merge-small-region-size"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-merge-min-num-connections = %d\n",
                    fs_merge_min_num_connections));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation merge min number of connections is %d.\n",
                    fs_merge_min_num_connections));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_merge_min_num_connections = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-merge-level"))
         || (match_pattern(lc_option, "t3-seg-merge-level"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-merge-level = %d\n",
                    fs_seg_merge_level));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation merge level is %d.\n",
                    fs_seg_merge_level));
        }
        else
        {
            ERE(ss1pi(value, &temp_int));
            fs_seg_merge_level = temp_int;
        }

        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-connect-corners"))
         || (match_pattern(lc_option, "t3-seg-connect-corners"))
       )
    {
        int temp_boolean;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-connect-corners = %s\n",
                    fs_seg_connect_corners ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 Segmentation connects corners %s used.\n",
                    fs_seg_connect_corners ? "is" : "is not"));
        }
        else
        {
            ERE(temp_boolean = get_boolean_value(value));
            fs_seg_connect_corners = temp_boolean;
        }

        result = NO_ERROR;
    }

#ifdef USE_NO_VALUE
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_rel_RGB_var < 0.0)
            {
                ERE(pso("t3-seg-max-rel-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-rel-rgb-var = %.3f\n",
                        (double)fs_max_rel_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_rel_RGB_var < FLT_ZERO)
            {
                ERE(pso("Maximum relative RGB variance is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative RGB variance for segmenting is %.3f.\n",
                        (double)fs_max_rel_RGB_var));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_rel_RGB_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_abs_RGB_var < FLT_ZERO)
            {
                ERE(pso("t3-seg-max-abs-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-abs-rgb-var = %.3f\n",
                        (double)fs_max_abs_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_abs_RGB_var < FLT_ZERO)
            {
                ERE(pso("Maximum absolute RGB variance is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute RGB variance for T3 segmenting is %.3f.\n",
                        (double)fs_max_abs_RGB_var));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_abs_RGB_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-sum-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_rel_sum_RGB_var < FLT_ZERO)
            {
                ERE(pso("t3-seg-max-rel-sum-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-rel-sum-rgb-var = %.3f\n",
                        (double)fs_max_rel_sum_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_rel_sum_RGB_var < FLT_ZERO)
            {
                ERE(pso("Maximum relative sum RGB variance is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative sum RGB variance for T3 segmenting is %.3f.\n",
                        (double)fs_max_rel_sum_RGB_var));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_rel_sum_RGB_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_sum_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-sum-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_abs_sum_RGB_var < FLT_ZERO)
            {
                ERE(pso("t3-seg-max-abs-sum-rgb-var = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-abs-sum-rgb-var = %.3f\n",
                        (double)fs_max_abs_sum_RGB_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_abs_sum_RGB_var < FLT_ZERO)
            {
                ERE(pso("Maximum absolute sum RGB variance is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute sum RGB variance for T3 segmenting is %.3f.\n",
                        (double)fs_max_abs_sum_RGB_var));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_abs_sum_RGB_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_sum_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-chrom-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_rel_chrom_var < FLT_ZERO)
            {
                ERE(pso("t3-seg-max-rel-chrom-var = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-rel-chrom-var = %.3f\n",
                        (double)fs_max_rel_chrom_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_rel_chrom_var < FLT_ZERO)
            {
                ERE(pso("Maximum relative chrom variance is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative chrom variance for T3 segmenting is %.3f.\n",
                        (double)fs_max_rel_chrom_var));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_rel_chrom_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_chrom_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-chrom-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_abs_chrom_var < FLT_ZERO)
            {
                ERE(pso("t3-seg-max-abs-chrom-var = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-abs-chrom-var = %.3f\n",
                        (double)fs_max_abs_chrom_var));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_abs_chrom_var < FLT_ZERO)
            {
                ERE(pso("Maximum absolute chrom variance is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute chrom variance for T3 segmenting is %.3f.\n",
                        (double)fs_max_abs_chrom_var));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_abs_chrom_var = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_chrom_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-rgb-sqr-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_rel_RGB_sqr_diff < FLT_ZERO)
            {
                ERE(pso("t3-seg-max-rel-rgb-sqr-diff = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-rel-rgb-sqr-diff = %.3f\n",
                        (double)fs_max_rel_RGB_sqr_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_rel_RGB_sqr_diff < FLT_ZERO)
            {
                ERE(pso("Maximum relative sum RGB sqr difference is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative sum RGB sqr difference for T3 segmenting is %.3f.\n",
                        (double)fs_max_rel_RGB_sqr_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_rel_RGB_sqr_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_RGB_sqr_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-rgb-sqr-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_abs_RGB_sqr_diff < FLT_ZERO)
            {
                ERE(pso("t3-seg-max-abs-rgb-sqr-diff = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-abs-rgb-sqr-diff = %.3f\n",
                        (double)fs_max_abs_RGB_sqr_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_abs_RGB_sqr_diff < FLT_ZERO)
            {
                ERE(pso("Maximum absolute sum RGB sqr difference is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute sum RGB sqr difference for T3 segmenting is %.3f.\n",
                        (double)fs_max_abs_RGB_sqr_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_abs_RGB_sqr_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_RGB_sqr_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-sum-rgb-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_rel_sum_RGB_diff < FLT_ZERO)
            {
                ERE(pso("t3-seg-max-rel-sum-rgb-diff = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-rel-sum-rgb-diff = %.3f\n",
                        (double)fs_max_rel_sum_RGB_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_rel_sum_RGB_diff < FLT_ZERO)
            {
                ERE(pso("Maximum relative sum RGB difference is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative sum RGB difference for T3 segmenting is %.3f.\n",
                        (double)fs_max_rel_sum_RGB_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_rel_sum_RGB_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_sum_RGB_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-sum-rgb-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_abs_sum_RGB_diff < FLT_ZERO)
            {
                ERE(pso("t3-seg-max-abs-sum-rgb-diff = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-abs-sum-rgb-diff = %.3f\n",
                        (double)fs_max_abs_sum_RGB_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_abs_sum_RGB_diff < FLT_ZERO)
            {
                ERE(pso("Maximum absolute sum RGB difference is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute sum RGB difference for T3 segmenting is %.3f.\n",
                        (double)fs_max_abs_sum_RGB_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_abs_sum_RGB_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_sum_RGB_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-chrom-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_rel_chrom_diff < FLT_ZERO)
            {
                ERE(pso("t3-seg-max-rel-chrom-diff = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-rel-chrom-diff = %.3f\n",
                        (double)fs_max_rel_chrom_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_rel_chrom_diff < FLT_ZERO)
            {
                ERE(pso("Maximum relative chrom difference is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum relative chrom difference for segmenting is %.3f.\n",
                        (double)fs_max_rel_chrom_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_rel_chrom_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_chrom_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-chrom-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if (fs_max_abs_chrom_diff < FLT_ZERO)
            {
                ERE(pso("t3-seg-max-abs-chrom-diff = off\n"));
            }
            else
            {
                ERE(pso("t3-seg-max-abs-chrom-diff = %.3f\n",
                        (double)fs_max_abs_chrom_diff));
            }
        }
        else if (value[ 0 ] == '\0')
        {
            if (fs_max_abs_chrom_diff < FLT_ZERO)
            {
                ERE(pso("Maximum absolute chrom difference is not used for T3 segmenting.\n"));
            }
            else
            {
                ERE(pso("Maximum absolute chrom difference for T3 segmenting is %.3f.\n",
                        (double)fs_max_abs_chrom_diff));
            }
        }
        else if (is_no_value_word(value))
        {
            fs_max_abs_chrom_diff = FLT_NOT_SET;
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_chrom_diff = temp_float;
        }
        result = NO_ERROR;
    }
#else
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-rel-rgb-var = %.3f\n",
                    (double)fs_max_rel_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative RGB variance for T3 segmenting is %.3f.\n",
                    (double)fs_max_rel_RGB_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-abs-rgb-var = %.3f\n", (double)fs_max_abs_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute RGB variance for T3 segmenting is %.3f.\n",
                    (double)fs_max_abs_RGB_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-sum-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-rel-sum-rgb-var = %.3f\n",
                    (double)fs_max_rel_sum_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative sum RGB variance for T3 segmenting is %.3f.\n",
                    (double)fs_max_rel_sum_RGB_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_sum_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-sum-rgb-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-abs-sum-rgb-var = %.3f\n",
                    (double)fs_max_abs_sum_RGB_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute sum RGB variance for T3 segmenting is %.3f.\n",
                    (double)fs_max_abs_sum_RGB_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_sum_RGB_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-chrom-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-rel-chrom-var = %.3f\n",
                    (double)fs_max_rel_chrom_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative chrom variance for T3 segmenting is %.3f.\n",
                    (double)fs_max_rel_chrom_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_chrom_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-chrom-var"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-abs-chrom-var = %.3f\n",
                    (double)fs_max_abs_chrom_var));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute chrom variance for T3 segmenting is %.3f.\n",
                    (double)fs_max_abs_chrom_var));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_chrom_var = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-rgb-sqr-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-rel-rgb-sqr-diff = %.3f\n",
                    (double)fs_max_rel_RGB_sqr_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative sum RGB sqr difference for T3 segmenting is %.3f.\n",
                    (double)fs_max_rel_RGB_sqr_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_RGB_sqr_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-rgb-sqr-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-abs-rgb-sqr-diff = %.3f\n",
                    (double)fs_max_abs_RGB_sqr_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute sum RGB sqr difference for T3 segmenting is %.3f.\n",
                    (double)fs_max_abs_RGB_sqr_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_RGB_sqr_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-sum-rgb-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-rel-sum-rgb-diff = %.3f\n",
                    (double)fs_max_rel_sum_RGB_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative sum RGB difference for T3 segmenting is %.3f.\n",
                    (double)fs_max_rel_sum_RGB_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_sum_RGB_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-sum-rgb-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-abs-sum-rgb-diff = %.3f\n",
                    (double)fs_max_abs_sum_RGB_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute sum RGB difference for T3 segmenting is %.3f.\n",
                    (double)fs_max_abs_sum_RGB_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_sum_RGB_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-rel-chrom-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-rel-chrom-diff = %.3f\n",
                    (double)fs_max_rel_chrom_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum relative chrom difference for T3 segmenting is %.3f.\n",
                    (double)fs_max_rel_chrom_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_rel_chrom_diff = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-max-abs-chrom-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-max-abs-chrom-diff = %.3f\n",
                    (double)fs_max_abs_chrom_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Maximum absolute chrom difference for segmenting is %.3f.\n",
                    (double)fs_max_abs_chrom_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_max_abs_chrom_diff = temp_float;
        }
        result = NO_ERROR;
    }
#endif

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-min-rgb-sum-for-chrom-test"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-min-rgb-sum-for-chrom-test = %.3f\n",
                    (double)fs_min_sum_RGB_for_chrom_test));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 segmentation min sum RGB for chrom tests is %.3f.\n",
                    (double)fs_min_sum_RGB_for_chrom_test));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_min_sum_RGB_for_chrom_test = temp_float;
        }
        result = NO_ERROR;
    }

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-seg-min-rgb-sum-for-relative-sum-diff"))
         || (match_pattern(lc_option, "t3-seg-min-rgb-sum-for-relative-diff"))
         || (match_pattern(lc_option, "t3-seg-min-rgb-sum-for-rel-diff"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-min-rgb-sum-for-relative-sum-diff = %.3f\n",
                    (double)fs_min_sum_RGB_for_relative_sum_diff));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 segmentation min sum RGB for relative sum differences is %.3f.\n",
                    (double)fs_min_sum_RGB_for_relative_sum_diff));
        }
        else
        {
            ERE(ss1f(value, &temp_float));
            fs_min_sum_RGB_for_relative_sum_diff = temp_float;
        }
        result = NO_ERROR;
    }

#ifdef TEST
    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "t3-segmentation-verify-segmentation"))
         || (match_pattern(lc_option, "t3-seg-verify-segmentation"))
       )
    {
        int temp_boolean;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("t3-seg-verify-segmentation = %s\n",
                    fs_seg_verify_segmentation ? "t" : "f"));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("T3 segmentations %s verified.\n",
                    fs_seg_verify_segmentation ? "are" : "are not"));
        }
        else
        {
            ERE(temp_boolean = get_boolean_value(value));
            fs_seg_verify_segmentation = temp_boolean;
        }

        result = NO_ERROR;
    }
#endif
    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_resegment_image
(
    const KJB_image*  ip,
    const Int_matrix* seg_map_mp,
    Segmentation_t3**    segmentation_ptr_ptr
)
{
    Segmentation_t3* segmentation_ptr;
    int           i, j;
    int           num_rows = ip->num_rows;
    int           num_cols = ip->num_cols;
    long          initial_cpu_time = get_cpu_time();
    int           count;
    int           max_seg_num  = INT_MIN;
    int           num_segments;
    int**         seg_map;


    if (    (seg_map_mp->num_rows != num_rows)
         || (seg_map_mp->num_cols != num_cols)
       )
    {
        set_error("Region map is not the same size as input image.");
        return ERROR;
    }

    ERE(get_target_segmentation(segmentation_ptr_ptr, num_rows, num_cols));
    segmentation_ptr = *segmentation_ptr_ptr;
    seg_map = segmentation_ptr->seg_map;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            int seg_num = seg_map_mp->elements[ i ][ j ];

            seg_map[ i ][ j ] = seg_num;

            max_seg_num = MAX_OF(max_seg_num, seg_num);
        }
    }

    dbi(max_seg_num);

    num_segments = max_seg_num;

    if (num_segments <= 0)
    {
        set_error("No non-negative region numbers found in region map.");
        return ERROR;
    }

    ERE(initialize_caches(num_rows, num_cols, num_segments));

    for (i=0; i < num_segments; i++)
    {
        fs_cached_initial_segment_numbers[ i ] = i;
        fs_cached_pixel_counts[ i ] = 0;

        fs_cached_R_means[ i ] = 0.0;
        fs_cached_G_means[ i ] = 0.0;
        fs_cached_B_means[ i ] = 0.0;
        fs_cached_r_chrom_means[ i ] = 0.0;
        fs_cached_g_chrom_means[ i ] = 0.0;
        fs_cached_sum_RGB_means[ i ] = 0.0;
    }

    fs_cached_initial_segment_numbers[ num_segments ] = num_segments;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            int seg_num = seg_map[ i ][ j ];

            if (seg_num > 0)
            {
                double R = ip->pixels[ i ][ j ].r;
                double G = ip->pixels[ i ][ j ].g;
                double B = ip->pixels[ i ][ j ].b;
                double sum_RGB = R + G + B;
                double r = R / sum_RGB;
                double g = G / sum_RGB;
                int seg_index = seg_num - 1;


                fs_cached_R_means[ seg_index ] += R;
                fs_cached_G_means[ seg_index ] += G;
                fs_cached_B_means[ seg_index ] += B;

                fs_cached_r_chrom_means[ seg_index ] += r;
                fs_cached_g_chrom_means[ seg_index ] += g;
                fs_cached_sum_RGB_means[ seg_index ] += sum_RGB;

                (fs_cached_pixel_counts[ seg_index ])++;
            }
        }
    }

    find_boundary_pixels(ip, segmentation_ptr->seg_map,
                         &fs_num_cached_pixels, fs_cached_pixels);

    /*
    // Code copied from find_boundary_pixels and modified a bit.
    */

    fs_cached_good_segment_numbers[ 0 ] = 0;

    for (i=0; i < num_segments; i++)
    {
        fs_cached_good_segment_numbers[ i + 1 ] = 0;
    }

    NRE(segmentation_ptr->segments =
                            N_TYPE_MALLOC(Segment_t3*, num_segments));
    segmentation_ptr->num_segments = num_segments;

    /* We want a safe free for the caller if we return without finishing. */
    for (i=0; i<num_segments; i++)
    {
        segmentation_ptr->segments[ i ] = NULL;
    }

    count = 0;

    for (i=0; i<num_segments; i++)
    {
        int num_pixels = fs_cached_pixel_counts[ i ];
        int seg_num = i + 1;
        Segment_t3* seg_ptr;

        NRE(segmentation_ptr->segments[ i ] =
                          create_segment(seg_num, num_pixels));
        seg_ptr = segmentation_ptr->segments[ i ];

        seg_ptr->R_ave = fs_cached_R_means[ i ] / num_pixels;
        seg_ptr->G_ave = fs_cached_G_means[ i ] / num_pixels;
        seg_ptr->B_ave = fs_cached_B_means[ i ] / num_pixels;

        seg_ptr->r_chrom_ave = fs_cached_r_chrom_means[ i ] / num_pixels;
        seg_ptr->g_chrom_ave = fs_cached_g_chrom_means[ i ] / num_pixels;
        seg_ptr->sum_RGB_ave = fs_cached_sum_RGB_means[ i ] / num_pixels;

        fs_cached_good_segment_numbers[ i + 1 ] = seg_num;
    }

    for (i=0; i<num_rows; i++)
    {
        int* row_ptr = seg_map[ i ];

        for (j=0; j<num_cols; j++)
        {
            int seg_num;

            ERE(seg_num = get_good_segment_number(*row_ptr++));

            ASSERT(seg_num >= 0);

            if (seg_num > 0)
            {
                Segment_t3* seg_ptr = segmentation_ptr->segments[ seg_num-1 ];
                int p = seg_ptr->pixel_count;

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

    for (i=0; i<num_segments; i++)
    {
        Segment_t3*seg_ptr = segmentation_ptr->segments[ i ];

        seg_ptr->i_CM /= (seg_ptr->num_pixels);
        seg_ptr->j_CM /= (seg_ptr->num_pixels);

        ASSERT(seg_ptr->pixel_count == seg_ptr->num_pixels);
    }


    ERE(get_boundaries_of_segments(segmentation_ptr, fs_num_cached_pixels,
                                   fs_cached_pixels));

    update_segment_map(segmentation_ptr);

#ifdef DO_NEGATIVE_SEGMENTS
    ERE(do_negative_segments(ip, segmentation_ptr));
#endif

    ERE(find_interior_points(segmentation_ptr));
    ERE(find_outside_boundary(segmentation_ptr));
    ERE(find_neighbours(segmentation_ptr));

#ifdef COMPUTE_EDGE_STRENGTHS
    ERE(get_edge_strengths(ip, segmentation_ptr));
#endif

#ifdef TEST
    if (fs_seg_verify_segmentation)
    {
        ERE(verify_segmentation(segmentation_ptr));
    }
#endif

    verbose_pso(3, "Complete resegmentation has %d segments and took %ldms.\n",
                segmentation_ptr->num_segments,
                get_cpu_time() - initial_cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_segment_image
(
    const KJB_image*  ip,
    const Int_matrix* initial_region_map_mp,
    Segmentation_t3**    segmentation_ptr_ptr
)
{
    Segmentation_t3* segmentation_ptr;
    int           i;
    int           num_rows = ip->num_rows;
    int           num_cols = ip->num_cols;
    long          initial_cpu_time = get_cpu_time();
    int           merge;
    int           merge_res;
    int continue_one = TRUE;
    int continue_two = TRUE;
    int count;


    ERE(get_target_segmentation(segmentation_ptr_ptr, num_rows, num_cols));
    segmentation_ptr = *segmentation_ptr_ptr;

    ERE(initialize_caches(num_rows, num_cols, NOT_SET));

    if (initial_region_map_mp != NULL)
    {
        if (    (initial_region_map_mp->num_rows != num_rows)
             || (initial_region_map_mp->num_cols != num_cols)
           )
        {
            set_error("Region map is not the same size as input image.");
            return ERROR;
        }
        ERE(use_initial_segmentation(ip, initial_region_map_mp->elements,
                                     segmentation_ptr->seg_map));
    }
    else
    {
        ERE(get_initial_segmentation(ip, segmentation_ptr->seg_map));
    }

    dbi(fs_cached_segment_count);

    for (i=0; i <= fs_cached_segment_count; i++)
    {
        fs_cached_initial_segment_numbers[ i ] = i;
    }

    /*
    erode_segments(ip, segmentation_ptr->seg_map);
    */
    initial_expand_edges(ip, segmentation_ptr->seg_map);

    fill_holes(ip, segmentation_ptr->seg_map);

#ifdef HOW_IT_WAS_FOR_A_LONG_TIME
    for (merge = 0; merge < fs_seg_merge_level; merge++)
    {

        find_boundary_pixels(ip, segmentation_ptr->seg_map, &fs_num_cached_pixels,
                             fs_cached_pixels);

        ERE(merge_res = merge_segments(ip, segmentation_ptr->seg_map,
                                       fs_num_cached_pixels, fs_cached_pixels,
                                       fs_cached_segment_count,
                                       fs_cached_pixel_counts,
                                       fs_cached_R_means,
                                       fs_cached_G_means,
                                       fs_cached_B_means,
                                       fs_cached_r_chrom_means,
                                       fs_cached_g_chrom_means,
                                       fs_cached_sum_RGB_means));

        /*
        fill_holes(ip, segmentation_ptr->seg_map);
        */

        if (merge_res == 0)
        {
            verbose_pso(4, "Stopping merging at iteration %d.\n",
                        merge + 1);
            break;
        }
        else
        {
            verbose_pso(5, "%d merges at iteration %d.\n",
                        merge_res, merge + 1);
        }
    }
#endif

    for (count = 0; count < 5; count++)
    {
    for (merge = 0; merge < fs_seg_merge_level; merge++)
    {

        if (continue_one)
        {
            find_boundary_pixels(ip, segmentation_ptr->seg_map, &fs_num_cached_pixels,
                                 fs_cached_pixels);

            ERE(merge_res = merge_segments_1(ip, segmentation_ptr->seg_map,
                                             fs_num_cached_pixels, fs_cached_pixels,
                                             fs_cached_segment_count,
                                             fs_cached_pixel_counts,
                                             fs_cached_R_means,
                                             fs_cached_G_means,
                                             fs_cached_B_means,
                                             fs_cached_r_chrom_means,
                                             fs_cached_g_chrom_means,
                                             fs_cached_sum_RGB_means));

            /*
            fill_holes(ip, segmentation_ptr->seg_map);
            */

            if (merge_res == 0)
            {
                verbose_pso(4, "Stopping merging one at iteration %d.\n",
                            merge + 1);
                continue_one = FALSE;
            }
        }

        if (continue_two)
        {
            find_boundary_pixels(ip, segmentation_ptr->seg_map, &fs_num_cached_pixels,
                                 fs_cached_pixels);

            ERE(merge_res = merge_segments_2(ip, segmentation_ptr->seg_map,
                                             fs_num_cached_pixels, fs_cached_pixels,
                                             fs_cached_segment_count,
                                             fs_cached_pixel_counts,
                                             fs_cached_R_means,
                                             fs_cached_G_means,
                                             fs_cached_B_means,
                                             fs_cached_r_chrom_means,
                                             fs_cached_g_chrom_means,
                                             fs_cached_sum_RGB_means));
            /*
            fill_holes(ip, segmentation_ptr->seg_map);
            */

            if (merge_res == 0)
            {
                verbose_pso(4, "Stopping merging (two) at iteration %d.\n",
                            merge + 1);
                continue_two = FALSE;
            }
        }
    }

    for (merge = 0; merge < fs_seg_merge_level; merge++)
    {

        find_boundary_pixels(ip, segmentation_ptr->seg_map, &fs_num_cached_pixels,
                             fs_cached_pixels);

        ERE(merge_res = merge_small_segment_pairs(ip, segmentation_ptr->seg_map,
                                                  fs_num_cached_pixels,
                                                  fs_cached_pixels,
                                                  fs_cached_segment_count,
                                                  fs_cached_pixel_counts,
                                                  fs_cached_R_means,
                                                  fs_cached_G_means,
                                                  fs_cached_B_means,
                                                  fs_cached_r_chrom_means,
                                                  fs_cached_g_chrom_means,
                                                  fs_cached_sum_RGB_means));


        /*
        fill_holes(ip, segmentation_ptr->seg_map);
        */

        if (merge_res == 0)
        {
            verbose_pso(4, "Stopping merging of small segment pairs at iteration %d.\n",
                        merge + 1);
            break;
        }
        else
        {
            verbose_pso(4, "%d merges of small segment pairs at iteration %d.\n",
                        merge_res, merge + 1);
        }
    }

#ifdef EXPAND_AFTER_MERGING
    /*
    // Need to first break up get_segment_pixels() so that we can back of of
    // segments which are not big enough. Then we do the expansion. Basically
    // we want to update the map, exapnd, and then to the boundaries, etc.
    //
    // BUT HACK IT FOR NOW!!!!
    */
    for (i=0; i<num_rows; i++)
    {
        int j;

        for (j=0; j<num_cols; j++)
        {
            int seg_num = segmentation_ptr->seg_map[ i ][ j ];
            int seg_index = chase_initial_segment_numbers(seg_num) - 1;

            if (    (seg_index >= 0)
                 && (fs_cached_pixel_counts[ seg_index ] < fs_seg_min_segment_size)
               )
            {
                segmentation_ptr->seg_map[ i ][ j ] = 0;
                (fs_cached_pixel_counts[ seg_index ])--;
            }
        }
    }

    post_merge_expand_edges(ip, segmentation_ptr->seg_map, count);
#endif
    }

    erode_segments(ip, segmentation_ptr->seg_map);
    fill_holes(ip, segmentation_ptr->seg_map);

    find_boundary_pixels(ip, segmentation_ptr->seg_map,
                         &fs_num_cached_pixels, fs_cached_pixels);

    /*
    // Home stretch. We are not longer modifying things, just getting the
    // segmentation information into the appropriate form.
    */
    ERE(get_segment_pixels(ip, segmentation_ptr, fs_cached_segment_count,
                           fs_cached_pixel_counts, fs_cached_R_means,
                           fs_cached_G_means, fs_cached_B_means,
                           fs_cached_r_chrom_means, fs_cached_g_chrom_means,
                           fs_cached_sum_RGB_means));

    ERE(get_boundaries_of_segments(segmentation_ptr, fs_num_cached_pixels,
                                   fs_cached_pixels));

    update_segment_map(segmentation_ptr);

#ifdef DO_NEGATIVE_SEGMENTS
    ERE(do_negative_segments(ip, segmentation_ptr));
#endif

    /*
    // At this point, the correct segment number is in the segment map. We no
    // longer use get_good_segment_number().
    */
    ERE(find_interior_points(segmentation_ptr));
    ERE(find_outside_boundary(segmentation_ptr));
    ERE(find_neighbours(segmentation_ptr));

#ifdef COMPUTE_EDGE_STRENGTHS
    ERE(get_edge_strengths(ip, segmentation_ptr));
#endif

#ifdef TEST
    if (fs_seg_verify_segmentation)
    {
        ERE(verify_segmentation(segmentation_ptr));
    }
#endif

    verbose_pso(3, "Complete segmentation has %d segments and took %ldms.\n",
                segmentation_ptr->num_segments,
                get_cpu_time() - initial_cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DO_NEGATIVE_SEGMENTS
static int do_negative_segments
(
    const KJB_image* ip,
    Segmentation_t3*    segmentation_ptr
)
{
    int result = NO_ERROR;
    int num_rows = segmentation_ptr->num_rows;
    int num_cols = segmentation_ptr->num_cols;
    int i, j;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            if (segmentation_ptr->seg_map[ i ][ j ] < 0)
            {
                segmentation_ptr->seg_map[ i ][ j ] = 0;
            }
            else
            {
                segmentation_ptr->seg_map[ i ][ j ] *= -1;
            }
        }
    }

    get_negative_segmentation(ip, segmentation_ptr->seg_map);

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            if (segmentation_ptr->seg_map[ i ][ j ] < 0)
            {
                segmentation_ptr->seg_map[ i ][ j ] *= -1;
            }
            else if (segmentation_ptr->seg_map[ i ][ j ] > 0)
            {
                segmentation_ptr->seg_map[ i ][ j ] -= 1;
                segmentation_ptr->seg_map[ i ][ j ] *= -1;
            }
        }
    }

    return result;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef DO_NEGATIVE_SEGMENTS
static void get_negative_segmentation(const KJB_image* ip, int** seg_map)
{
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int i, j;
    long initial_cpu_time = get_cpu_time();


    UNTESTED_CODE();

    zero_seg_map(ip, seg_map);

    fs_cached_seg_map = seg_map;
    fs_cached_num_rows = num_rows;
    fs_cached_num_cols = num_cols;

    fs_cached_segment_count = 0;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            if (seg_map[ i ][ j ] == 0)
            {
                fs_cached_segment_count++;
                grow_unconstrained_segment(i, j);

                if (fs_num_cached_pixels < fs_seg_min_initial_segment_size)
                {
                    /*
                    // Segment_t3 is not big enough. Back off this segment,
                    // and set the map locations to the special value of 1.
                    */
#ifdef COLLECT_PIXELS_DURING_INITIAL_SEG
                    UNTESTED_CODE();

                    {
                        int ii;

                        for (ii = 0; ii < fs_num_cached_pixels; ii++)
                        {
                            int iii = fs_cached_pixels[ ii ].i;
                            int jjj = fs_cached_pixels[ ii ].j;

                            seg_map[ iii ][ jjj ] = 1;
                        }
                    }
#else
#ifdef TEST
                    fs_count_remark = 0;
#endif
                    remark_segment(i, j, 1);
#ifdef TEST
                    ASSERT(fs_count_remark == fs_num_cached_pixels);
#endif
#endif
                    fs_cached_segment_count--;
                }
                else
                {
                    int  num_pixels = fs_num_cached_pixels;
                    int  s = fs_cached_segment_count - 1;
                    double ave_sum_RGB;

                    fs_cached_pixel_counts[ s ] = num_pixels;

                    fs_cached_R_means[ s ] = fs_cached_R_sum / num_pixels;
                    fs_cached_G_means[ s ] = fs_cached_G_sum / num_pixels;
                    fs_cached_B_means[ s ] = fs_cached_B_sum / num_pixels;

                    fs_cached_r_chrom_means[ s ] = fs_cached_r_chrom_sum / num_pixels;
                    fs_cached_g_chrom_means[ s ] = fs_cached_g_chrom_sum / num_pixels;

                    ave_sum_RGB = fs_cached_R_sum + fs_cached_G_sum + fs_cached_B_sum;

                    fs_cached_sum_RGB_means[ s ] = ave_sum_RGB / num_pixels;

#ifdef MAINTAIN_SS
                    fs_cached_r_chrom_SS[ s ] = fs_cached_r_chrom_sqr_sum;
                    fs_cached_g_chrom_SS[ s ] = fs_cached_g_chrom_sqr_sum;
                    fs_cached_sum_RGB_SS[ s ] = fs_cached_sum_RGB_sqr_sum;
#endif
                }
            }
        }
    }

#ifdef TEST
    {
        int num_pixels_in_segments = 0;
        int num_unsegmented_pixels = 0 ;

        for (i=0; i < fs_cached_segment_count; i++)
        {
            num_pixels_in_segments += fs_cached_pixel_counts[ i ];
        }

        verbose_pso(10, "Image has %d pixels in negative segments.\n",
                    num_pixels_in_segments);

        for (i=0; i<num_rows; i++)
        {
            for (j=0; j<num_cols; j++)
            {
                /* ASSERT(fs_cached_seg_map[ i ][ j ] != 0); */

                if (fs_cached_seg_map[ i ][ j ] <= 0)
                {
                    num_unsegmented_pixels++;
                }
            }
        }

        verbose_pso(10, "Image has %d unsegmented or postive pixels.\n",
                    num_unsegmented_pixels);

        verbose_pso(10, "Image size is %d x %d = %d.\n",
                    num_rows, num_cols, num_rows * num_cols);

        ASSERT(num_pixels_in_segments + num_unsegmented_pixels == num_rows * num_cols);
    }
#endif

    verbose_pso(3, "Negative segmentation has %d segments and took %ldms.\n",
                fs_cached_segment_count, get_cpu_time() - initial_cpu_time);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_unconstrained_segment(int i, int j)
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
    Pixel cur_image_pixel;


    UNTESTED_CODE();

    cur_image_pixel = fs_cached_ip->pixels[ i ][ j ];

    ASSERT(! (fs_cached_seg_map[ i ][ j ]));
    ASSERT(cur_image_pixel.extra.invalid.pixel == VALID_PIXEL);

    fs_cached_pixels[ fs_num_cached_pixels ].i = i;
    fs_cached_pixels[ fs_num_cached_pixels ].j = j;

    cur_R = cur_image_pixel.r;
    cur_G = cur_image_pixel.g;
    cur_B = cur_image_pixel.b;

    cur_sum_RGB = cur_R + cur_G + cur_B + RGB_SUM_EPSILON;
    cur_r_chrom = cur_R / cur_sum_RGB;
    cur_g_chrom = cur_G / cur_sum_RGB;

    fs_cached_r_chrom_sum += cur_r_chrom;
    fs_cached_g_chrom_sum += cur_g_chrom;
    fs_cached_R_sum += cur_R;
    fs_cached_G_sum += cur_G;
    fs_cached_B_sum += cur_B;
#ifdef MAINTAIN_SS
    fs_cached_r_chrom_sqr_sum += (cur_r_chrom * cur_r_chrom);
    fs_cached_g_chrom_sqr_sum += (cur_g_chrom * cur_g_chrom);
    fs_cached_sum_RGB_sqr_sum += (cur_sum_RGB * cur_sum_RGB);
#endif

    fs_num_cached_pixels++;

    ASSERT(fs_num_cached_pixels <= fs_cached_num_rows * fs_cached_num_cols);

    /*
    // We number the segments starting at 2 because segment 1 is used to denote
    // bits of segments which have been explored already but were too small to
    // call a segment. In the case of positive segmentation, we use negatives
    // for this, but now the negatives are being used to store the negatives of
    // positives.
    */
    fs_cached_seg_map[ i ][ j ] = fs_cached_segment_count + 1;

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
                     && (i_offset < fs_cached_num_rows)
                     && (j_offset < fs_cached_num_cols)
#ifdef CONNECT_CORNER_OPTION
                     && (fs_seg_connect_corners || ((di == 0) || (dj == 0)))
#else
#ifndef DEFAULT_IS_TO_CONNECT_CORNERS
                     && ((di == 0) || (dj == 0))
#endif
#endif
                     && ( ! fs_cached_seg_map[ i_offset ][ j_offset ])
                     && (fs_cached_ip->pixels[ i_offset ][ j_offset ].extra.invalid.pixel == VALID_PIXEL)
                   )
                {
                    grow_unconstrained_segment(i_offset, j_offset);
                }
            }
        }
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_initial_segmentation(const KJB_image* ip, int** seg_map)
{
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int i, j;
    long initial_cpu_time = get_cpu_time();
    int  seed_size;
    int  smooth_size;
    int  smooth_count     = NOT_SET;


    zero_seg_map(ip, seg_map);

    /* Normally bad practice, but we want to improve performace! */
    fs_cached_seg_map = seg_map;
    fs_cached_num_rows = num_rows;
    fs_cached_num_cols = num_cols;

    fs_cached_segment_count = 0;

    for (smooth_size = 0; smooth_size <= fs_seg_max_smooth_size; smooth_size++)
    {
        if (kjb_copy_image(&fs_cached_ip, ip) == ERROR)
        {
            add_error("Aborting additional smoothing for initial segmentation.");
            return ERROR;
        }

        if (smooth_size == 0)
        {
            smooth_count = 0;
        }
        else if (smooth_size == 1)
        {
            smooth_count = 2;
        }
        else
        {
            ASSERT(KJB_IS_SET(smooth_count));
            smooth_count = 2 * smooth_count;
        }

        if (smooth_count > 0)
        {
            for (i=smooth_count; i<num_rows - smooth_count; i++)
            {
                for (j=smooth_count; j<num_cols - smooth_count; j++)
                {
                    if (    (seg_map[ i ][ j ] <= 0)
                         && (fs_cached_ip->pixels[ i ][ j ].extra.invalid.pixel == VALID_PIXEL)
                       )
                    {
                        float weight_sum = FLT_ZERO;
#ifdef WEIGHT_SMOOTHING
                        float weight;
#endif
                        float r_sum    = FLT_ZERO;
                        float g_sum    = FLT_ZERO;
                        float b_sum    = FLT_ZERO;
                        int ii, jj, i_offset, j_offset;

                        for (ii = -smooth_count; ii < smooth_count; ii++)
                        {
                            for (jj = -smooth_count; jj < smooth_count; jj++)
                            {
                                i_offset = i + ii;
                                j_offset = j + jj;

                                if (    (seg_map[ i_offset ][ j_offset ] <= 0)
                                     && (fs_cached_ip->pixels[ i_offset ][ j_offset ].extra.invalid.pixel == VALID_PIXEL)
                                   )
                                {
/* #define WEIGHT_SMOOTHING */

#ifdef WEIGHT_SMOOTHING
                                    weight = FLT_ONE / (FLT_ONE + ABS_OF((float)ii) + ABS_OF((float)jj));
                                    r_sum += ip->pixels[ i_offset ][ j_offset ].r * weight;
                                    g_sum += ip->pixels[ i_offset ][ j_offset ].g * weight;
                                    b_sum += ip->pixels[ i_offset ][ j_offset ].b * weight;
                                    weight_sum += weight;
#else
                                    r_sum += ip->pixels[ i_offset ][ j_offset ].r;
                                    g_sum += ip->pixels[ i_offset ][ j_offset ].g;
                                    b_sum += ip->pixels[ i_offset ][ j_offset ].b;
                                    weight_sum += FLT_ONE;
#endif
                                }

                            }
                        }

                        fs_cached_ip->pixels[ i ][ j ].r = r_sum / weight_sum;
                        fs_cached_ip->pixels[ i ][ j ].g = g_sum / weight_sum;
                        fs_cached_ip->pixels[ i ][ j ].b = b_sum / weight_sum;

                        seg_map[ i ][ j ] = 0;
                    }
                }
            }

#ifdef REALLY_TEST
            if (smooth_size == 1)
            {
                EPETE(kjb_write_image(fs_cached_ip, "smooth-1.kiff"));
            }
            else if (smooth_size == 2)
            {
                EPETE(kjb_write_image(fs_cached_ip, "smooth-2.kiff"));
            }
            else if (smooth_size == 3)
            {
                EPETE(kjb_write_image(fs_cached_ip, "smooth-3.kiff"));
            }
            else if (smooth_size == 4)
            {
                EPETE(kjb_write_image(fs_cached_ip, "smooth-4.kiff"));
            }
            else if (smooth_size == 5)
            {
                EPETE(kjb_write_image(fs_cached_ip, "smooth-5.kiff"));
            }
#endif
        }

        seed_size = fs_seg_max_seed_size;

        while (seed_size >= fs_seg_min_seed_size)
        {
#ifdef USE_RANDOM_SEEDING
            int k;

            for (k = 0; k < num_rows*num_cols / 5; k++)
            {
                i = (int)(5.0 + ((double)num_rows - 10.0) * kjb_rand());
                j = (int)(5.0 + ((double)num_cols - 10.0) * kjb_rand());
#else
            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
#endif
                {
                    if (    (seg_map[ i ][ j ] == 0)
                         && (fs_cached_ip->pixels[ i ][ j ].extra.invalid.pixel == VALID_PIXEL)
                         && (neighbourhood_is_smooth(fs_cached_ip, i, j, seed_size))
                       )

                    {
                        fs_cached_segment_count++;
                        get_initial_segmentation_helper(i, j);

                        if (    (fs_num_cached_pixels < fs_seg_min_initial_segment_size)
                                /*
                             || (segment_is_sliver(seg_map,
                                                   fs_cached_segment_count,
                                                   fs_num_cached_pixels,
                                                   fs_cached_pixels))
                                                   */
                           )

                        {
                            /*
                            // Segment_t3 is not big enough. Back off this segment,
                            // and set the map locations to the negative of the
                            // number of pixels.
                            */
#ifdef COLLECT_PIXELS_DURING_INITIAL_SEG
                            {
                                int ii;

                                for (ii = 0; ii < fs_num_cached_pixels; ii++)
                                {
                                    int iii = fs_cached_pixels[ ii ].i;
                                    int jjj = fs_cached_pixels[ ii ].j;

                                    seg_map[ iii ][ jjj ] = -fs_num_cached_pixels;
                                }
                            }
#else
#ifdef TEST
                            fs_count_remark = 0;
#endif
#ifdef REMARK_SMALL_REGIONS_AS_ZERO
                            remark_segment(i, j, 0);
#else
                            remark_segment(i, j, -fs_num_cached_pixels);
#endif
#ifdef TEST
                            ASSERT(fs_count_remark == fs_num_cached_pixels);
#endif
#endif
                            fs_cached_segment_count--;
                        }
                        else
                        {
                            int  num_pixels = fs_num_cached_pixels;
                            int  s = fs_cached_segment_count - 1;
                            double ave_sum_RGB;

                            fs_cached_pixel_counts[ s ] = num_pixels;

                            fs_cached_R_means[ s ] = fs_cached_R_sum / num_pixels;
                            fs_cached_G_means[ s ] = fs_cached_G_sum / num_pixels;
                            fs_cached_B_means[ s ] = fs_cached_B_sum / num_pixels;

                            fs_cached_r_chrom_means[ s ] = fs_cached_r_chrom_sum / num_pixels;
                            fs_cached_g_chrom_means[ s ] = fs_cached_g_chrom_sum / num_pixels;

                            ave_sum_RGB = fs_cached_R_sum + fs_cached_G_sum + fs_cached_B_sum;

                            fs_cached_sum_RGB_means[ s ] = ave_sum_RGB / num_pixels;
#ifdef MAINTAIN_SS
                            fs_cached_r_chrom_SS[ s ] = fs_cached_r_chrom_sqr_sum;
                            fs_cached_g_chrom_SS[ s ] = fs_cached_g_chrom_sqr_sum;
                            fs_cached_sum_RGB_SS[ s ] = fs_cached_sum_RGB_sqr_sum;
#endif

                            /*
                            {
                                double stdev_r;
                                double var_r = fs_cached_r_chrom_sqr_sum;

                                var_r -= fs_cached_r_chrom_sum * fs_cached_r_chrom_sum / num_pixels;
                                var_r /= (num_pixels - 1);

                                stdev_r = sqrt(var_r);

                                dbf(fs_cached_r_chrom_means[ s ]);
                                dbf(stdev_r);
                            }
                            */
                        }
                    }
                }
            /* Duplicate the parenthesis to help count them.  */
#ifdef USE_RANDOM_SEEDING
            }
#else
            }
#endif
            seed_size--;
        }
    }

#ifdef TEST
    {
        int num_pixels_in_segments = 0;
        int num_unsegmented_pixels = 0;

        for (i=0; i < fs_cached_segment_count; i++)
        {
            num_pixels_in_segments += fs_cached_pixel_counts[ i ];
        }

        verbose_pso(10, "Image has %d pixels in initial segments.\n",
                    num_pixels_in_segments);

        for (i=0; i<num_rows; i++)
        {
            for (j=0; j<num_cols; j++)
            {
                /* ASSERT(fs_cached_seg_map[ i ][ j ] != 0); */

                if (fs_cached_seg_map[ i ][ j ] <= 0)
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
                fs_cached_segment_count, get_cpu_time() - initial_cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int neighbourhood_is_smooth
(
    const KJB_image* ip,
    int              i,
    int              j,
    int              size
)
{
    float R, G, B;
    int   ii, jj;

    if (size == 0) return TRUE;

    R = ip->pixels[ i ][ j ].r;
    G = ip->pixels[ i ][ j ].g;
    B = ip->pixels[ i ][ j ].b;

    verbose_pso(500, "Seeing if (%d, %d) is a smooth neigbourood .... ", i, j);

    for (ii = i - size; ii < i + size; ii++)
    {
        for (jj = j - size; jj < j + size; jj++)
        {
            if (    (ii > 0) && (ii < ip->num_rows)
                 && (jj > 0) && (jj < ip->num_cols)
               )
            {
                float r = ip->pixels[ ii ][ jj ].r;
                float g = ip->pixels[ ii ][ jj ].g;
                float b = ip->pixels[ ii ][ jj ].b;

                if (    (ABS_OF((R - r) / (R + r + 1.0)) > 0.15)
                     || (ABS_OF((G - g) / (G + g + 1.0)) > 0.15)
                     || (ABS_OF((B - b) / (B + b + 1.0)) > 0.15)
                   )
                {
                    verbose_pso(500, "  nope.\n");
                    return FALSE;
                }
            }
        }
    }

    verbose_pso(500, "  yup.\n");

    return TRUE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef NOT_USED_YET
/*ARGSUSED*/
static int segment_is_sliver
(
    int**         seg_map,
    int           seg,
    int           num_pixels,
    Cached_pixel* pixels
)
{

    return FALSE;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void erode_segments(const KJB_image* ip, int** seg_map)
{
    int i, j;
    long initial_cpu_time = get_cpu_time();
    int  seg_num;
    int  iteration;
    int  num_rows  = ip->num_rows;
    int  num_cols  = ip->num_cols;


#define ERODE_TINY_HAIRS

#ifdef ERODE_TINY_HAIRS
    for (iteration = 0; iteration < fs_seg_num_segment_erosions; iteration++)
    {
        int erosion_done = FALSE;

        for (i=1; i<num_rows - 1; i++)
        {
            for (j=1; j<num_cols - 1; j++)
            {
                seg_num = seg_map[ i ][ j ];

                if (seg_num > 0)
                {
                    int connect_count = 0;
                    int s;
                    int ii, jj, i_offset, j_offset;


                    for (ii = -1; ii<=1; ii ++)
                    {
                        for (jj = -1; jj<=1; jj++)
                        {
                            i_offset = i + ii;
                            j_offset = j + ii;

                            s = seg_map[ i_offset ][ j_offset ];

                            if (s == seg_num)
                            {
                                connect_count++;
                            }
                        }
                    }

                    /*
                    // We got one for free, since we tested against oursevles.
                    // If we only have one more connection, then we erode.
                    */
                    if (connect_count <= 2)
                    {
                        erosion_done = TRUE;
                        seg_map[ i][j ] = 0;
                        fs_cached_pixel_counts[ seg_num - 1]--;
                    }
                }
            }
        }

        /*
        // If we did not get one on this iteration, we are not going to get one
        // on the next!
        */
        if ( ! erosion_done )
        {
            iteration++;
            break;
        }
    }
#else
    for (iteration = 0; iteration < fs_seg_num_segment_erosions; iteration++)
    {
        int erosion_done = FALSE;

        for (i=1; i<num_rows - 1; i++)
        {
            for (j=1; j<num_cols - 1; j++)
            {
                seg_num = seg_map[ i ][ j ];
                seg_num = chase_initial_segment_numbers(seg_num);

                if (seg_num > 0)
                {
                    int s;
                    int ii, jj, i_offset, j_offset;
                    float R = ip->pixels[ i ][ j ].r;
                    float G = ip->pixels[ i ][ j ].g;
                    float B = ip->pixels[ i ][ j ].b;
                    float sum = R + G + B + RGB_SUM_EPSILON;
                    float r = R / sum;
                    float g = G / sum;
                    float r_diff = ABS_OF(fs_cached_r_chrom_means[ seg_num - 1 ] - r);
                    float g_diff = ABS_OF(fs_cached_g_chrom_means[ seg_num - 1 ] - g);
                    float sum_diff = ABS_OF(fs_cached_sum_RGB_means[ seg_num - 1 ] - sum);
                    float diff_prod = r_diff * g_diff * sum_diff;
                    int   found_a_better_home = FALSE;


                    for (ii = -1; ii<=1; ii ++)
                    {
                        if (found_a_better_home) break;

                        for (jj = -1; jj<=1; jj++)
                        {
                            if (found_a_better_home) break;

                            i_offset = i + ii;
                            j_offset = j + ii;

                            s = seg_map[ i_offset ][ j_offset ];
                            s= chase_initial_segment_numbers(s);

                            if ((s > 0) && (s != seg_num))
                            {
                                float R2 = ip->pixels[ i_offset ][ j_offset ].r;
                                float G2 = ip->pixels[ i_offset ][ j_offset ].g;
                                float B2 = ip->pixels[ i_offset ][ j_offset ].b;
                                float sum2 = R2 + G2 + B2 + RGB_SUM_EPSILON;
                                float r2 = R2 / sum2;
                                float g2 = G2 / sum2;
                                float r_diff_2 = ABS_OF(fs_cached_r_chrom_means[ s - 1 ] - r2);
                                float g_diff_2 = ABS_OF(fs_cached_g_chrom_means[ s - 1 ] - g2);
                                float sum_diff_2 = ABS_OF(fs_cached_sum_RGB_means[ s - 1 ] - sum2);
                                float diff_prod_2 = r_diff_2 * g_diff_2 * sum_diff_2;

                                if (diff_prod_2 < diff_prod)
                                {
                                    erosion_done = TRUE;
                                    seg_map[ i][j ] = s;
                                    fs_cached_pixel_counts[ seg_num - 1]--;
                                    fs_cached_pixel_counts[ s - 1]++;

                                    found_a_better_home = TRUE;
                                }
                            }
                        }
                    }
                }
            }
        }

        /*
        // If we did not get one on this iteration, we are not going to get one
        // on the next!
        */
        if ( ! erosion_done )
        {
            iteration++;
            break;
        }
    }
#endif

    verbose_pso(5, "Eroding segments (%d expansions out of %d used) took %ldms.\n",
                iteration, fs_seg_num_segment_erosions,
                get_cpu_time() - initial_cpu_time);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void initial_expand_edges(const KJB_image* ip, int** seg_map)
{
    expand_edges(ip, seg_map, fs_seg_initial_expand_edge_level,
                 fs_seg_initial_num_edge_expansions);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef EXPAND_AFTER_MERGING
static void post_merge_expand_edges
(
    const KJB_image* ip,
    int**            seg_map,
    int              count
)
{
    expand_edges(ip, seg_map, MIN_OF(count + 1, fs_seg_expand_edge_level), fs_seg_num_edge_expansions);
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void expand_edges
(
    const KJB_image* ip,
    int**            seg_map,
    int              expand_edge_level,
    int              num_edge_expansions
)
{
    int i, j;
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    long initial_cpu_time = get_cpu_time();
    int         seg_num;
    int         iteration;
    int k;
    float chrom_var_thresh = fs_max_abs_chrom_var;
    float sum_var_thresh   = fs_max_rel_sum_RGB_var;
    int   level;

    for (level = 0; level < expand_edge_level; level++)
    {

#define MAX_NUM_BAD_PIXELS 100000
        for (iteration = 0; iteration < num_edge_expansions; iteration++)
        {
            int expansion_succeeded = FALSE;
            int num_bad_pixels = 0;
            int bad_pixels_i[ MAX_NUM_BAD_PIXELS ];
            int bad_pixels_j[ MAX_NUM_BAD_PIXELS ];

                for (j=1; j<num_cols - 1; j++)
                {
                if (num_bad_pixels >= MAX_NUM_BAD_PIXELS - 1) break;

            for (i=1; i<num_rows - 1; i++)
            {
                    seg_num = seg_map[ i ][ j ];

#ifdef EXPAND_AFTER_MERGING
                    seg_num = chase_initial_segment_numbers(seg_num);
#endif
                    if (    (seg_num <= 0)
#ifdef EXPAND_AFTER_MERGING
                         && (    (seg_map[ i - 1][ j ] > 0)
                              || (seg_map[ i + 1][ j ] > 0)
                              || (seg_map[ i ][ j - 1] > 0)
                              || (seg_map[ i ][ j + 1] > 0)
                            )
#else
                         && (    (chase_initial_segment_numbers(seg_map[ i - 1][ j ]) > 0)
                              || (chase_initial_segment_numbers(seg_map[ i + 1][ j ]) > 0)
                              || (chase_initial_segment_numbers(seg_map[ i ][ j - 1]) > 0)
                              || (chase_initial_segment_numbers(seg_map[ i ][ j + 1]) > 0)
                            )
#endif
                        )
                    {
                        if (num_bad_pixels >= MAX_NUM_BAD_PIXELS - 1) break;

                        bad_pixels_i[ num_bad_pixels ] = i;
                        bad_pixels_j[ num_bad_pixels ] = j;
                        num_bad_pixels++;
                    }
                }
            }

#ifdef EXPAND_RANDOMLY
            for (k = 0; k < 30; k++)
            {
                int step = 5 + 10 * kjb_rand();
                int offset = kjb_rand() * step;
                int kk;

                for (kk = offset; kk < num_bad_pixels; kk += step)
                {
                    i = bad_pixels_i[ kk ];
                    j = bad_pixels_j[ kk ];

#else
                for (k = 0; k < num_bad_pixels; k ++)
                {
                    i = bad_pixels_i[ k ];
                    j = bad_pixels_j[ k ];

#ifdef DEF_OUT
            for (i=1; i<num_rows - 1; i++)
            {
                for (j=1; j<num_cols - 1; j++)
                {
#endif
#endif
                    seg_num = seg_map[ i ][ j ];
#ifdef EXPAND_AFTER_MERGING
                    seg_num = chase_initial_segment_numbers(seg_num);
#endif

                    if (seg_num <= 0)
                    {
                        int expand_seg = NOT_SET;
#ifdef FIND_BEST_EXPANSION_HOME
                        float min_diff_prod = FLT_NOT_SET;
#endif
                        float R = ip->pixels[ i ][ j ].r;
                        float G = ip->pixels[ i ][ j ].g;
                        float B = ip->pixels[ i ][ j ].b;
                        float sum = R + G + B + RGB_SUM_EPSILON;
                        float r = R / sum;
                        float g = G / sum;
                        int s;
                        int ii, jj, i_offset, j_offset;

                        for (ii = -1; ii<=1; ii += 2)
                        {
#ifndef FIND_BEST_EXPANSION_HOME
                            if (KJB_IS_SET(expand_seg)) break;
#endif

                            for (jj = 0; jj<=1; jj ++)
                            {
                                if (j == 0)
                                {
                                    i_offset = i;
                                    j_offset = j + ii;
                                }
                                else
                                {
                                    i_offset = i + ii;
                                    j_offset = j;
                                }

                                s = seg_map[ i_offset ][ j_offset ];
#ifdef EXPAND_AFTER_MERGING
                                s = chase_initial_segment_numbers(s);
#endif

                                if (s > 0)
                                {
                                    float r_diff = ABS_OF(fs_cached_r_chrom_means[ s - 1 ] - r);
                                    float g_diff = ABS_OF(fs_cached_g_chrom_means[ s - 1 ] - g);
                                    float sum_diff = 2.0 * ABS_OF(fs_cached_sum_RGB_means[ s - 1 ] - sum);
                                    float sum_test = sum_diff / ABS_OF(fs_cached_sum_RGB_means[ s - 1 ] + sum);
#ifdef FIND_BEST_EXPANSION_HOME
                                    float diff_prod = r_diff * g_diff * sum_test;
#endif

                                    if (    (     (expand_seg == NOT_SET)
                                               && (r_diff*r_diff + g_diff*g_diff < chrom_var_thresh*chrom_var_thresh)
                                               && (sum_test < sum_var_thresh)
                                            )
#ifdef FIND_BEST_EXPANSION_HOME
                                        || (diff_prod < min_diff_prod)
#endif
                                       )
                                    {
                                        expand_seg = s;
#ifdef FIND_BEST_EXPANSION_HOME
                                        min_diff_prod = diff_prod;
#else
                                        break;
#endif
                                    }
                                }
                            }
                        }

                        if (KJB_IS_SET(expand_seg))
                        {
                            seg_map[ i][j ] = expand_seg;
                            fs_cached_pixel_counts[ expand_seg - 1]++;
                            expansion_succeeded++;
                        }
#ifdef EXPAND_RANDOMLY
                    }
                }
#else
                }
#endif
            }

            if ( !expansion_succeeded )
            {
                verbose_pso(10,
                            "Stopping expansion level %d at iteration %d.\n",
                            level + 1, iteration + 1);
                break;
            }
        }

        sum_var_thresh *= 1.5;
        chrom_var_thresh *= 1.5;
    }

    verbose_pso(5, "Expanding edges (%d levels, %d expansions) took %ldms.\n",
                expand_edge_level, num_edge_expansions,
                get_cpu_time() - initial_cpu_time);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void fill_holes(const KJB_image* ip, int** seg_map)
{
    int i, j;
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    long initial_cpu_time = get_cpu_time();
    int         seg_num;
#ifdef CHANGE_IMAGE_DUE_TO_HOLES
    Pixel pixel;
#endif
    int         iteration;
    int num_holes_filled = 0;


#ifdef CHANGE_IMAGE_DUE_TO_HOLES
    pixel.extra.invalid.r = VALID_PIXEL;
    pixel.extra.invalid.g = VALID_PIXEL;
    pixel.extra.invalid.b = VALID_PIXEL;
    pixel.extra.invalid.pixel = VALID_PIXEL;
#endif

    for (iteration = 0; iteration < fs_seg_fill_hole_level; iteration++)
    {
        int holes_filled = FALSE;

        for (i=1; i<num_rows - 1; i++)
        {
            for (j=1; j<num_cols - 1; j++)
            {
                seg_num = seg_map[ i ][ j ];

                /* ASSERT(seg_num != 0); */

                if (seg_num <= 0)
                {
                    /*
                    // Identifies the 4 orientations of
                    //     XX
                    //     0X
                    //     XX
                    */
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
#ifdef CHANGE_IMAGE_DUE_TO_HOLES
                        float R = FLT_ZERO;
                        float G = FLT_ZERO;
                        float B = FLT_ZERO;
#endif

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
#ifdef CHANGE_IMAGE_DUE_TO_HOLES
                                        R += ip->pixels[ m ][ n ].r;
                                        G += ip->pixels[ m ][ n ].g;
                                        B += ip->pixels[ m ][ n ].b;
#endif
                                        surround_count++;
                                    }
                                }
                            }
                        }

                        ASSERT(surround_count >= 5);

                        seg_map[ i][j ] = surround_seg_num;

#ifdef CHANGE_IMAGE_DUE_TO_HOLES
                        R /= surround_count;
                        pixel.r = R;

                        G /= surround_count;
                        pixel.g = G;

                        B /= surround_count;
                        pixel.b = B;

                        ip->pixels[ i ][ j ] = pixel;
#endif

                        fs_cached_pixel_counts[ surround_seg_num - 1]++;

                        holes_filled = TRUE;
                        num_holes_filled++;
                    }
                }
            }
        }

        if (! holes_filled)
        {
            iteration++;
            break;
        }
    }

    verbose_pso(5, "Filling %d holes (%d iterations used out of %d) took %ldms.\n",
                num_holes_filled,
                iteration, fs_seg_fill_hole_level,
                get_cpu_time() - initial_cpu_time);
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


    *num_pixels_ptr = 0;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            int seg_num;

            seg_num = chase_initial_segment_numbers(seg_map[ i ][ j ]);

            if (seg_num > 0)
            {
                if (    (i == 0) || (j == 0)
                     || (i == num_rows - 1) || (j == num_cols - 1)
                     || (chase_initial_segment_numbers(seg_map[ i + 1 ][ j ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i - 1 ][ j ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i ][ j + 1 ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i ][ j - 1 ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i + 1 ][ j + 1 ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i - 1 ][ j + 1 ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i + 1 ][ j - 1 ]) != seg_num)
                     || (chase_initial_segment_numbers(seg_map[ i - 1 ][ j - 1 ]) != seg_num)
                   )
                {
                    pixel_buff->pixel = ip->pixels[ i ][ j ];
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

static int merge_small_segment_pairs
(
    const KJB_image* ip,
    int**            seg_map,
    int              num_boundary_pixels,
    Cached_pixel*    boundary_pixels,
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
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int i, j;
    long initial_cpu_time = get_cpu_time();
    long cpu_time_1;
    long cpu_time_2;
    long cpu_time_3;
    long cpu_time_4;
    long cpu_time_5;
    long cpu_time_6;
    int  result           = 0;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    ERE(update_segment_pair_storage(num_segments, "merging"));
    ERE(update_segment_merge_storage(num_segments, "merging"));

    cpu_time_1 = get_cpu_time();
    verbose_pso(10, "Merge part one took %ldms.\n",
                cpu_time_1 - initial_cpu_time);

    /* Initilize merge chart to zeros. We only use one of the upper right
    // triangle of the chart, but it is easiest to initialize the whole thin. */

    for (i=0; i<num_segments; i++)
    {
        for (j=0; j<num_segments; j++)
        {
            fs_boundary_pair_counts[ i ][ j ] = 0;
            fs_properly_connected_pairs[ i ][ j ] = FALSE;
            fs_data_pair_counts[ i ][ j ] = 0;
            fs_merge_candidates[ i ][ j ] = 0;
        }
    }

    cpu_time_2 = get_cpu_time();
    verbose_pso(10, "Merge part two took %ldms.\n",
                cpu_time_2 - cpu_time_1);


    /*
    // Update merge info based on boundary pairs. Note that things can be
    // overcounted many times.
    */
    for (i=0; i<num_boundary_pixels; i++)
    {
        int seg_num = boundary_pixels[ i ].seg_num;
        int b_i = boundary_pixels[ i ].i;
        int b_j = boundary_pixels[ i ].j;
        int b_seg_num;
        int seg_index = seg_num - 1;
        int b_i_m = b_i - 1;
        int b_j_m = b_j - 1;
        int b_i_p = b_i + 1;
        int b_j_p = b_j + 1;

        if (b_j_m >= 0)
        {
            b_seg_num = seg_map[ b_i ][ b_j_m ];

            if (b_seg_num > 0)
            {
                fs_properly_connected_pairs[ seg_index ][ b_seg_num - 1 ] = TRUE;
                fs_properly_connected_pairs[ b_seg_num - 1 ][ seg_index ] = TRUE;
            }
        }
        if (b_j_p < num_cols)
        {
            b_seg_num = seg_map[ b_i ][ b_j_p ];

            if (b_seg_num > 0)
            {
                fs_properly_connected_pairs[ seg_index ][ b_seg_num - 1 ] = TRUE;
                fs_properly_connected_pairs[ b_seg_num - 1 ][ seg_index ] = TRUE;
            }
        }
        if (b_i_m >= 0 )
        {
            b_seg_num = seg_map[ b_i_m ][ b_j ];

            if (b_seg_num > 0)
            {
                fs_properly_connected_pairs[ seg_index ][ b_seg_num - 1 ] = TRUE;
                fs_properly_connected_pairs[ b_seg_num - 1 ][ seg_index ] = TRUE;
            }
        }
        if (b_i_p < num_rows)
        {
            b_seg_num = seg_map[ b_i_p ][ b_j ];

            if (b_seg_num > 0)
            {
                fs_properly_connected_pairs[ seg_index ][ b_seg_num - 1 ] = TRUE;
                fs_properly_connected_pairs[ b_seg_num - 1 ][ seg_index ] = TRUE;
            }
        }
    }

    cpu_time_3 = get_cpu_time();
    verbose_pso(10, "Merge part three took %ldms.\n",
                cpu_time_3 - cpu_time_2);

    /* Look at merge charts for regions that should be merged. */

    for (i=0; i<num_segments; i++)
    {
        for (j = i + 1; j<num_segments; j++)
        {
            if ( ! fs_properly_connected_pairs[ i ][ j ]) continue;

            if (fs_merge_candidates[ i ][ j ]) continue;

            if (    (pixel_counts[ i ] < fs_merge_small_region_size)
                 && (pixel_counts[ j ] < fs_merge_small_region_size)
               )
            {
                verbose_pso(10, "Merging regions %d and %d since they are both small.\n",
                            i+1, j+1);
                fs_merge_candidates[ i ][ j ] = 1;
                result++;
            }
#ifdef MERGE_SMALL_SURROUNDED_REGIONS
            /*
            // I don't think that this is working properly, and I am not sure
            // if we want to do it anyway.
            */
            else if (pixel_counts[ i ] < fs_merge_small_region_size)
            {
                int jj;
                int completely_inside = TRUE;

                for (jj = 0; jj < num_segments; jj++)
                {
                    if (    (fs_properly_connected_pairs[ i ][ jj ])
                         && (jj != j)
                         && (jj != i)
                       )
                    {
                        completely_inside = FALSE;
                        break;
                    }
                }

                /*
                // FIX
                //
                // May want some additional tests to verify that too much of
                // the boundary is not actually unset regions instead of the
                // proposed surrounding regions.
                */
                if (completely_inside)
                {
                    verbose_pso(10, "Merging regions %d and %d becuase %d is inside %d.\n",
                                i+1, j+1, i+1, j+1);
                    fs_merge_candidates[ i ][ j ] = 1;
                    result++;
                }
            }
            /* Must try it the other way, as "surround" is not symmetric. */
            else if (pixel_counts[ j ] < fs_merge_small_region_size)
            {
                int ii;
                int completely_inside = TRUE;

                for (ii = 0; ii < num_segments; ii++)
                {
                    if (    (fs_properly_connected_pairs[ ii ][ j ])
                         && (ii != j)
                         && (ii != i)
                       )
                    {
                        completely_inside = FALSE;
                        break;
                    }
                }

                /*
                // FIX
                //
                // May want some additional tests to verify that too much of
                // the boundary is not actually unset regions instead of the
                // proposed surrounding regions.
                */
                if (completely_inside)
                {
                    verbose_pso(10, "Merging regions %d and %d becuase %d is inside %d.\n",
                                j+1, i+1, j+1, i+1);
                    fs_merge_candidates[ i ][ j ] = 1;
                    result++;
                }
            }
#endif
        }
    }

    cpu_time_4 = get_cpu_time();
    verbose_pso(10, "Merge part four took %ldms.\n",
                cpu_time_4 - cpu_time_3);

    cpu_time_5 = get_cpu_time();
    verbose_pso(10, "Merge part five took %ldms.\n",
                cpu_time_5 - cpu_time_4);

    /* Look at merge charts for regions that should be merged. */

    for (i=0; i<num_segments; i++)
    {
        for (j = i+1; j<num_segments; j++)
        {
            if (fs_merge_candidates[ i ][ j ])
            {
                ERE(merge_segment_pair(i, j, pixel_counts,
                                       R_means, G_means, B_means,
                                       r_chrom_means, g_chrom_means,
                                       sum_RGB_means));
            }
        }
    }

    cpu_time_6 = get_cpu_time();
    verbose_pso(10, "Merge part six took %ldms.\n",
                cpu_time_6 - cpu_time_5);

    verbose_pso(5, "Merge took %ldms.\n", get_cpu_time() - initial_cpu_time);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int merge_segments_1
(
    const KJB_image* ip,
    int**            seg_map,
    int              num_boundary_pixels,
    Cached_pixel*    boundary_pixels,
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
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int i, j;
    long   initial_cpu_time = get_cpu_time();
    long   cpu_time_1;
    long   cpu_time_2;
    long   cpu_time_3;
    long   cpu_time_4;
    long   cpu_time_5;
    int    result           = 0;
    double score;
    double min_score        = DBL_NOT_SET;
    int    min_score_i      = NOT_SET;
    int    min_score_j      = NOT_SET;
    int    min_score_count  = NOT_SET;
    double merge_rg_rms_threshold_2;
    double merge_sum_RGB_rel_threshold_2;
    double boundary_r_mean_diff_2;
    double boundary_g_mean_diff_2;


    if (    (fs_merge_sum_RGB_rel_threshold <= 0.0)
         || (fs_merge_rg_rms_threshold <= 0.0)
       )
    {
        return 0;
    }

    merge_rg_rms_threshold_2 = fs_merge_rg_rms_threshold * fs_merge_rg_rms_threshold;
    merge_sum_RGB_rel_threshold_2 = fs_merge_sum_RGB_rel_threshold * fs_merge_sum_RGB_rel_threshold;

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    ERE(update_segment_pair_storage(num_segments, "merging"));
    ERE(update_segment_merge_storage(num_segments, "merging"));

    cpu_time_1 = get_cpu_time();
    verbose_pso(10, "Merge part one took %ldms.\n",
                cpu_time_1 - initial_cpu_time);

    /* Initilize merge chart to zeros. We only use one of the upper right
    // triangle of the chart, but it is easiest to initialize the whole thin. */

    for (i=0; i<num_segments; i++)
    {
        for (j=0; j<num_segments; j++)
        {
            fs_boundary_pair_counts[ i ][ j ] = 0;
            fs_properly_connected_pairs[ i ][ j ] = FALSE;
            fs_data_pair_counts[ i ][ j ] = 0;
            fs_merge_candidates[ i ][ j ] = 0;
            fs_merge_r_sum_mp->elements[ i ][ j ] = 0.0;
            fs_merge_g_sum_mp->elements[ i ][ j ] = 0.0;
            fs_merge_sum_RGM_sum_mp->elements[ i ][ j ] = 0.0;
        }
    }

    cpu_time_2 = get_cpu_time();
    verbose_pso(10, "Merge part two took %ldms.\n",
                cpu_time_2 - cpu_time_1);


    /*
    // Update merge info based on boundary pairs. Note that things can be
    // overcounted many times.
    */
    for (i=0; i<num_boundary_pixels; i++)
    {
        int seg_num = boundary_pixels[ i ].seg_num;
        int b_i = boundary_pixels[ i ].i;
        int b_j = boundary_pixels[ i ].j;

        if (pixel_counts[ seg_num - 1 ] > fs_merge_min_num_pixels)
        {

            if (    (b_i >= fs_merge_max_step)
                 && (b_j >= fs_merge_max_step)
                 && (b_i < num_rows - fs_merge_max_step)
                 && (b_j < num_cols - fs_merge_max_step)
               )
            {

                int ii, jj;
                int ii_min = b_i - fs_merge_max_step;
                int ii_max = b_i + fs_merge_max_step;
                int jj_min = b_j - fs_merge_max_step;
                int jj_max = b_j + fs_merge_max_step;
                float R1 = ip->pixels[ b_i ][ b_j ].r;
                float G1 = ip->pixels[ b_i ][ b_j ].g;
                float B1 = ip->pixels[ b_i ][ b_j ].b;
                float sum_1 = R1 + G1 + B1 + RGB_SUM_EPSILON;
                float r_1 = R1 / sum_1;
                float g_1 = G1 / sum_1;

                for (ii = ii_min; ii <= ii_max; ii++)
                {
                    for (jj = jj_min; jj <= jj_max; jj++)
                    {
                        int nb_seg_num = chase_initial_segment_numbers(seg_map[ ii ][ jj ]);

                        if (    (nb_seg_num > 0)
                             && (nb_seg_num != seg_num)
                             && (pixel_counts[ nb_seg_num - 1 ] > fs_merge_min_num_pixels)
                           )
                        {
                            int s1 = seg_num - 1;
                            int s2 = nb_seg_num - 1;
                            float R2    = ip->pixels[ ii ][ jj ].r;
                            float G2    = ip->pixels[ ii ][ jj ].g;
                            float B2    = ip->pixels[ ii ][ jj ].b;
                            float sum_2 = R2 + G2 + B2 + RGB_SUM_EPSILON;
                            float r_2   = R2 / sum_2;
                            float g_2   = G2 / sum_2;

                            (fs_boundary_pair_counts[ s1 ][ s2 ])++;
                            (fs_boundary_pair_counts[ s2 ][ s1 ])++;

                            fs_merge_r_sum_mp->elements[ s1 ][ s2 ] += r_1;
                            fs_merge_g_sum_mp->elements[ s1 ][ s2 ] += g_1;
                            fs_merge_sum_RGM_sum_mp->elements[ s1 ][ s2 ] += sum_1;
                            fs_merge_r_sum_mp->elements[ s2 ][ s1 ] += r_2;
                            fs_merge_g_sum_mp->elements[ s2 ][ s1 ] += g_2;
                            fs_merge_sum_RGM_sum_mp->elements[ s2 ][ s1 ] += sum_2;

                            (fs_data_pair_counts[ s1 ][ s2 ])++;
                            (fs_data_pair_counts[ s2 ][ s1 ])++;

                            if ( ! fs_properly_connected_pairs[ s1 ][ s2 ])
                            {
                                if (    ((ii == b_i) && (ABS_OF(jj - b_j) == 1))
                                     || ((jj == b_j) && (ABS_OF(ii - b_i) == 1))
                                   )
                                {
                                    fs_properly_connected_pairs[ s1 ][ s2 ] = TRUE;
                                    fs_properly_connected_pairs[ s2 ][ s1 ] = TRUE;
                                }
                            }
                        }
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
        for (j = i + 1; j<num_segments; j++)
        {
            int count = (fs_boundary_pair_counts[i][j] + fs_boundary_pair_counts[j][i])/2;

            if ( ! fs_properly_connected_pairs[ i ][ j ]) continue;

            if (count > fs_merge_min_num_connections)
            {
                double boundary_r_mean_diff;
                double boundary_g_mean_diff;
                double boundary_sum_RGB_mean_rel_diff;
                double min_sum_diff;


                boundary_r_mean_diff = fs_merge_r_sum_mp->elements[ i ][ j ] - fs_merge_r_sum_mp->elements[ j ][ i ];
                boundary_r_mean_diff /= fs_data_pair_counts[ i ][ j ];

                boundary_g_mean_diff = fs_merge_g_sum_mp->elements[ i ][ j ] - fs_merge_g_sum_mp->elements[ j ][ i ];
                boundary_g_mean_diff /= fs_data_pair_counts[ i ][ j ];

                boundary_sum_RGB_mean_rel_diff = ABS_OF(fs_merge_sum_RGM_sum_mp->elements[ i ][ j ] - fs_merge_sum_RGM_sum_mp->elements[ j ][ i ]);
                boundary_sum_RGB_mean_rel_diff /= (fs_merge_sum_RGM_sum_mp->elements[ i ][ j ] + fs_merge_sum_RGM_sum_mp->elements[ j ][ i ]);
                boundary_sum_RGB_mean_rel_diff *= 2.0;

                min_sum_diff = MIN_OF(fs_merge_sum_RGM_sum_mp->elements[ i ][ j ],
                                      fs_merge_sum_RGM_sum_mp->elements[ j ][ i ]);

                if (min_sum_diff < fs_min_sum_RGB_for_chrom_test / 4.0)
                {
                    boundary_r_mean_diff = 0.0;
                    boundary_g_mean_diff = 0.0;
                }
                else if (min_sum_diff < fs_min_sum_RGB_for_chrom_test / 2.0)
                {
                    boundary_r_mean_diff /= 2.0;
                    boundary_g_mean_diff /= 2.0;
                }
                else if (min_sum_diff < fs_min_sum_RGB_for_chrom_test)
                {
                    boundary_r_mean_diff /= 1.5;
                    boundary_g_mean_diff /= 1.5;
                }

                /* Merge based on proposed shading or noise boundary */

                /* We don't check that the fs_merge_rg_rms_threshold and
                // fs_merge_sum_RGB_rel_threshold are set becasue if they are not,
                // the tests will fail, which is what we want.
                */
                boundary_r_mean_diff_2 = boundary_r_mean_diff * boundary_r_mean_diff;
                boundary_g_mean_diff_2 = boundary_g_mean_diff * boundary_g_mean_diff;

                if (    (    (boundary_sum_RGB_mean_rel_diff < fs_merge_sum_RGB_rel_threshold)
                          && (boundary_r_mean_diff_2 + boundary_g_mean_diff_2 < merge_rg_rms_threshold_2)
                        )
                   )
                {
                    score = boundary_sum_RGB_mean_rel_diff * boundary_sum_RGB_mean_rel_diff / merge_sum_RGB_rel_threshold_2;
                    score += boundary_r_mean_diff_2 / merge_rg_rms_threshold_2;
                    score += boundary_g_mean_diff_2 / merge_rg_rms_threshold_2;

                    if ((min_score < 0.0) || (score < min_score))
                    {
                        min_score = score;
                        min_score_i = i;
                        min_score_j = j;
                        min_score_count = count;
                    }
                }
            }

        }
    }

    cpu_time_4 = get_cpu_time();
    verbose_pso(10, "Merge part four took %ldms.\n",
                cpu_time_4 - cpu_time_3);

    if (min_score >= 0.0)
    {
        verbose_pso(10, "Merging regions %d and %d due to ",
                    min_score_i + 1, min_score_j + 1);
        verbose_pso(10, "small border differences\n");
        verbose_pso(10, "    (connectivity count is %d).\n",
                    min_score_count);

        fs_merge_candidates[ min_score_i ][ min_score_j ] = 1;

        ERE(merge_segment_pair(min_score_i, min_score_j, pixel_counts,
                               R_means, G_means, B_means,
                               r_chrom_means, g_chrom_means,
                               sum_RGB_means));

        result++;
    }

    cpu_time_5 = get_cpu_time();
    verbose_pso(10, "Merge part five took %ldms.\n",
                cpu_time_5 - cpu_time_4);

    verbose_pso(5, "Merge took %ldms.\n", get_cpu_time() - initial_cpu_time);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int merge_segments_2
(
    const KJB_image* ip,
    int**            seg_map,
    int              num_boundary_pixels,
    Cached_pixel*    boundary_pixels,
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
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int i, j;
    long   initial_cpu_time = get_cpu_time();
    long   cpu_time_1;
    long   cpu_time_2;
    long   cpu_time_3;
    long   cpu_time_4;
    long   cpu_time_5;
    int    result           = 0;
    double score;
    double min_score        = DBL_NOT_SET;
    int    min_score_i      = NOT_SET;
    int    min_score_j      = NOT_SET;
    int    min_score_count  = NOT_SET;
    double segment_r_mean_diff_2;
    double segment_g_mean_diff_2;
    double merge_rg_drift_2;
    double merge_sum_RGB_drift_2;


    if (    (fs_merge_rg_drift <= 0.0)
         || (fs_merge_sum_RGB_drift <= 0.0)
       )
    {
        return 0;
    }

    merge_rg_drift_2 = fs_merge_rg_drift * fs_merge_rg_drift;
    merge_sum_RGB_drift_2 = fs_merge_sum_RGB_drift * fs_merge_sum_RGB_drift;

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    ERE(update_segment_pair_storage(num_segments, "merging"));
    ERE(update_segment_merge_storage(num_segments, "merging"));

    cpu_time_1 = get_cpu_time();
    verbose_pso(10, "Merge part one took %ldms.\n",
                cpu_time_1 - initial_cpu_time);

    /* Initilize merge chart to zeros. We only use one of the upper right
    // triangle of the chart, but it is easiest to initialize the whole thin. */

    for (i=0; i<num_segments; i++)
    {
        for (j=0; j<num_segments; j++)
        {
            fs_boundary_pair_counts[ i ][ j ] = 0;
            fs_properly_connected_pairs[ i ][ j ] = FALSE;
            fs_data_pair_counts[ i ][ j ] = 0;
            fs_merge_candidates[ i ][ j ] = 0;
            fs_merge_r_sum_mp->elements[ i ][ j ] = 0.0;
            fs_merge_g_sum_mp->elements[ i ][ j ] = 0.0;
            fs_merge_sum_RGM_sum_mp->elements[ i ][ j ] = 0.0;
        }
    }

    cpu_time_2 = get_cpu_time();
    verbose_pso(10, "Merge part two took %ldms.\n",
                cpu_time_2 - cpu_time_1);


    /*
    // Update merge info based on boundary pairs. Note that things can be
    // overcounted many times.
    */
    for (i=0; i<num_boundary_pixels; i++)
    {
        int seg_num = boundary_pixels[ i ].seg_num;
        int b_i = boundary_pixels[ i ].i;
        int b_j = boundary_pixels[ i ].j;

        if (pixel_counts[ seg_num - 1 ] > fs_merge_min_num_pixels)
        {
            if (    (b_i >= fs_merge_max_step)
                 && (b_j >= fs_merge_max_step)
                 && (b_i < num_rows - fs_merge_max_step)
                 && (b_j < num_cols - fs_merge_max_step)
               )
            {
                int ii, jj;
                int ii_min = b_i - fs_merge_max_step;
                int ii_max = b_i + fs_merge_max_step;
                int jj_min = b_j - fs_merge_max_step;
                int jj_max = b_j + fs_merge_max_step;

                for (ii = ii_min; ii <= ii_max; ii++)
                {
                    for (jj = jj_min; jj <= jj_max; jj++)
                    {
                        int nb_seg_num = chase_initial_segment_numbers(seg_map[ ii ][ jj ]);

                        if (    (nb_seg_num > 0)
                             && (nb_seg_num != seg_num)
                             && (pixel_counts[ nb_seg_num - 1 ] > fs_merge_min_num_pixels)
                           )
                        {
                            int s1 = seg_num - 1;
                            int s2 = nb_seg_num - 1;

                            (fs_boundary_pair_counts[ s1 ][ s2 ])++;
                            (fs_boundary_pair_counts[ s2 ][ s1 ])++;

                            if ( ! fs_properly_connected_pairs[ s1 ][ s2 ])
                            {
                                if (    ((ii == b_i) && (ABS_OF(jj - b_j) == 1))
                                     || ((jj == b_j) && (ABS_OF(ii - b_i) == 1))
                                   )
                                {
                                    fs_properly_connected_pairs[ s1 ][ s2 ] = TRUE;
                                    fs_properly_connected_pairs[ s2 ][ s1 ] = TRUE;
                                }
                            }
                        }
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
        for (j = i + 1; j<num_segments; j++)
        {
            int count = (fs_boundary_pair_counts[i][j] + fs_boundary_pair_counts[j][i]) / 2;

            if ( ! fs_properly_connected_pairs[ i ][ j ]) continue;

            if (count >= fs_merge_min_num_connections)
            {
                double segment_r_mean_diff;
                double segment_g_mean_diff;
                double segment_sum_mean_diff;
                double min_sum_diff;

                segment_r_mean_diff = ABS_OF(r_chrom_means[i] - r_chrom_means[j]);
                segment_g_mean_diff = ABS_OF(g_chrom_means[i] - g_chrom_means[j]);
                segment_sum_mean_diff = ABS_OF(sum_RGB_means[i] - sum_RGB_means[j]);

#ifdef LOOKS_BUGGY_TO_ME
                segment_sum_mean_diff /= ABS_OF(sum_RGB_means[i] + sum_RGB_means[j]);
                segment_sum_mean_diff *= 2.0;
#endif

                min_sum_diff = MIN_OF(sum_RGB_means[ i ], sum_RGB_means[ j ]);

                if (min_sum_diff < fs_min_sum_RGB_for_chrom_test / 4.0)
                {
                    segment_r_mean_diff = 0.0;
                    segment_g_mean_diff = 0.0;
                }
                else if (min_sum_diff < fs_min_sum_RGB_for_chrom_test / 2.0)
                {
                    segment_r_mean_diff /= 2.0;
                    segment_g_mean_diff /= 2.0;
                }
                else if (min_sum_diff < fs_min_sum_RGB_for_chrom_test)
                {
                    segment_r_mean_diff /= 1.5;
                    segment_g_mean_diff /= 1.5;
                }

                segment_r_mean_diff_2 = segment_r_mean_diff*segment_r_mean_diff;
                segment_g_mean_diff_2 = segment_g_mean_diff*segment_g_mean_diff;

                if (    (segment_r_mean_diff_2 + segment_g_mean_diff_2 < merge_rg_drift_2)
                     && (segment_sum_mean_diff < fs_merge_sum_RGB_drift)
                   )
                {
                    score = segment_r_mean_diff_2 / merge_rg_drift_2;
                    score += segment_g_mean_diff_2 / merge_rg_drift_2;
                    score += segment_sum_mean_diff * segment_sum_mean_diff / merge_sum_RGB_drift_2;

                    ASSERT(score >= 0.0);

                    if ((min_score < 0.0) || (score < min_score))
                    {
                        min_score = score;
                        min_score_i = i;
                        min_score_j = j;
                        min_score_count = count;
                    }
                }
            }
        }
    }

    cpu_time_4 = get_cpu_time();
    verbose_pso(10, "Merge part four took %ldms.\n",
                cpu_time_4 - cpu_time_3);

    if (min_score >= 0.0)
    {
        verbose_pso(10, "Merging regions %d and %d due to ",
                    min_score_i + 1, min_score_j + 1);
        verbose_pso(10, "small region differences\n");
        verbose_pso(10, "    (connectivity count is %d).\n",
                    min_score_count);

        fs_merge_candidates[ min_score_i ][ min_score_j ] = 1;

        ERE(merge_segment_pair(min_score_i, min_score_j, pixel_counts,
                               R_means, G_means, B_means,
                               r_chrom_means, g_chrom_means,
                               sum_RGB_means));

        result++;
    }

    cpu_time_5 = get_cpu_time();
    verbose_pso(10, "Merge part five took %ldms.\n",
                cpu_time_5 - cpu_time_4);

    verbose_pso(5, "Merge took %ldms.\n", get_cpu_time() - initial_cpu_time);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef HOW_IT_WAS_FOR_A_LONG_TIME

/*
// This should be used in conjunction with the option of
// merge_multiple_regions_simultaneously.
//
// Previous doing all sub-threshold merges was done simlutaneoulsy, which is
// fast, but dangerous.
//
// If this is resurected:
//      max_sum_diff (and the corresponding MAX_OF) should become MIN
//
//      update_segment_pair_storage should be used.
*/

static int merge_segments
(
    const KJB_image* ip,
    int**            seg_map,
    int              num_boundary_pixels,
    Cached_pixel*    boundary_pixels,
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
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int i, j;
    int count;
    long initial_cpu_time = get_cpu_time();
    long cpu_time_1;
    long cpu_time_2;
    long cpu_time_3;
    long cpu_time_4;
    long cpu_time_5;
    long cpu_time_6;
    int  result           = 0;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (num_segments > fs_max_num_segments)
    {
        verbose_pso(10, "Bumping up storage for merging.\n");

        free_2D_int_array(fs_properly_connected_pairs);
        free_2D_int_array(fs_boundary_pair_counts);
        free_2D_int_array(fs_data_pair_counts);
        free_2D_int_array(fs_merge_candidates);

        NRE(fs_data_pair_counts = allocate_2D_int_array(num_segments, num_segments));
        NRE(fs_properly_connected_pairs = allocate_2D_int_array(num_segments,
                                                         num_segments));
        NRE(fs_boundary_pair_counts = allocate_2D_int_array(num_segments,
                                                         num_segments));
        NRE(fs_merge_candidates = allocate_2D_int_array(num_segments,
                                                     num_segments));

        ERE(get_target_matrix(&fs_merge_r_sum_mp, num_segments,
                              num_segments));

        ERE(get_target_matrix(&fs_merge_g_sum_mp, num_segments,
                              num_segments));

        ERE(get_target_matrix(&fs_merge_sum_RGM_sum_mp, num_segments,
                              num_segments));

        fs_max_num_segments = num_segments;
    }

    cpu_time_1 = get_cpu_time();
    verbose_pso(10, "Merge part one took %ldms.\n",
                cpu_time_1 - initial_cpu_time);

    /* Initilize merge chart to zeros. We only use one of the upper right
    // triangle of the chart, but it is easiest to initialize the whole thin. */

    for (i=0; i<num_segments; i++)
    {
        for (j=0; j<num_segments; j++)
        {
            fs_boundary_pair_counts[ i ][ j ] = 0;
            fs_properly_connected_pairs[ i ][ j ] = FALSE;
            fs_data_pair_counts[ i ][ j ] = 0;
            fs_merge_candidates[ i ][ j ] = 0;
            fs_merge_r_sum_mp->elements[ i ][ j ] = 0.0;
            fs_merge_g_sum_mp->elements[ i ][ j ] = 0.0;
            fs_merge_sum_RGM_sum_mp->elements[ i ][ j ] = 0.0;
        }
    }

    cpu_time_2 = get_cpu_time();
    verbose_pso(10, "Merge part two took %ldms.\n",
                cpu_time_2 - cpu_time_1);


    /*
    // Update merge info based on boundary pairs. Note that things can be
    // overcounted many times.
    */
    for (i=0; i<num_boundary_pixels; i++)
    {
        int seg_num = boundary_pixels[ i ].seg_num;
        int b_i = boundary_pixels[ i ].i;
        int b_j = boundary_pixels[ i ].j;

#ifdef MERGE_SMALL_PAIRS
        int b_seg_num;
        int seg_index = seg_num - 1;
        int b_i_m = b_i - 1;
        int b_j_m = b_j - 1;
        int b_i_p = b_i + 1;
        int b_j_p = b_j + 1;

        if (b_j_m >= 0)
        {
            b_seg_num = seg_map[ b_i ][ b_j_m ];

            if (b_seg_num > 0)
            {
                fs_properly_connected_pairs[ seg_index ][ b_seg_num - 1 ] = TRUE;
                fs_properly_connected_pairs[ b_seg_num - 1 ][ seg_index ] = TRUE;
            }
        }
        if (b_j_p < num_cols)
        {
            b_seg_num = seg_map[ b_i ][ b_j_p ];

            if (b_seg_num > 0)
            {
                fs_properly_connected_pairs[ seg_index ][ b_seg_num - 1 ] = TRUE;
                fs_properly_connected_pairs[ b_seg_num - 1 ][ seg_index ] = TRUE;
            }
        }
        if (b_i_m >= 0 )
        {
            b_seg_num = seg_map[ b_i_m ][ b_j ];

            if (b_seg_num > 0)
            {
                fs_properly_connected_pairs[ seg_index ][ b_seg_num - 1 ] = TRUE;
                fs_properly_connected_pairs[ b_seg_num - 1 ][ seg_index ] = TRUE;
            }
        }
        if (b_i_p < num_rows)
        {
            b_seg_num = seg_map[ b_i_p ][ b_j ];

            if (b_seg_num > 0)
            {
                fs_properly_connected_pairs[ seg_index ][ b_seg_num - 1 ] = TRUE;
                fs_properly_connected_pairs[ b_seg_num - 1 ][ seg_index ] = TRUE;
            }
        }
#endif

        if (pixel_counts[ seg_num - 1 ] > fs_merge_min_num_pixels)
        {

            if (    (b_i >= fs_merge_max_step)
                 && (b_j >= fs_merge_max_step)
                 && (b_i < num_rows - fs_merge_max_step)
                 && (b_j < num_cols - fs_merge_max_step)
               )
            {
                int d1, d2, horizontal, ii, jj, mm, nn, seg_num_1;
                int nb_seg_num = NOT_SET;

                for (d1 = 0; d1 <= 2 * fs_merge_max_step; d1++)
                {
                    for (horizontal = 0; horizontal <= 1; horizontal++)
                    {
                        if (horizontal)
                        {
                            jj = b_j;

                            if (IS_EVEN(d1))
                            {
                                ii = b_i + d1 / 2;
                            }
                            else
                            {
                                ii = b_i - (d1  + 1)/ 2;
                            }
                        }
                        else
                        {
                            ii = b_i;

                            if (IS_EVEN(d1))
                            {
                                jj = b_j + d1 / 2;
                            }
                            else
                            {
                                jj = b_j - (d1  + 1)/ 2;
                            }
                        }

                        seg_num_1 = seg_map[ ii ][ jj ];

                        if (seg_num_1 == seg_num)
                        {
                            float R1 = ip->pixels[ ii ][ jj ].r;
                            float G1 = ip->pixels[ ii ][ jj ].g;
                            float B1 = ip->pixels[ ii ][ jj ].b;
                            float sum_1 = R1 + G1 + B1 + RGB_SUM_EPSILON;
                            float r_1 = R1 / sum_1;
                            float g_1 = G1 / sum_1;
                            int   seg_num_2;

                            for (d2 = 1; d2 <= 2 * fs_merge_max_step; d2++)
                            {
                                if (horizontal)
                                {
                                    nn = b_j;

                                    if (IS_EVEN(d2))
                                    {
                                        mm = b_i + d2 / 2;
                                    }
                                    else
                                    {
                                        mm = b_i - (d2  + 1)/ 2;
                                    }
                                }
                                else
                                {
                                    mm = b_i;

                                    if (IS_EVEN(d1))
                                    {
                                        nn = b_j + d2 / 2;
                                    }
                                    else
                                    {
                                        nn = b_j - (d2  + 1)/ 2;
                                    }
                                }

                                seg_num_2 = seg_map[ mm ][ nn ];

                                if (    (seg_num_2 > 0)
                                     && (seg_num_2 != seg_num_1)
                                     && (pixel_counts[ seg_num_2 - 1 ] > fs_merge_min_num_pixels)
                                     && (    (nb_seg_num == NOT_SET)
                                          || (nb_seg_num == seg_num_2)
                                        )
                                   )
                                {
                                    int s1 = seg_num_1 - 1;
                                    int s2 = seg_num_2 - 1;
                                    float R2 = ip->pixels[ mm ][ nn ].r;
                                    float G2 = ip->pixels[ mm ][ nn ].g;
                                    float B2 = ip->pixels[ mm ][ nn ].b;
                                    float sum_2 = R2 + G2 + B2 + RGB_SUM_EPSILON;
                                    float r_2 = R2 / sum_2;
                                    float g_2 = G2 / sum_2;

                                    if (nb_seg_num == NOT_SET)
                                    {
                                        nb_seg_num = seg_num_2;
                                        (fs_boundary_pair_counts)[s1][s2]++;
                                    }

#ifndef MERGE_SMALL_PAIRS
                                    if (    (d1 == 0) && (d2 <= 3))
                                    {
                                        fs_properly_connected_pairs[s1][s2]=TRUE;
                                    }
#endif

                                    fs_merge_r_sum_mp->elements[ s1 ][ s2 ] += r_1;
                                    fs_merge_g_sum_mp->elements[ s1 ][ s2 ] += g_1;
                                    fs_merge_sum_RGM_sum_mp->elements[ s1 ][ s2 ] += sum_1;
                                    fs_merge_r_sum_mp->elements[ s2 ][ s1 ] += r_2;
                                    fs_merge_g_sum_mp->elements[ s2 ][ s1 ] += g_2;
                                    fs_merge_sum_RGM_sum_mp->elements[ s2 ][ s1 ] += sum_2;

                                    (fs_data_pair_counts[ s1 ][ s2 ])++;
                                    (fs_data_pair_counts[ s2 ][ s1 ])++;
                                }
                            }
                        }
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
        for (j = i + 1; j<num_segments; j++)
        {
            count = fs_boundary_pair_counts[i][j] + fs_boundary_pair_counts[j][i];
            count /= 2;

            if ( ! fs_properly_connected_pairs[ i ][ j ]) continue;

            if (count > fs_merge_min_num_connections)
            {
                double boundary_r_mean_diff;
                double boundary_g_mean_diff;
                double boundary_sum_mean_diff;
                double segment_r_mean_diff;
                double segment_g_mean_diff;
                double segment_sum_mean_diff;
                double max_sum_diff;

                segment_r_mean_diff = ABS_OF(r_chrom_means[i] - r_chrom_means[j]);
                segment_g_mean_diff = ABS_OF(g_chrom_means[i] - g_chrom_means[j]);
                segment_sum_mean_diff = ABS_OF(sum_RGB_means[i] - sum_RGB_means[j]);
                segment_sum_mean_diff /= ABS_OF(sum_RGB_means[i] + sum_RGB_means[j]);
                segment_sum_mean_diff *= 2.0;

                max_sum_diff = MAX_OF(sum_RGB_means[ i ], sum_RGB_means[ j ]);

                if (max_sum_diff < fs_min_sum_RGB_for_chrom_test / 4.0)
                {
                    segment_r_mean_diff = 0.0;
                    segment_g_mean_diff = 0.0;
                }
                else if (max_sum_diff < fs_min_sum_RGB_for_chrom_test / 2.0)
                {
                    segment_r_mean_diff /= 2.0;
                    segment_g_mean_diff /= 2.0;
                }
                else if (max_sum_diff < fs_min_sum_RGB_for_chrom_test)
                {
                    segment_r_mean_diff /= 1.5;
                    segment_g_mean_diff /= 1.5;
                }

                boundary_r_mean_diff = fs_merge_r_sum_mp->elements[ i ][ j ] - fs_merge_r_sum_mp->elements[ j ][ i ];
                boundary_r_mean_diff /= fs_data_pair_counts[ i ][ j ];

                boundary_g_mean_diff = fs_merge_g_sum_mp->elements[ i ][ j ] - fs_merge_g_sum_mp->elements[ j ][ i ];
                boundary_g_mean_diff /= fs_data_pair_counts[ i ][ j ];

                boundary_sum_mean_diff = ABS_OF(fs_merge_sum_RGM_sum_mp->elements[ i ][ j ] - fs_merge_sum_RGM_sum_mp->elements[ j ][ i ]);
                boundary_sum_mean_diff /= (fs_merge_sum_RGM_sum_mp->elements[ i ][ j ] + fs_merge_sum_RGM_sum_mp->elements[ j ][ i ]);
                boundary_sum_mean_diff *= 2.0;

                max_sum_diff = MAX_OF(fs_merge_sum_RGM_sum_mp->elements[ i ][ j ],
                                      fs_merge_sum_RGM_sum_mp->elements[ j ][ i ]);

                if (max_sum_diff < fs_min_sum_RGB_for_chrom_test / 4.0)
                {
                    boundary_r_mean_diff = 0.0;
                    boundary_g_mean_diff = 0.0;
                }
                else if (max_sum_diff < fs_min_sum_RGB_for_chrom_test / 2.0)
                {
                    boundary_r_mean_diff /= 2.0;
                    boundary_g_mean_diff /= 2.0;
                }
                else if (max_sum_diff < fs_min_sum_RGB_for_chrom_test)
                {
                    boundary_r_mean_diff /= 1.5;
                    boundary_g_mean_diff /= 1.5;
                }

                /* Merge based on proposed shading boundary */

                /* We don't check that the fs_merge_rg_rms_threshold and
                // fs_merge_sum_RGB_rel_threshold are set becasue if they are not,
                // the tests will fail, which is what we want.
                */
                if (    (    (boundary_sum_mean_diff < fs_merge_sum_RGB_rel_threshold)
                          && (boundary_r_mean_diff*boundary_r_mean_diff + boundary_g_mean_diff*boundary_g_mean_diff < fs_merge_rg_rms_threshold*fs_merge_rg_rms_threshold)
                        )
                   )
                {
                    verbose_pso(10, "Merging regions %d and %d due to ",
                                i+1, j+1);
                    verbose_pso(10, "small border differences\n");
                    verbose_pso(10, "    (connectivity count is %d).\n", count);

                    fs_merge_candidates[ i ][ j ] = 1;

                    result++;
                }
                /* Merge based on differnce in regions */
                else if (    (segment_r_mean_diff*segment_r_mean_diff + segment_g_mean_diff*segment_g_mean_diff < fs_merge_rg_drift*fs_merge_rg_drift)
                          && (segment_sum_mean_diff < fs_merge_sum_RGB_drift)
                        )
                {
                    verbose_pso(10, "Merging regions %d and %d due to ",
                                i+1, j+1);
                    verbose_pso(10, "small region difference\n");
                    verbose_pso(10, "    (connectivity count is %d).\n", count);

                    fs_merge_candidates[ i ][ j ] = 1;

                    result++;
                }
                /* Merge based on differnce in regions */
                else if (    (ABS_OF(R_means[ i ] - R_means[ j ]) <= FLT_TWO)
                          && (ABS_OF(G_means[ i ] - G_means[ j ]) <= FLT_TWO)
                          && (ABS_OF(B_means[ i ] - B_means[ j ]) <= FLT_TWO)
                        )
                {
                    verbose_pso(10, "Merging regions %d and %d due to ",
                                i+1, j+1);
                    verbose_pso(10, "small region RGB difference\n");
                    verbose_pso(10, "    (connectivity count is %d).\n", count);

                    fs_merge_candidates[ i ][ j ] = 1;

                    result++;
                }
            }

#ifdef MERGE_SMALL_PAIRS
            if (fs_merge_candidates[ i ][ j ]) continue;

            if (    (pixel_counts[ i ] < fs_merge_min_num_pixels)
                 && (pixel_counts[ j ] < fs_merge_min_num_pixels)
               )
            {
                verbose_pso(10, "Merging regions %d and %d since they are both small.\n",
                            i+1, j+1);
                fs_merge_candidates[ i ][ j ] = 1;
                result++;
            }
#ifdef MERGE_SMALL_SURROUNDED_REGIONS
            /*
            // I don't think that this is working properly, and I am not sure
            // if we want to do it anyway.
            */
            else if (pixel_counts[ i ] < fs_merge_min_num_pixels)
            {
                int jj;
                int completely_inside = TRUE;

                for (jj = 0; jj < num_segments; jj++)
                {
                    if (    (fs_properly_connected_pairs[ i ][ jj ])
                         && (jj != j)
                         && (jj != i)
                       )
                    {
                        completely_inside = FALSE;
                        break;
                    }
                }

                /*
                // FIX
                //
                // May want some additional tests to verify that too much of
                // the boundary is not actually unset regions instead of the
                // proposed surrounding regions.
                */
                if (completely_inside)
                {
                    verbose_pso(10, "Merging regions %d and %d becuase %d is inside %d.\n",
                                i+1, j+1, i+1, j+1);
                    fs_merge_candidates[ i ][ j ] = 1;
                    result++;
                }
            }
            /* Must try it the other way, as "surround" is not symmetric. */
            else if (pixel_counts[ j ] < fs_merge_min_num_pixels)
            {
                int ii;
                int completely_inside = TRUE;

                for (ii = 0; ii < num_segments; ii++)
                {
                    if (    (fs_properly_connected_pairs[ ii ][ j ])
                         && (ii != j)
                         && (ii != i)
                       )
                    {
                        completely_inside = FALSE;
                        break;
                    }
                }

                /*
                // FIX
                //
                // May want some additional tests to verify that too much of
                // the boundary is not actually unset regions instead of the
                // proposed surrounding regions.
                */
                if (completely_inside)
                {
                    verbose_pso(10, "Merging regions %d and %d becuase %d is inside %d.\n",
                                j+1, i+1, j+1, i+1);
                    fs_merge_candidates[ i ][ j ] = 1;
                    result++;
                }
            }
#endif
#endif
        }
    }

#ifdef MERGE_PENUMBRA
    for (i=0; i<num_segments; i++)
    {
        for (j = 0; j<num_segments; j++)
        {
            if (fs_merge_candidates[ i ][ j ]) continue;

            count = fs_boundary_pair_counts[i][j] + fs_boundary_pair_counts[j][i];
            count /= 2;

            if ( ! fs_properly_connected_pairs[ i ][ j ]) continue;

            /* Now needed with the penumbra thing. */
            if (fs_merge_candidates[ i ][ j ]) continue;

            if (count > fs_merge_min_num_connections)
            {
                double boundary_r_mean_diff;
                double boundary_g_mean_diff;
                double boundary_sum_mean_diff;
                double segment_r_mean_diff;
                double segment_g_mean_diff;
                double segment_sum_mean_diff;
                double max_sum_diff;

                if (    (R_means[ i ] < R_means[ j ])
                     && (G_means[ i ] < G_means[ j ])
                     && (B_means[ i ] < B_means[ j ])
                     && (R_means[ i ] + G_means[ i ] + B_means[ i ] > FLT_TEN)
                     && (R_means[ j ] + G_means[ j ] + B_means[ j ] > FLT_TEN)
                     && (is_possible_illum_ratio(R_means[i]/R_means[j],
                                                 G_means[i]/G_means[j],
                                                 B_means[i]/B_means[j]))

                   )
                {
                    int k;

                    for (k = 0; k < num_segments; k++)
                    {
                        if (    (k != i)
                             && (k != j)
                             && ((fs_boundary_pair_counts[i][k] + fs_boundary_pair_counts[k][i]) / 2 > fs_merge_min_num_connections)
                             && (fs_properly_connected_pairs[ i ][ k ])
                             && (R_means[ k ] < R_means[ i ])
                             && (G_means[ k ] < G_means[ i ])
                             && (B_means[ k ] < B_means[ i ])
                             && (R_means[ k ] + G_means[ k ] + B_means[ k ] > FLT_TEN)
                             && (is_possible_illum_ratio(R_means[k]/R_means[i],
                                                         G_means[k]/G_means[i],
                                                         B_means[k]/B_means[i]))

                            )
                        {
                            double dr = R_means[ i ] - R_means[ k ];
                            double dg = G_means[ i ] - G_means[ k ];
                            double db = B_means[ i ] - B_means[ k ];
                            double tr = R_means[ j ] - R_means[ k ];
                            double tg = G_means[ j ] - G_means[ k ];
                            double tb = B_means[ j ] - B_means[ k ];
                            double fr = dr / tr;
                            double fg = dg / tg;
                            double fb = db / tb;
                            double m1 = MAX_OF(fr / fg, fg / fr) - 1.0;
                            double m2 = MAX_OF(fr / fb, fb / fr) - 1.0;
                            double m3 = MAX_OF(fg / fb, fb / fg) - 1.0;
                            double sum_m = m1 + m2 + m3;


                            if (sum_m < 0.30)
                            {
                                if (    (dr + dg + db < 0.5 * (tr + tg + tb))
                                     && (k > i)
                                     && ( ! fs_merge_candidates[ i ][ k ])
                                   )
                                {
                                    verbose_pso(9,
                                                "Merging regions %d and %d ",
                                                i+1, k+1);
                                    verbose_pso(9, "due to penumbra\n");
                                    verbose_pso(9, "    (connectivity count is %d).\n",
                                                count);

                                    fs_merge_candidates[ i ][ k ] = 1;

                                    result++;
                                }
                                else if (    (dr + dg + db < 0.5 * (tr + tg + tb))
                                          && (k < i)
                                          && ( ! fs_merge_candidates[ k ][ i ])
                                        )
                                {
                                    verbose_pso(9,
                                                "Merging regions %d and %d ",
                                                k+1, i+1);
                                    verbose_pso(9, "due to penumbra\n");
                                    verbose_pso(9, "    (connectivity count is %d).\n",
                                                count);

                                    fs_merge_candidates[ k ][ i ] = 1;

                                    result++;
                                }
                                else if (dr + dg + db >= 0.5 * (tr + tg + tb))
                                {
                                    verbose_pso(9,
                                                "Merging regions %d and %d ",
                                                i+1, j+1);
                                    verbose_pso(9, "due to penumbra\n");
                                    verbose_pso(9, "    (connectivity count is %d).\n",
                                                count);

                                    fs_merge_candidates[ i ][ j ] = 1;

                                    result++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
#endif

    cpu_time_4 = get_cpu_time();
    verbose_pso(10, "Merge part four took %ldms.\n",
                cpu_time_4 - cpu_time_3);

    cpu_time_5 = get_cpu_time();
    verbose_pso(10, "Merge part five took %ldms.\n",
                cpu_time_5 - cpu_time_4);

    /* Look at merge charts for regions that should be merged. */

    for (i=0; i<num_segments; i++)
    {
        for (j = i+1; j<num_segments; j++)
        {
            if (fs_merge_candidates[ i ][ j ])
            {
                ERE(merge_segment_pair(i, j, pixel_counts,
                                       R_means, G_means, B_means,
                                       r_chrom_means, g_chrom_means,
                                       sum_RGB_means));
            }
        }
    }

    cpu_time_6 = get_cpu_time();
    verbose_pso(10, "Merge part six took %ldms.\n",
                cpu_time_6 - cpu_time_5);

    verbose_pso(5, "Merge took %ldms.\n", get_cpu_time() - initial_cpu_time);

    return result;
}

#endif

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

    verbose_pso(15, "Merging relabeled regions %d and %d.\n", i+1, j+1);

    if (i == j)
    {
        verbose_pso(15, "Regions are already merge. Skipping.\n");
        return NO_ERROR;
    }

    if (i > j)
    {
        temp = i;
        i = j;
        j = temp;
    }

    verbose_pso(15, "Merging %d (%d pixels) into %d (%d pixels).\n", j+1,
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

    fs_cached_initial_segment_numbers[ j + 1 ] = i + 1;

    verbose_pso(15, "Region %d now has %d pixels and %d has %d pixels).\n",
                i+1, pixel_counts[ i ], j+1, pixel_counts[ j ]);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_segment_pixels
(
    const KJB_image* ip,
    Segmentation_t3*    segmentation_ptr,
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
    int** seg_map  = segmentation_ptr->seg_map;
    int   i, j;
    int   count;
    int   num_good_segments = 0;
    int   initial_seg_num;
    int   num_pixels;
    int   seg_num;
    Segment_t3* seg_ptr;
    int p;
    int* row_ptr;
    long initial_cpu_time = get_cpu_time();


    fs_cached_good_segment_numbers[ 0 ] = 0;

    for (i=0; i < num_segments; i++)
    {
        fs_cached_good_segment_numbers[ i + 1 ] = 0;

        num_pixels = pixel_counts[ i ];

        if (num_pixels >= fs_seg_min_segment_size)
        {
            num_good_segments++;
        }
    }

    verbose_pso(3, "Image has %d good segments.\n", num_good_segments);

    if (num_good_segments == 0) return NO_ERROR;

    NRE(segmentation_ptr->segments =
                            N_TYPE_MALLOC(Segment_t3*, num_good_segments));
    segmentation_ptr->num_segments = num_good_segments;

    /* We want a safe free for the caller if we return without finishing. */
    for (i=0; i<num_good_segments; i++)
    {
        segmentation_ptr->segments[ i ] = NULL;
    }

    count = 0;

    for (i=0; i<num_segments; i++)
    {
        ERE(initial_seg_num = chase_initial_segment_numbers(i + 1));

        if (initial_seg_num == i + 1)
        {
            num_pixels = pixel_counts[ initial_seg_num - 1 ];

            if (num_pixels >= fs_seg_min_segment_size)
            {
                seg_num = count + 1;

                NRE(segmentation_ptr->segments[ count ] =
                                  create_segment(seg_num, num_pixels));
                seg_ptr = segmentation_ptr->segments[ count ];

                seg_ptr->R_ave = R_means[ i ];
                seg_ptr->G_ave = G_means[ i ];
                seg_ptr->B_ave = B_means[ i ];
                seg_ptr->r_chrom_ave = r_chrom_means[ i ];
                seg_ptr->g_chrom_ave = g_chrom_means[ i ];
                seg_ptr->sum_RGB_ave = sum_RGB_means[ i ];

                fs_cached_good_segment_numbers[ initial_seg_num ] = seg_num;

                verbose_pso(20, "Initial segment %d has %d pixels. ",
                            initial_seg_num, num_pixels);
                verbose_pso(20, "Making it good segment %d.\n", seg_num);

                count++;
            }
        }
    }

    ASSERT(count == num_good_segments);

    for (i=0; i<num_rows; i++)
    {
        row_ptr = seg_map[ i ];

        for (j=0; j<num_cols; j++)
        {
            ERE(seg_num = get_good_segment_number(*row_ptr++));

            ASSERT(seg_num >= 0);

            if (seg_num > 0)
            {
                seg_ptr = segmentation_ptr->segments[ seg_num-1 ];

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
        seg_ptr = segmentation_ptr->segments[ i ];

        seg_ptr->i_CM /= (seg_ptr->num_pixels);
        seg_ptr->j_CM /= (seg_ptr->num_pixels);

        ASSERT(seg_ptr->pixel_count == seg_ptr->num_pixels);
    }

    verbose_pso(5, "Collecting segment pixels took %ldms.\n",
                get_cpu_time() - initial_cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void update_segment_map(Segmentation_t3* segmentation_ptr)
{
    int** seg_map  = segmentation_ptr->seg_map;
    int   num_rows = segmentation_ptr->num_rows;
    int   num_cols = segmentation_ptr->num_cols;
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

            /* ASSERT(seg_num != 0); */

            if (seg_num > 0)
            {
                *row_ptr = get_good_segment_number(seg_num);
            }
            row_ptr++;
        }
    }

    verbose_pso(5, "Updating the segment map took %ldms.\n",
                get_cpu_time() - initial_cpu_time);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_boundaries_of_segments
(
    Segmentation_t3* segmentation_ptr,
    int           num_boundary_pixels,
    Cached_pixel* pixel_buff
)
{
    int i, p;
    int seg_num;
    int num_segments = segmentation_ptr->num_segments;
    Segment_t3* seg_ptr;
    long initial_cpu_time = get_cpu_time();


    for (i=0; i<num_boundary_pixels; i++)
    {
        ERE(seg_num = get_good_segment_number(pixel_buff[ i ].seg_num));

        ASSERT(seg_num >= 0);

        if (seg_num > 0)
        {
            seg_ptr = segmentation_ptr->segments[ seg_num - 1 ];
            (seg_ptr->num_boundary_pixels)++;
        }
    }

    for (i=0; i<num_segments; i++)
    {
        seg_ptr = segmentation_ptr->segments[ i ];

        NRE(seg_ptr->boundary_pixels =
                    N_TYPE_MALLOC(Pixel_info, seg_ptr->num_boundary_pixels));
    }

    for (i=0; i<num_boundary_pixels; i++)
    {
        ERE(seg_num = get_good_segment_number(pixel_buff[ i ].seg_num));

        ASSERT(seg_num >= 0);

        if (seg_num > 0)
        {
            seg_ptr = segmentation_ptr->segments[ seg_num - 1 ];
            p = seg_ptr->boundary_pixel_count;

            seg_ptr->boundary_pixels[ p ].i = fs_cached_pixels[ i ].i;
            seg_ptr->boundary_pixels[ p ].j = fs_cached_pixels[ i ].j;

            (seg_ptr->boundary_pixel_count)++;

            ASSERT(seg_ptr->boundary_pixel_count <= seg_ptr->num_boundary_pixels);
        }
    }

    verbose_pso(5, "Finding boundary pixels of segments took %ldms.\n",
                get_cpu_time() - initial_cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_interior_points(Segmentation_t3* segmentation_ptr)
{
    Segment_t3** segments     = segmentation_ptr->segments;
    int             num_segments = segmentation_ptr->num_segments;
    int**           seg_map      = segmentation_ptr->seg_map;
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
        Segment_t3* seg_ptr = segments[ seg_num - 1 ];
        int            j_inside    = (int)(seg_ptr->j_CM);
        int            start_i     = NOT_SET;
        int            end_i;


        seg_ptr->j_inside = j_inside;

        ASSERT(seg_ptr->segment_number == seg_num);
        ASSERT(seg_ptr->i_min >= 0);
        ASSERT(seg_ptr->j_min >= 0);
        ASSERT(seg_ptr->i_max < segmentation_ptr->num_rows);
        ASSERT(seg_ptr->j_max < segmentation_ptr->num_cols);
        ASSERT(seg_ptr->i_CM >= seg_ptr->i_min);
        ASSERT(seg_ptr->j_CM >= seg_ptr->j_min);
        ASSERT(seg_ptr->i_CM <= seg_ptr->i_max);
        ASSERT(seg_ptr->j_CM <= seg_ptr->j_max);

#ifdef TEST
        verbose_pso(30, "Segment_t3 %d i is in (%d, %d), j is in (%d, %d) ",
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
            seg_ptr->i_inside = (int)(seg_ptr->i_CM);
        }
        else
        {
            int best_start = start_i;
            int best_length = 0;

            i = start_i;

            while (i < seg_ptr->i_max)
            {
                int length;
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

static int find_outside_boundary(Segmentation_t3* segmentation_ptr)
{
    static int prev_max_num_boundary_points = 0;
    Segment_t3**  segments     = segmentation_ptr->segments;
    int        num_segments = segmentation_ptr->num_segments;
    int        num_rows     = segmentation_ptr->num_rows;
    int        num_cols     = segmentation_ptr->num_cols;
    int**      mag_seg_map;
    int        mag_num_rows = 2 * num_rows;
    int        mag_num_cols = 2 * num_cols;
    int        j, k;
    long       cpu_time = get_cpu_time();
    int        seg_num;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    ERE(get_zero_int_matrix(&fs_cached_magnified_seg_map, mag_num_rows,
                            mag_num_cols));
    mag_seg_map = fs_cached_magnified_seg_map->elements;

    /*
    // First pass: Mark all boundary points.
    */
    for (seg_num = 1; seg_num <= num_segments; seg_num++)
    {
        Segment_t3* cur_seg_ptr         = segments[ seg_num - 1 ];
        int            num_boundary_pixels = cur_seg_ptr->num_boundary_pixels;
        Pixel_info*    boundary_pixels     = cur_seg_ptr->boundary_pixels;

        verbose_pso(150, "Segment_t3 %d has %d boundary pixels.\n",
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
        Segment_t3* cur_seg_ptr = segments[ seg_num - 1 ];
        int            start_j     = (int)(2.0 * cur_seg_ptr->j_CM);
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
            ERE(get_target_int_matrix(&fs_cached_magnified_boundary_points,
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

                    verbose_pso(300, "E==>(%3d, %3d)   ", ni, nj);
                    if (mag_seg_map[ ni ][ nj ] == seg_num)
                    {
                        verbose_pso(300, "found\n");

                        end_i = ni;
                        end_j = nj;
                        goto end_pixel_found;
                    }
                    verbose_pso(300, "\n");
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

                verbose_pso(300, "E==>(%3d, %3d)   ", ni, nj);
                if (mag_seg_map[ ni ][ nj ] == seg_num)
                {
                    verbose_pso(300, "found\n");

                    end_i = ni;
                    end_j = nj;
                    goto end_pixel_found;
                }
                verbose_pso(300, "\n");
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

                    verbose_pso(300, "E==>(%3d, %3d)   ", ni, nj);
                    if (mag_seg_map[ ni ][ nj ] == seg_num)
                    {
                        verbose_pso(300, "found\n");

                        end_i = ni;
                        end_j = nj;
                        goto end_pixel_found;
                    }
                    verbose_pso(300, "\n");
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

                            verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(300, "found\n");
                                if (ni == ii - 1) approach = SEG_LEFT;
                                goto add_pixel;
                            }
                            verbose_pso(300, "\n");
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

                        verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                        if (mag_seg_map[ ni ][ nj ] == seg_num)
                        {
                            approach = SEG_RIGHT;
                            verbose_pso(300, "found\n");
                            goto add_pixel;
                        }
                        verbose_pso(300, "\n");
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

                            verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(300, "found\n");

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
                            verbose_pso(300, "\n");
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

                            verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(300, "found\n");
                                if (ni == ii + 1) approach = SEG_RIGHT;
                                goto add_pixel;
                            }
                            verbose_pso(300, "\n");
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

                        verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                        if (mag_seg_map[ ni ][ nj ] == seg_num)
                        {
                            verbose_pso(300, "found\n");
                            approach = SEG_LEFT;
                            goto add_pixel;
                        }
                        verbose_pso(300, "\n");
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

                            verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(300, "found\n");

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
                            verbose_pso(300, "\n");
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

                            verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(300, "found\n");
                                if (nj == jj - 1) approach = SEG_DOWN;
                                goto add_pixel;
                            }
                            verbose_pso(300, "\n");
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

                        verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                        if (mag_seg_map[ ni ][ nj ] == seg_num)
                        {
                            verbose_pso(300, "found\n");
                            approach = SEG_UP;
                            goto add_pixel;
                        }
                        verbose_pso(300, "\n");
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

                            verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(300, "found\n");

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
                            verbose_pso(300, "\n");
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

                            verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(300, "found\n");
                                if (nj == jj + 1) approach = SEG_UP;
                                goto add_pixel;
                            }
                            verbose_pso(300, "\n");
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

                        verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                        if (mag_seg_map[ ni ][ nj ] == seg_num)
                        {
                            verbose_pso(300, "found\n");
                            approach = SEG_DOWN;
                            goto add_pixel;
                        }
                        verbose_pso(300, "\n");
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

                            verbose_pso(300, "(%3d, %3d)   ", ni, nj);
                            if (mag_seg_map[ ni ][ nj ] == seg_num)
                            {
                                verbose_pso(300, "found\n");

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
                            verbose_pso(300, "\n");
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

                fs_cached_magnified_boundary_points->elements[ count ][ 0 ] = ni;
                fs_cached_magnified_boundary_points->elements[ count ][ 1 ] = nj;

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
                    int mag_i = fs_cached_magnified_boundary_points->elements[k][0];
                    int mag_j = fs_cached_magnified_boundary_points->elements[k][1];
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

                verbose_pso(20, "Segment_t3 %d has %d outside boundary points.\n",
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

static int find_neighbours(Segmentation_t3* segmentation_ptr)
{
    static int prev_num_segments = 0;
    int**      seg_map      = segmentation_ptr->seg_map;
    int        num_segments = segmentation_ptr->num_segments;
    int        num_rows     = segmentation_ptr->num_rows;
    int        num_cols     = segmentation_ptr->num_cols;
    Segment_t3**  segment_ptr  = segmentation_ptr->segments;
    int        nb;
    int        seg_num;
    int        i, j, k;
    long       cpu_time     = get_cpu_time();


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    if (num_segments > prev_num_segments)
    {
        verbose_pso(10, "Bumping up storage for neighbours.\n");
        verbose_pso(10, "Now have %d segments.\n", num_segments);

        kjb_free(fs_cached_neighbours);
        NRE(fs_cached_neighbours = INT_MALLOC(num_segments));
        prev_num_segments = num_segments;
    }

    ERE(update_segment_pair_storage(num_segments, "finding neighbours"));

    for (i=0; i<num_segments; i++)
    {
        for (j=0; j<num_segments; j++)
        {
            fs_boundary_pair_counts[ i ][ j ] = 0;
        }
    }

    /*
    // For each boundary point, see if it is near another segment.
    */
    for (seg_num=1; seg_num <= num_segments; seg_num++)
    {
        int num_boundary_pixels = segment_ptr[ seg_num - 1 ]->num_boundary_pixels;
        Pixel_info* boundary_pixels = segment_ptr[ seg_num - 1 ]->boundary_pixels;

        for (j=0; j<num_boundary_pixels; j++)
        {
            int i_b = boundary_pixels[ j ].i;
            int j_b = boundary_pixels[ j ].j;
            int di, dj;

            for (di = -fs_connection_max_step; di <= fs_connection_max_step; di++)
            {
                for (dj = -fs_connection_max_step; dj <= fs_connection_max_step; dj++)
                {
                    int i_offset = i_b + di;
                    int j_offset = j_b + dj;
#ifdef TRY_IT_DIFFERENTLY
                    int step;
                    int weighted_step;
#endif

                    if (    (i_offset >= 0)
                         && (j_offset >= 0)
                         && (i_offset < num_rows)
                         && (j_offset < num_cols)
                         && ((nb = seg_map[ i_offset ][ j_offset ]) > 0)
                         && (nb != seg_num)
#ifdef COUNT_ONLY_BOUNDARY_CONNECTIONS
                         /* Condition for the neighbour to be on boundary.*/
                         && (    (    (i_offset > 0)
                                   && (seg_map[ i_offset - 1][ j_offset ] != nb)
                                 )
                             ||
                                (    (i_offset < num_rows - 1)
                                  && (seg_map[ i_offset + 1][ j_offset ] != nb)
                                )
                             ||
                                (    (j_offset > 0)
                                  && (seg_map[ i_offset ][ j_offset -1 ] != nb)
                                )
                             ||
                                (    (j_offset < num_cols - 1)
                                  && (seg_map[ i_offset ][ j_offset + 1 ] != nb)
                                )
                           )
#endif
                       )
                    {
                        if (nb == ERROR) return ERROR;

#ifdef TRY_IT_DIFFERENTLY
                        step = MAX_OF(ABS_OF(di), ABS_OF(dj));

                        /* Hack implemation of taking roots fast */
                        if (step < 3)
                        {
                            weighted_step = 3;
                        }
                        else if (step < 7)
                        {
                            weighted_step = 2;
                        }
                        else
                        {
                            weighted_step = 1;
                        }

                        (fs_boundary_pair_counts[ seg_num - 1 ][ nb - 1 ]) += weighted_step;
                        (fs_boundary_pair_counts[ nb - 1 ][ seg_num - 1 ]) += weighted_step;
#endif
                        (fs_boundary_pair_counts[ seg_num - 1 ][ nb - 1 ])++;
                        (fs_boundary_pair_counts[ nb - 1 ][ seg_num - 1 ])++;
                    }
                }
            }
        }
    }

    for (seg_num=1; seg_num <= num_segments; seg_num++)
    {
        int num_neighbours = 0;

        for (nb=1; nb <= num_segments; nb++)
        {
            if (fs_boundary_pair_counts[ seg_num - 1 ][ nb - 1 ] > 2.0 * fs_min_num_connections
                                                           * fs_connection_max_step
#ifndef COUNT_ONLY_BOUNDARY_CONNECTIONS
                                                           * fs_connection_max_step
#endif
                )
            {
                fs_cached_neighbours[ num_neighbours ] = nb;
                num_neighbours++;
            }
        }

        if (num_neighbours > 0)
        {
#ifdef TEST
            verbose_pso(50, "seg %3d : ", seg_num);
#endif

            NRE(segment_ptr[ seg_num - 1 ]->neighbours = INT_MALLOC(num_neighbours));
            NRE(segment_ptr[ seg_num - 1 ]->connection_counts = INT_MALLOC(num_neighbours));

            for (k = 0; k < num_neighbours; k++)
            {
                nb = fs_cached_neighbours[ k ];
                segment_ptr[ seg_num - 1 ]->neighbours[ k ] = nb;
                segment_ptr[ seg_num - 1 ]->connection_counts[ k ] = fs_boundary_pair_counts[ seg_num - 1 ][ nb - 1];

#ifdef TEST
                verbose_pso(50, "%d, ", fs_cached_neighbours[ k ]);
#endif
            }

            segment_ptr[ seg_num - 1 ]->num_neighbours = num_neighbours;

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

#ifdef COMPUTE_EDGE_STRENGTHS
static int get_edge_strengths
(
    const KJB_image* ip,
    Segmentation_t3*    segmentation_ptr
)
{
    int**     seg_map      = segmentation_ptr->seg_map;
    const int num_segments = segmentation_ptr->num_segments;
    int       num_rows     = ip->num_rows;
    int       num_cols     = ip->num_cols;
    int       i, j;
    long      initial_cpu_time = get_cpu_time();
    long      cpu_time_1;
    long      cpu_time_2;
    long      cpu_time_3;
    long      cpu_time_4;
    long      cpu_time_5;
    int       result           = 0;
    /* Perhaps make static some time. */
    Matrix*   edge_r_diffs_mp  = NULL;
    Matrix*   edge_g_diffs_mp  = NULL;
    Matrix*   edge_rel_sum_diffs_mp = NULL;


    /* Leave for a while! */
    UNTESTED_CODE();

    /*
    // Could merge some computations with merging via subroutines.
    */

#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    ERE(update_segment_pair_storage(num_segments, "finding edge strengths"));
    ERE(update_segment_merge_storage(num_segments, "finding edge strengths"));

    cpu_time_1 = get_cpu_time();
    verbose_pso(10, "Edge strength part one took %ldms.\n",
                cpu_time_1 - initial_cpu_time);

    /* Initilize merge chart to zeros. We only use one of the upper right
    // triangle of the chart, but it is easiest to initialize the whole thin. */

    for (i=0; i<num_segments; i++)
    {
        for (j=0; j<num_segments; j++)
        {
            fs_boundary_pair_counts[ i ][ j ] = 0;
            fs_properly_connected_pairs[ i ][ j ] = FALSE;
            fs_data_pair_counts[ i ][ j ] = 0;
            fs_merge_candidates[ i ][ j ] = 0;
            fs_merge_r_sum_mp->elements[ i ][ j ] = 0.0;
            fs_merge_g_sum_mp->elements[ i ][ j ] = 0.0;
            fs_merge_sum_RGM_sum_mp->elements[ i ][ j ] = 0.0;
        }
    }

    cpu_time_2 = get_cpu_time();
    verbose_pso(10, "Edge strength part two took %ldms.\n",
                cpu_time_2 - cpu_time_1);

    for (i = 0; i < num_segments; i++)
    {
        Segment_t3*    seg_ptr             = segmentation_ptr->segments[ i ];
        int         num_boundary_pixels = seg_ptr->num_boundary_pixels;
        Pixel_info* boundary_pixels     = seg_ptr->boundary_pixels;
        int         num_pixels          = seg_ptr->num_pixels;
        int         seg_num             = seg_ptr->segment_number;

        ASSERT(seg_num == i + 1);

        if (num_pixels < fs_merge_min_num_pixels) continue;

        for (j=0; j<num_boundary_pixels; j++)
        {
            int b_i = boundary_pixels[ j ].i;
            int b_j = boundary_pixels[ j ].j;

            if (    (b_i >= fs_merge_max_step)
                 && (b_j >= fs_merge_max_step)
                 && (b_i < num_rows - fs_merge_max_step)
                 && (b_j < num_cols - fs_merge_max_step)
               )
            {
                int ii, jj;
                int ii_min = b_i - fs_merge_max_step;
                int ii_max = b_i + fs_merge_max_step;
                int jj_min = b_j - fs_merge_max_step;
                int jj_max = b_j + fs_merge_max_step;
                float R1 = ip->pixels[ b_i ][ b_j ].r;
                float G1 = ip->pixels[ b_i ][ b_j ].g;
                float B1 = ip->pixels[ b_i ][ b_j ].b;
                float sum_1 = R1 + G1 + B1 + RGB_SUM_EPSILON;
                float r_1 = R1 / sum_1;
                float g_1 = G1 / sum_1;

                for (ii = ii_min; ii <= ii_max; ii++)
                {
                    for (jj = jj_min; jj <= jj_max; jj++)
                    {
                        int nb_seg_num = seg_map[ ii ][ jj ];

                        if (    (nb_seg_num > 0)
                             && (nb_seg_num != seg_num)
                             && (segmentation_ptr->segments[ nb_seg_num - 1 ]->num_pixels > fs_merge_min_num_pixels)
                           )
                        {
                            int s1 = seg_num - 1;
                            int s2 = nb_seg_num - 1;
                            float R2    = ip->pixels[ ii ][ jj ].r;
                            float G2    = ip->pixels[ ii ][ jj ].g;
                            float B2    = ip->pixels[ ii ][ jj ].b;
                            float sum_2 = R2 + G2 + B2 + RGB_SUM_EPSILON;
                            float r_2   = R2 / sum_2;
                            float g_2   = G2 / sum_2;

                            (fs_boundary_pair_counts[ s1 ][ s2 ])++;
                            (fs_boundary_pair_counts[ s2 ][ s1 ])++;

                            fs_merge_r_sum_mp->elements[ s1 ][ s2 ] += r_1;
                            fs_merge_g_sum_mp->elements[ s1 ][ s2 ] += g_1;
                            fs_merge_sum_RGM_sum_mp->elements[ s1 ][ s2 ] += sum_1;
                            fs_merge_r_sum_mp->elements[ s2 ][ s1 ] += r_2;
                            fs_merge_g_sum_mp->elements[ s2 ][ s1 ] += g_2;
                            fs_merge_sum_RGM_sum_mp->elements[ s2 ][ s1 ] += sum_2;
                            (fs_data_pair_counts[ s1 ][ s2 ])++;
                            (fs_data_pair_counts[ s2 ][ s1 ])++;
                        }
                    }
                }
             }
        }
    }

    cpu_time_3 = get_cpu_time();
    verbose_pso(10, "Merge part three took %ldms.\n",
                cpu_time_3 - cpu_time_2);

    ERE(get_initialized_matrix(&edge_r_diffs_mp, num_segments, num_segments,
                               DBL_NOT_SET));
    ERE(get_initialized_matrix(&edge_g_diffs_mp, num_segments, num_segments,
                               DBL_NOT_SET));
    ERE(get_initialized_matrix(&edge_rel_sum_diffs_mp,
                               num_segments, num_segments,
                               DBL_NOT_SET));

    for (i=0; i<num_segments; i++)
    {
        for (j = i + 1; j<num_segments; j++)
        {
            int count = (fs_boundary_pair_counts[i][j] + fs_boundary_pair_counts[j][i]) / 2;

            if (count > fs_merge_min_num_connections)
            {
                double boundary_r_mean_diff;
                double boundary_g_mean_diff;
                double boundary_sum_RGB_mean_rel_diff;
                double min_sum_diff;


                boundary_r_mean_diff = ABS_OF(fs_merge_r_sum_mp->elements[ i ][ j ] - fs_merge_r_sum_mp->elements[ j ][ i ]);
                boundary_r_mean_diff /= fs_data_pair_counts[ i ][ j ];

                boundary_g_mean_diff = ABS_OF(fs_merge_g_sum_mp->elements[ i ][ j ] - fs_merge_g_sum_mp->elements[ j ][ i ]);
                boundary_g_mean_diff /= fs_data_pair_counts[ i ][ j ];

                boundary_sum_RGB_mean_rel_diff = ABS_OF(fs_merge_sum_RGM_sum_mp->elements[ i ][ j ] - fs_merge_sum_RGM_sum_mp->elements[ j ][ i ]);
                boundary_sum_RGB_mean_rel_diff /= (fs_merge_sum_RGM_sum_mp->elements[ i ][ j ] + fs_merge_sum_RGM_sum_mp->elements[ j ][ i ]);
                boundary_sum_RGB_mean_rel_diff *= 2.0;

                min_sum_diff = MIN_OF(fs_merge_sum_RGM_sum_mp->elements[ i ][ j ],
                                      fs_merge_sum_RGM_sum_mp->elements[ j ][ i ]);

                if (min_sum_diff < fs_min_sum_RGB_for_chrom_test / 4.0)
                {
                    boundary_r_mean_diff = 0.0;
                    boundary_g_mean_diff = 0.0;
                }
                else if (min_sum_diff < fs_min_sum_RGB_for_chrom_test / 2.0)
                {
                    boundary_r_mean_diff /= 2.0;
                    boundary_g_mean_diff /= 2.0;
                }
                if (min_sum_diff > fs_min_sum_RGB_for_chrom_test)
                {
                    boundary_r_mean_diff /= 1.5;
                    boundary_g_mean_diff /= 1.5;
                }

                edge_r_diffs_mp->elements[ i ][ j ] = boundary_r_mean_diff;
                edge_r_diffs_mp->elements[ j ][ i ] = boundary_r_mean_diff;
                edge_g_diffs_mp->elements[ i ][ j ] = boundary_g_mean_diff;
                edge_g_diffs_mp->elements[ j ][ i ] = boundary_g_mean_diff;
                edge_rel_sum_diffs_mp->elements[ i ][ j ] = boundary_sum_RGB_mean_rel_diff;
                edge_rel_sum_diffs_mp->elements[ j ][ i ] = boundary_sum_RGB_mean_rel_diff;
            }
        }
    }

    cpu_time_4 = get_cpu_time();
    verbose_pso(10, "Edge strength part four took %ldms.\n",
                cpu_time_4 - cpu_time_3);

    for (i=0; i<segmentation_ptr->num_segments; i++)
    {
        Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];

        NRE(seg_ptr->nb_edge_r_diffs = DBL_MALLOC(seg_ptr->num_neighbours));
        NRE(seg_ptr->nb_edge_g_diffs = DBL_MALLOC(seg_ptr->num_neighbours));
        NRE(seg_ptr->nb_edge_rel_sum_diffs = DBL_MALLOC(seg_ptr->num_neighbours));

        for (j = 0; j < seg_ptr->num_neighbours; j++)
        {
            int nb_index = seg_ptr->neighbours[ j ] - 1;

             seg_ptr->nb_edge_r_diffs[ j ] = edge_r_diffs_mp->elements[ i ][ nb_index ];
             seg_ptr->nb_edge_g_diffs[ j ] = edge_g_diffs_mp->elements[ i ][ nb_index ];
             seg_ptr->nb_edge_rel_sum_diffs[ j ] = edge_rel_sum_diffs_mp->elements[ i ][ nb_index ];
        }
    }

    cpu_time_5 = get_cpu_time();
    verbose_pso(10, "Edge strength part five took %ldms.\n",
                cpu_time_5 - cpu_time_4);

    free_matrix(edge_r_diffs_mp);
    free_matrix(edge_g_diffs_mp);
    free_matrix(edge_rel_sum_diffs_mp);

    verbose_pso(5, "Edge strength took %ldms.\n",
                get_cpu_time() - initial_cpu_time);

    return result;
}
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
static int verify_segmentation(Segmentation_t3* segmentation_ptr)
{
    Segment_t3** segments     = segmentation_ptr->segments;
    int       num_segments = segmentation_ptr->num_segments;
    int**     seg_map      = segmentation_ptr->seg_map;
    int       num_rows     = segmentation_ptr->num_rows;
    int       num_cols     = segmentation_ptr->num_cols;
    int       i, j;
    long      cpu_time     = get_cpu_time();
    int       seg_num;
    int       total_num_good_pixels = 0;
    int       num_positive_seg_nums = 0;


#ifdef TRACK_MEMORY_ALLOCATION
    prepare_memory_cleanup();
#endif

    for (seg_num = 1; seg_num <= num_segments; seg_num++)
    {
        Segment_t3* seg_ptr = segments[ seg_num - 1 ];
        int            j_inside    = seg_ptr->j_inside;
        int            i_inside    = seg_ptr->i_inside;


        ASSERT(seg_ptr->segment_number == seg_num);
        ASSERT(seg_ptr->i_min >= 0);
        ASSERT(seg_ptr->j_min >= 0);
        ASSERT(seg_ptr->i_max < segmentation_ptr->num_rows);
        ASSERT(seg_ptr->j_max < segmentation_ptr->num_cols);
        ASSERT(seg_ptr->i_CM >= seg_ptr->i_min);
        ASSERT(seg_ptr->j_CM >= seg_ptr->j_min);
        ASSERT(seg_ptr->i_CM <= seg_ptr->i_max);
        ASSERT(seg_ptr->j_CM <= seg_ptr->j_max);
        ASSERT(i_inside >= seg_ptr->i_min);
        ASSERT(j_inside >= seg_ptr->j_min);
        ASSERT(i_inside <= seg_ptr->i_max);
        ASSERT(j_inside <= seg_ptr->j_max);
        ASSERT(seg_map[ i_inside ][ j_inside ] == seg_num);
        ASSERT(seg_ptr->num_pixels >= fs_seg_min_segment_size);

        ASSERT(seg_ptr->num_pixels == count_connected_pixels(
                                                        segmentation_ptr,
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

            if (non_corner_connect_count + corner_connect_count < 8)
            {
                is_boundary_pixel = TRUE;
            }

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
        // it is part of the region), and also must be connected to something
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
                if (fs_seg_connect_corners)
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
    Segmentation_t3* segmentation_ptr,
    int           i,
    int           j
)
{
    int** seg_map     = segmentation_ptr->seg_map;
    int   num_rows    = segmentation_ptr->num_rows;
    int   num_cols    = segmentation_ptr->num_cols;
    int   cur_seg     = seg_map[ i ][ j ];
    int   count;
    int   count_2;
    int   check_sum   = 0;
    int   check_sum_2 = 0;
    int   ii, jj;


    ASSERT(fs_cached_seg_map == seg_map);
    ASSERT(fs_cached_num_rows == num_rows);
    ASSERT(fs_cached_num_cols == num_cols);

    for (ii=0; ii<num_rows; ii++)
    {
        for (jj=0; jj<num_cols; jj++)
        {
            check_sum += seg_map[ ii ][ jj ];
        }
    }

    fs_count_remark = 0;

    remark_segment(i, j, -1000);

    count = fs_count_remark;
    fs_count_remark = 0;
    remark_segment(i, j, cur_seg);
    count_2 = fs_count_remark;
    fs_count_remark = 0;

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
    int (*seg_fn)(int,int) =  (int(*)(int,int))fs_segmentation_methods[ fs_segmentation_method ].fn;

    num_reseg = 1;

    /* There is no point in re-segmenting, unless we are fixing the overall
    // colour (as opposed to only relying on differences.)
    */
    if (    (fs_max_abs_RGB_var >= FLT_ZERO)
         || (fs_max_rel_RGB_var >= FLT_ZERO)
         || (fs_max_abs_chrom_var >= FLT_ZERO)
         || (fs_max_rel_chrom_var >= FLT_ZERO)
         || (fs_max_abs_sum_RGB_var >= FLT_ZERO)
         || (fs_max_rel_sum_RGB_var >= FLT_ZERO)
       )
    {
        num_reseg += fs_seg_resegment_level;
    }

    for (reseg = 0; reseg < num_reseg; reseg++)
    {
        if (reseg != 0)
        {
            int k;

            R = fs_cached_R_sum / fs_num_cached_pixels;
            G = fs_cached_G_sum / fs_num_cached_pixels;
            B = fs_cached_B_sum / fs_num_cached_pixels;
            r_chrom = fs_cached_r_chrom_sum / fs_num_cached_pixels;
            g_chrom = fs_cached_g_chrom_sum / fs_num_cached_pixels;
            sum_RGB = R + G + B + RGB_SUM_EPSILON;

            /*
            // If we are going to re-segment, we have to undo what we just did
            // to the marked pixel database.
            */
            for (k=0; k<fs_num_cached_pixels; k++)
            {
                int ii = fs_cached_pixels[ k ].i;
                int jj = fs_cached_pixels[ k ].j;

                fs_cached_seg_map[ ii ][ jj ] = 0;
            }
        }
        else
        {
            R = fs_cached_ip->pixels[ i ][ j ].r;
            G = fs_cached_ip->pixels[ i ][ j ].g;
            B = fs_cached_ip->pixels[ i ][ j ].b;
            sum_RGB = R + G + B + RGB_SUM_EPSILON;
            r_chrom = R / sum_RGB;
            g_chrom = G / sum_RGB;
        }

        fs_num_cached_pixels = 0;

        fs_cached_r_chrom_sum     = 0.0;
        fs_cached_g_chrom_sum     = 0.0;
        fs_cached_R_sum           = 0.0;
        fs_cached_G_sum           = 0.0;
        fs_cached_B_sum           = 0.0;
#ifdef MAINTAIN_SS
        fs_cached_r_chrom_sqr_sum = 0.0;
        fs_cached_g_chrom_sqr_sum = 0.0;
        fs_cached_sum_RGB_sqr_sum = 0.0;
#endif

#ifdef USE_NO_VALUE
        if (    (fs_max_abs_RGB_var >= FLT_ZERO)
             && (fs_max_rel_RGB_var >= FLT_ZERO))
        {
            fs_cached_R_min = MAX_OF(R - fs_max_abs_RGB_var,
                                  R*(1.0 - fs_max_rel_RGB_var));
            fs_cached_R_max = MIN_OF(R + fs_max_abs_RGB_var,
                                  R*(1.0 + fs_max_rel_RGB_var));

            fs_cached_G_min = MAX_OF(G - fs_max_abs_RGB_var,
                                  G*(1.0 - fs_max_rel_RGB_var));
            fs_cached_G_max = MIN_OF(G + fs_max_abs_RGB_var,
                                  G*(1.0 + fs_max_rel_RGB_var));

            fs_cached_B_min = MAX_OF(B - fs_max_abs_RGB_var,
                                  B*(1.0 - fs_max_rel_RGB_var));
            fs_cached_B_max = MIN_OF(B + fs_max_abs_RGB_var,
                                  B*(1.0 + fs_max_rel_RGB_var));
        }
        else if (fs_max_abs_RGB_var >= FLT_ZERO)
        {
            fs_cached_R_min = R - fs_max_abs_RGB_var;
            fs_cached_R_max = R + fs_max_abs_RGB_var;
            fs_cached_G_min = G - fs_max_abs_RGB_var;
            fs_cached_G_max = G + fs_max_abs_RGB_var;
            fs_cached_B_min = B - fs_max_abs_RGB_var;
            fs_cached_B_max = B + fs_max_abs_RGB_var;
        }
        else if (fs_max_rel_RGB_var >= FLT_ZERO)
        {
            fs_cached_R_min = R*(1.0 - fs_max_rel_RGB_var);
            fs_cached_R_max = R*(1.0 + fs_max_rel_RGB_var);
            fs_cached_G_min = G*(1.0 - fs_max_rel_RGB_var);
            fs_cached_G_max = G*(1.0 + fs_max_rel_RGB_var);
            fs_cached_B_min = B*(1.0 - fs_max_rel_RGB_var);
            fs_cached_B_max = B*(1.0 + fs_max_rel_RGB_var);
        }
        else
        {
            fs_cached_R_min = FLT_NOT_SET;
            fs_cached_R_max = FLT_NOT_SET;
            fs_cached_G_min = FLT_NOT_SET;
            fs_cached_G_max = FLT_NOT_SET;
            fs_cached_B_min = FLT_NOT_SET;
            fs_cached_B_max = FLT_NOT_SET;
        }

        if (    (fs_max_abs_chrom_var >= FLT_ZERO)
             && (fs_max_rel_chrom_var >= FLT_ZERO))
        {
            fs_cached_r_chrom_min = MAX_OF(r_chrom - fs_max_abs_chrom_var,
                                        r_chrom*(1.0 - fs_max_rel_chrom_var));
            fs_cached_r_chrom_max = MIN_OF(r_chrom + fs_max_abs_chrom_var,
                                        r_chrom*(1.0 + fs_max_rel_chrom_var));

            fs_cached_g_chrom_min = MAX_OF(g_chrom - fs_max_abs_chrom_var,
                                        g_chrom*(1.0 - fs_max_rel_chrom_var));
            fs_cached_g_chrom_max = MIN_OF(g_chrom + fs_max_abs_chrom_var,
                                        g_chrom*(1.0 + fs_max_rel_chrom_var));
        }
        else if (fs_max_abs_chrom_var >= FLT_ZERO)
        {
            fs_cached_r_chrom_min = r_chrom - fs_max_abs_chrom_var;
            fs_cached_r_chrom_max = r_chrom + fs_max_abs_chrom_var;
            fs_cached_g_chrom_min = g_chrom - fs_max_abs_chrom_var;
            fs_cached_g_chrom_max = g_chrom + fs_max_abs_chrom_var;
        }
        else if (fs_max_rel_chrom_var >= FLT_ZERO)
        {
            fs_cached_r_chrom_min = r_chrom*(1.0 - fs_max_rel_chrom_var);
            fs_cached_r_chrom_max = r_chrom*(1.0 + fs_max_rel_chrom_var);
            fs_cached_g_chrom_min = g_chrom*(1.0 - fs_max_rel_chrom_var);
            fs_cached_g_chrom_max = g_chrom*(1.0 + fs_max_rel_chrom_var);
        }
        else
        {
            fs_cached_r_chrom_min = FLT_NOT_SET;
            fs_cached_r_chrom_max = FLT_NOT_SET;
            fs_cached_g_chrom_min = FLT_NOT_SET;
            fs_cached_g_chrom_max = FLT_NOT_SET;
        }

        if (    (fs_max_abs_sum_RGB_var >= FLT_ZERO)
             && (fs_max_rel_sum_RGB_var >= FLT_ZERO))
        {
            fs_cached_sum_RGB_min = MIN_OF(sum_RGB - fs_max_abs_sum_RGB_var,
                                        sum_RGB*(1.0 - fs_max_rel_sum_RGB_var));
            fs_cached_sum_RGB_max = MAX_OF(sum_RGB + fs_max_abs_sum_RGB_var,
                                        sum_RGB*(1.0 + fs_max_rel_sum_RGB_var));
        }
        else if (fs_max_abs_sum_RGB_var >= FLT_ZERO)
        {
            fs_cached_sum_RGB_min = sum_RGB - fs_max_abs_sum_RGB_var;
            fs_cached_sum_RGB_max = sum_RGB + fs_max_abs_sum_RGB_var;
        }
        else if (fs_max_rel_sum_RGB_var >= FLT_ZERO)
        {
            fs_cached_sum_RGB_min = sum_RGB*(1.0 - fs_max_rel_sum_RGB_var);
            fs_cached_sum_RGB_max = sum_RGB*(1.0 + fs_max_rel_sum_RGB_var);
        }
        else
        {
            fs_cached_sum_RGB_min = FLT_NOT_SET;
            fs_cached_sum_RGB_max = FLT_NOT_SET;
        }
#else
        fs_cached_R_min = MAX_OF(R - fs_max_abs_RGB_var,
                              R*(1.0 - fs_max_rel_RGB_var));
        fs_cached_R_max = MIN_OF(R + fs_max_abs_RGB_var,
                              R*(1.0 + fs_max_rel_RGB_var));

        fs_cached_G_min = MAX_OF(G - fs_max_abs_RGB_var,
                              G*(1.0 - fs_max_rel_RGB_var));
        fs_cached_G_max = MIN_OF(G + fs_max_abs_RGB_var,
                              G*(1.0 + fs_max_rel_RGB_var));

        fs_cached_B_min = MAX_OF(B - fs_max_abs_RGB_var,
                              B*(1.0 - fs_max_rel_RGB_var));
        fs_cached_B_max = MIN_OF(B + fs_max_abs_RGB_var,
                              B*(1.0 + fs_max_rel_RGB_var));

        fs_cached_r_chrom_min = MAX_OF(r_chrom - fs_max_abs_chrom_var,
                                    r_chrom*(1.0 - fs_max_rel_chrom_var));
        fs_cached_r_chrom_max = MIN_OF(r_chrom + fs_max_abs_chrom_var,
                                    r_chrom*(1.0 + fs_max_rel_chrom_var));

        fs_cached_g_chrom_min = MAX_OF(g_chrom - fs_max_abs_chrom_var,
                                    g_chrom*(1.0 - fs_max_rel_chrom_var));
        fs_cached_g_chrom_max = MIN_OF(g_chrom + fs_max_abs_chrom_var,
                                    g_chrom*(1.0 + fs_max_rel_chrom_var));
        fs_cached_sum_RGB_min = MAX_OF(sum_RGB - fs_max_abs_sum_RGB_var,
                                    sum_RGB*(1.0 - fs_max_rel_sum_RGB_var));
        fs_cached_sum_RGB_max = MIN_OF(sum_RGB + fs_max_abs_sum_RGB_var,
                                    sum_RGB*(1.0 + fs_max_rel_sum_RGB_var));
#endif

#ifdef TEST
        if (reseg != 0)
        {
            verbose_pso(300, "Re-");
        }
        verbose_pso(300, "Growing (%-3d, %-3d) ... ", i, j);
#endif

        seg_fn(i, j);

#ifdef TEST
        verbose_pso(300, "%d pixels.\n", fs_num_cached_pixels);
#endif

        if (fs_num_cached_pixels < fs_seg_min_resegment_size)
        {
            break;
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int grow_segment(int i, int j)
{
#ifdef CONNECT_CORNER_OPTION
    if (fs_seg_connect_corners)
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

static int grow_segment_using_rg_and_lum_var(int i, int j)
{
#ifdef CONNECT_CORNER_OPTION
    if (fs_seg_connect_corners)
    {
        grow_segment_using_rg_and_lum_var_8(i, j);
    }
    else
    {
        grow_segment_using_rg_and_lum_var_4(i, j);
    }
#else
#ifdef DEFAULT_IS_TO_CONNECT_CORNERS
    grow_segment_using_rg_and_lum_var_8(i, j);
#endif
    grow_segment_using_rg_and_lum_var_4(i, j);
#endif

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_segment_4(int i, int j)
{
    int        count;
    int        i_offset;
    int        j_offset;
    float      cur_R;
    float      cur_G;
    float      cur_B;
    float      cur_r_chrom;
    float      cur_g_chrom;
    float      cur_sum_RGB;
    float      cur_RGB_sqr;
    Pixel      cur_image_pixel;
#ifdef GROW_RANDOMLY
    static int call_count      = 0;
#endif


    cur_image_pixel = fs_cached_ip->pixels[ i ][ j ];

    ASSERT(! (fs_cached_seg_map[ i ][ j ]));
    ASSERT(cur_image_pixel.extra.invalid.pixel == VALID_PIXEL);

    fs_cached_pixels[ fs_num_cached_pixels ].i = i;
    fs_cached_pixels[ fs_num_cached_pixels ].j = j;

    cur_R = cur_image_pixel.r;
    cur_G = cur_image_pixel.g;
    cur_B = cur_image_pixel.b;

    cur_sum_RGB = cur_R + cur_G + cur_B + RGB_SUM_EPSILON;
    cur_r_chrom = cur_R / cur_sum_RGB;
    cur_g_chrom = cur_G / cur_sum_RGB;
    cur_RGB_sqr = (cur_R * cur_R) + (cur_G * cur_G) + (cur_B * cur_B);

    fs_cached_r_chrom_sum += cur_r_chrom;
    fs_cached_g_chrom_sum += cur_g_chrom;
    fs_cached_R_sum += cur_R;
    fs_cached_G_sum += cur_G;
    fs_cached_B_sum += cur_B;
#ifdef MAINTAIN_SS
    fs_cached_r_chrom_sqr_sum += (cur_r_chrom * cur_r_chrom);
    fs_cached_g_chrom_sqr_sum += (cur_g_chrom * cur_g_chrom);
    fs_cached_sum_RGB_sqr_sum += (cur_sum_RGB * cur_sum_RGB);
#endif

    fs_num_cached_pixels++;

    ASSERT(fs_num_cached_pixels <= fs_cached_num_rows * fs_cached_num_cols);

    fs_cached_seg_map[ i ][ j ] = fs_cached_segment_count;

#ifdef GROW_RANDOMLY
    for (count = 0; count < 4; count++)
    {
        i_offset = i + fs_direction_permutations_4[ call_count ][ count ][ 0 ];
        j_offset = j + fs_direction_permutations_4[ call_count ][ count ][ 1 ];
#else
    for (count = 0; count < 2; count++)
    {
        int d;

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
#endif

            if (    (i_offset >= 0)
                 && (j_offset >= 0)
                 && (i_offset < fs_cached_num_rows)
                 && (j_offset < fs_cached_num_cols)
                 && ( ! fs_cached_seg_map[ i_offset ][ j_offset ])
                 && (fs_cached_ip->pixels[ i_offset ][ j_offset ].extra.invalid.pixel == VALID_PIXEL)
               )
            {
                int skip = FALSE;
                float R = fs_cached_ip->pixels[ i_offset ][ j_offset ].r;
                float G = fs_cached_ip->pixels[ i_offset ][ j_offset ].g;
                float B = fs_cached_ip->pixels[ i_offset ][ j_offset ].b;
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
                if ((fs_cached_R_min >= FLT_ZERO) && (R < fs_cached_R_min))
                {
                    skip = TRUE;
                }
                else if ((fs_cached_R_max >= FLT_ZERO) && (R > fs_cached_R_max))
                {
                    skip = TRUE;
                }
                else if ((fs_cached_G_min >= FLT_ZERO) && (G < fs_cached_G_min))
                {
                    skip = TRUE;
                }
                else if ((fs_cached_G_max >= FLT_ZERO) && (G > fs_cached_G_max))
                {
                    skip = TRUE;
                }
                else if ((fs_cached_B_min >= FLT_ZERO) && (B < fs_cached_B_min))
                {
                    skip = TRUE;
                }
                else if ((fs_cached_B_max >= FLT_ZERO) && (B > fs_cached_B_max))
                {
                    skip = TRUE;
                }
                else if (    (fs_cached_r_chrom_min >= FLT_ZERO)
                          && (r_chrom < fs_cached_r_chrom_min)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                        )
                {
                    skip = TRUE;
                }
                else if (    (fs_cached_r_chrom_max >= FLT_ZERO)
                          && (r_chrom > fs_cached_r_chrom_max)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                        )
                {
                    skip = TRUE;
                }
                else if (    (fs_cached_g_chrom_min >= FLT_ZERO)
                          && (g_chrom < fs_cached_g_chrom_min)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                        )
                {
                    skip = TRUE;
                }
                else if (    (fs_cached_g_chrom_max >= FLT_ZERO)
                          && (g_chrom > fs_cached_g_chrom_max)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                        )
                {
                    skip = TRUE;
                }
                else if (    (fs_cached_sum_RGB_min >= FLT_ZERO)
                          && (sum_RGB < fs_cached_sum_RGB_min)
                        )
                {
                    skip = TRUE;
                }
                else if (    (fs_cached_sum_RGB_max >= FLT_ZERO)
                          && (sum_RGB > fs_cached_sum_RGB_max)
                        )
                {
                    skip = TRUE;
                }
                else if (    (fs_max_abs_RGB_sqr_diff >= FLT_ZERO)
                          && (d_RGB_sqr > fs_max_abs_RGB_sqr_diff)
                        )
                {
                    skip = TRUE;
                }
                else if (    (fs_max_rel_RGB_sqr_diff >= FLT_ZERO)
                          && (d_rel_RGB_sqr > fs_max_rel_RGB_sqr_diff)
                        )
                {
                    skip = TRUE;
                }
                else if (    (fs_max_abs_sum_RGB_diff >= FLT_ZERO)
                          && (d_sum_RGB > fs_max_abs_sum_RGB_diff)
                        )
                {
                    skip = TRUE;
                }
                else if (    (fs_max_rel_sum_RGB_diff >= FLT_ZERO)
                          && (sum_RGB > fs_min_sum_RGB_for_relative_sum_diff)
                          && (d_rel_sum_RGB > fs_max_rel_sum_RGB_diff)
                        )
                {
                    skip = TRUE;
                }
                else if (    (fs_max_abs_chrom_diff >= FLT_ZERO)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                          && (    (d_r_chrom > fs_max_abs_chrom_diff)
                               || (d_g_chrom > fs_max_abs_chrom_diff)
                             )
                        )
                {
                    skip = TRUE;
                }
                else if (    (fs_max_rel_chrom_diff >= FLT_ZERO)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                          && (    (d_rel_r_chrom > fs_max_rel_chrom_diff)
                               || (d_rel_g_chrom > fs_max_rel_chrom_diff)
                             )
                        )
                {
                    skip = TRUE;
                }
#else
                if (R < fs_cached_R_min)
                {
                    skip = TRUE;
                }
                else if (R > fs_cached_R_max)
                {
                    skip = TRUE;
                }
                else if (G < fs_cached_G_min)
                {
                    skip = TRUE;
                }
                else if (G > fs_cached_G_max)
                {
                    skip = TRUE;
                }
                else if (B < fs_cached_B_min)
                {
                    skip = TRUE;
                }
                else if (B > fs_cached_B_max)
                {
                    skip = TRUE;
                }
                else if (    (r_chrom < fs_cached_r_chrom_min)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                        )
                {
                    skip = TRUE;
                }
                else if (    (r_chrom > fs_cached_r_chrom_max)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                        )
                {
                    skip = TRUE;
                }
                else if (    (g_chrom < fs_cached_g_chrom_min)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                        )
                {
                    skip = TRUE;
                }
                else if (    (g_chrom > fs_cached_g_chrom_max)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                        )
                {
                    skip = TRUE;
                }
                else if (sum_RGB < fs_cached_sum_RGB_min)
                {
                    skip = TRUE;
                }
                else if (sum_RGB > fs_cached_sum_RGB_max)
                {
                    skip = TRUE;
                }
                else if (d_RGB_sqr > fs_max_abs_RGB_sqr_diff)
                {
                    skip = TRUE;
                }
                else if (d_rel_RGB_sqr > fs_max_rel_RGB_sqr_diff)
                {
                    skip = TRUE;
                }
                else if (d_sum_RGB > fs_max_abs_sum_RGB_diff)
                {
                    skip = TRUE;
                }
                else if (    (d_rel_sum_RGB > fs_max_rel_sum_RGB_diff)
                          && (sum_RGB > fs_min_sum_RGB_for_relative_sum_diff)
                        )
                {
                    skip = TRUE;
                }
                else if (    (d_r_chrom > fs_max_abs_chrom_diff)
                          || (d_g_chrom > fs_max_abs_chrom_diff)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                        )
                {
                    skip = TRUE;
                }
                else if (    (d_rel_r_chrom > fs_max_rel_chrom_diff)
                          || (d_rel_g_chrom > fs_max_rel_chrom_diff)
                          && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                        )
                {
                    skip = TRUE;
                }
#endif

                if ( ! skip )
                {
                    grow_segment_4(i_offset, j_offset);
                }
            }
#ifndef GROW_RANDOMLY
        }
#endif
    }
#ifdef GROW_RANDOMLY
    call_count++;
    if (call_count == 24) call_count = 0;
#endif
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


    cur_image_pixel = fs_cached_ip->pixels[ i ][ j ];

    ASSERT(! (fs_cached_seg_map[ i ][ j ]));
    ASSERT(cur_image_pixel.extra.invalid.pixel == VALID_PIXEL);

    fs_cached_pixels[ fs_num_cached_pixels ].i = i;
    fs_cached_pixels[ fs_num_cached_pixels ].j = j;

    cur_R = cur_image_pixel.r;
    cur_G = cur_image_pixel.g;
    cur_B = cur_image_pixel.b;

    cur_sum_RGB = cur_R + cur_G + cur_B + RGB_SUM_EPSILON;
    cur_r_chrom = cur_R / cur_sum_RGB;
    cur_g_chrom = cur_G / cur_sum_RGB;
    cur_RGB_sqr = (cur_R * cur_R) + (cur_G * cur_G) + (cur_B * cur_B);

    fs_cached_r_chrom_sum += cur_r_chrom;
    fs_cached_g_chrom_sum += cur_g_chrom;
    fs_cached_R_sum += cur_R;
    fs_cached_G_sum += cur_G;
    fs_cached_B_sum += cur_B;
#ifdef MAINTAIN_SS
    fs_cached_r_chrom_sqr_sum += (cur_r_chrom * cur_r_chrom);
    fs_cached_g_chrom_sqr_sum += (cur_g_chrom * cur_g_chrom);
    fs_cached_sum_RGB_sqr_sum += (cur_sum_RGB * cur_sum_RGB);
#endif

    fs_num_cached_pixels++;

    ASSERT(fs_num_cached_pixels <= fs_cached_num_rows * fs_cached_num_cols);

    fs_cached_seg_map[ i ][ j ] = fs_cached_segment_count;

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
                     && (i_offset < fs_cached_num_rows)
                     && (j_offset < fs_cached_num_cols)
                     && ( ! fs_cached_seg_map[ i_offset ][ j_offset ])
                     && (fs_cached_ip->pixels[ i_offset ][ j_offset ].extra.invalid.pixel == VALID_PIXEL)
                   )
                {
                    int skip = FALSE;
                    float R = fs_cached_ip->pixels[ i_offset ][ j_offset ].r;
                    float G = fs_cached_ip->pixels[ i_offset ][ j_offset ].g;
                    float B = fs_cached_ip->pixels[ i_offset ][ j_offset ].b;
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
                    if ((fs_cached_R_min >= FLT_ZERO) && (R < fs_cached_R_min))
                    {
                        skip = TRUE;
                    }
                    else if ((fs_cached_R_max >= FLT_ZERO) && (R > fs_cached_R_max))
                    {
                        skip = TRUE;
                    }
                    else if ((fs_cached_G_min >= FLT_ZERO) && (G < fs_cached_G_min))
                    {
                        skip = TRUE;
                    }
                    else if ((fs_cached_G_max >= FLT_ZERO) && (G > fs_cached_G_max))
                    {
                        skip = TRUE;
                    }
                    else if ((fs_cached_B_min >= FLT_ZERO) && (B < fs_cached_B_min))
                    {
                        skip = TRUE;
                    }
                    else if ((fs_cached_B_max >= FLT_ZERO) && (B > fs_cached_B_max))
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_cached_r_chrom_min >= FLT_ZERO)
                              && (r_chrom < fs_cached_r_chrom_min)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_cached_r_chrom_max >= FLT_ZERO)
                              && (r_chrom > fs_cached_r_chrom_max)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_cached_g_chrom_min >= FLT_ZERO)
                              && (g_chrom < fs_cached_g_chrom_min)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_cached_g_chrom_max >= FLT_ZERO)
                              && (g_chrom > fs_cached_g_chrom_max)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_cached_sum_RGB_min >= FLT_ZERO)
                              && (sum_RGB < fs_cached_sum_RGB_min)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_cached_sum_RGB_max >= FLT_ZERO)
                              && (sum_RGB > fs_cached_sum_RGB_max)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_max_abs_RGB_sqr_diff >= FLT_ZERO)
                              && (d_RGB_sqr > fs_max_abs_RGB_sqr_diff)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_max_rel_RGB_sqr_diff >= FLT_ZERO)
                              && (d_rel_RGB_sqr > fs_max_rel_RGB_sqr_diff)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_max_abs_sum_RGB_diff >= FLT_ZERO)
                              && (d_sum_RGB > fs_max_abs_sum_RGB_diff)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_max_rel_sum_RGB_diff >= FLT_ZERO)
                              && (sum_RGB > fs_min_sum_RGB_for_relative_sum_diff)
                              && (d_rel_sum_RGB > fs_max_rel_sum_RGB_diff)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_max_abs_chrom_diff >= FLT_ZERO)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                              && (    (d_r_chrom > fs_max_abs_chrom_diff)
                                   || (d_g_chrom > fs_max_abs_chrom_diff)
                                 )
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (fs_max_rel_chrom_diff >= FLT_ZERO)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                              && (    (d_rel_r_chrom > fs_max_rel_chrom_diff)
                                   || (d_rel_g_chrom > fs_max_rel_chrom_diff)
                                 )
                            )
                    {
                        skip = TRUE;
                    }
#else
                    if (R < fs_cached_R_min)
                    {
                        skip = TRUE;
                    }
                    else if (R > fs_cached_R_max)
                    {
                        skip = TRUE;
                    }
                    else if (G < fs_cached_G_min)
                    {
                        skip = TRUE;
                    }
                    else if (G > fs_cached_G_max)
                    {
                        skip = TRUE;
                    }
                    else if (B < fs_cached_B_min)
                    {
                        skip = TRUE;
                    }
                    else if (B > fs_cached_B_max)
                    {
                        skip = TRUE;
                    }
                    else if (    (r_chrom < fs_cached_r_chrom_min)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (r_chrom > fs_cached_r_chrom_max)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (g_chrom < fs_cached_g_chrom_min)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (g_chrom > fs_cached_g_chrom_max)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (sum_RGB < fs_cached_sum_RGB_min)
                    {
                        skip = TRUE;
                    }
                    else if (sum_RGB > fs_cached_sum_RGB_max)
                    {
                        skip = TRUE;
                    }
                    else if (d_RGB_sqr > fs_max_abs_RGB_sqr_diff)
                    {
                        skip = TRUE;
                    }
                    else if (d_rel_RGB_sqr > fs_max_rel_RGB_sqr_diff)
                    {
                        skip = TRUE;
                    }
                    else if (d_sum_RGB > fs_max_abs_sum_RGB_diff)
                    {
                        skip = TRUE;
                    }
                    else if (    (d_rel_sum_RGB > fs_max_rel_sum_RGB_diff)
                              && (sum_RGB > fs_min_sum_RGB_for_relative_sum_diff)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                              && (     (d_r_chrom > fs_max_abs_chrom_diff)
                                    || (d_g_chrom > fs_max_abs_chrom_diff)
                                 )
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (d_rel_r_chrom > fs_max_rel_chrom_diff)
                              || (d_rel_g_chrom > fs_max_rel_chrom_diff)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
#endif

                    if ( ! skip )
                    {
                        grow_segment_8(i_offset, j_offset);
                    }
                }
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_segment_using_rg_and_lum_var_4(int i, int j)
{
    int   count;
    int   i_offset;
    int   j_offset;
    float cur_R;
    float cur_G;
    float cur_B;
    float cur_sum_RGB;
    float cur_r_chrom;
    float cur_g_chrom;
    Pixel* cur_image_pixel_ptr;
#ifdef GROW_RANDOMLY
    static int call_count = 0;
#endif


    cur_image_pixel_ptr = &(fs_cached_ip->pixels[ i ][ j ]);

    ASSERT(! (fs_cached_seg_map[ i ][ j ]));
    ASSERT(cur_image_pixel_ptr->extra.invalid.pixel == VALID_PIXEL);

    fs_cached_pixels[ fs_num_cached_pixels ].i = i;
    fs_cached_pixels[ fs_num_cached_pixels ].j = j;

    cur_R = cur_image_pixel_ptr->r;
    cur_G = cur_image_pixel_ptr->g;
    cur_B = cur_image_pixel_ptr->b;
    cur_sum_RGB = cur_R + cur_G + cur_B + RGB_SUM_EPSILON;
    cur_r_chrom = cur_R / cur_sum_RGB;
    cur_g_chrom = cur_G / cur_sum_RGB;

    fs_cached_R_sum += cur_R;
    fs_cached_G_sum += cur_G;
    fs_cached_B_sum += cur_B;
    fs_cached_r_chrom_sum += cur_r_chrom;
    fs_cached_g_chrom_sum += cur_g_chrom;
#ifdef MAINTAIN_SS
    fs_cached_r_chrom_sqr_sum += (cur_r_chrom * cur_r_chrom);
    fs_cached_g_chrom_sqr_sum += (cur_g_chrom * cur_g_chrom);
    fs_cached_sum_RGB_sqr_sum += (cur_sum_RGB * cur_sum_RGB);
#endif

    fs_num_cached_pixels++;

    fs_cached_seg_map[ i ][ j ] = fs_cached_segment_count;

    /* The code here should be kept consistent with that in grow_segment. */

#ifdef GROW_RANDOMLY
    for (count = 0; count < 4; count++)
    {
        i_offset = i + fs_direction_permutations_4[ call_count ][ count ][ 0 ];
        j_offset = j + fs_direction_permutations_4[ call_count ][ count ][ 1 ];
#else
    for (count = 0; count < 2; count++)
    {
        int d;

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
#endif

            if (    (i_offset >= 0)
                 && (j_offset >= 0)
                 && (i_offset < fs_cached_num_rows)
                 && (j_offset < fs_cached_num_cols)
                 && ( ! fs_cached_seg_map[ i_offset ][ j_offset ])
                 /*
                 // Not needed because we already set the invalid pixels to
                 // ERROR.
                 //
                 && (fs_cached_ip->pixels[ i_offset ][ j_offset ].extra.invalid.pixel == VALID_PIXEL)
                 */
               )
            {
                int skip = FALSE;
                Pixel* pix_ptr = &(fs_cached_ip->pixels[ i_offset ][ j_offset ]);
                float R = pix_ptr->r;
                float G = pix_ptr->g;
                float B = pix_ptr->b;
                float sum_RGB = R + G + B + RGB_SUM_EPSILON;
                float r_chrom = R / sum_RGB;
                float g_chrom = G / sum_RGB;

                if (    (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                     &&
                        (    (r_chrom > fs_cached_r_chrom_max)
                          || (r_chrom < fs_cached_r_chrom_min)
                          || (g_chrom > fs_cached_g_chrom_max)
                          || (g_chrom < fs_cached_g_chrom_min)
                        )
                   )
                {
                    skip = TRUE;
                }
                else if (sum_RGB < fs_cached_sum_RGB_min)
                {
                    skip = TRUE;
                }
                else if (sum_RGB > fs_cached_sum_RGB_max)
                {
                    skip = TRUE;
                }

                if ( ! skip )
                {
                    grow_segment_using_rg_and_lum_var_4(i_offset, j_offset);
                }
            }
#ifndef GROW_RANDOMLY
        }
#endif
    }
#ifdef GROW_RANDOMLY
    call_count++;
    if (call_count == 24) call_count = 0;
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_segment_using_rg_and_lum_var_8(int i, int j)
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

    cur_image_pixel = fs_cached_ip->pixels[ i ][ j ];

    ASSERT(! (fs_cached_seg_map[ i ][ j ]));
    ASSERT(cur_image_pixel.extra.invalid.pixel == VALID_PIXEL);

    fs_cached_pixels[ fs_num_cached_pixels ].i = i;
    fs_cached_pixels[ fs_num_cached_pixels ].j = j;

    cur_R = cur_image_pixel.r;
    cur_G = cur_image_pixel.g;
    cur_B = cur_image_pixel.b;
    cur_sum_RGB = cur_R + cur_G + cur_B + RGB_SUM_EPSILON;
    cur_r_chrom = cur_R / cur_sum_RGB;
    cur_g_chrom = cur_G / cur_sum_RGB;

    fs_cached_R_sum += cur_R;
    fs_cached_G_sum += cur_G;
    fs_cached_B_sum += cur_B;
    fs_cached_r_chrom_sum += cur_r_chrom;
    fs_cached_g_chrom_sum += cur_g_chrom;
#ifdef MAINTAIN_SS
    fs_cached_r_chrom_sqr_sum += (cur_r_chrom * cur_r_chrom);
    fs_cached_g_chrom_sqr_sum += (cur_g_chrom * cur_g_chrom);
    fs_cached_sum_RGB_sqr_sum += (cur_sum_RGB * cur_sum_RGB);
#endif

    fs_num_cached_pixels++;

    fs_cached_seg_map[ i ][ j ] = fs_cached_segment_count;

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
                     && (i_offset < fs_cached_num_rows)
                     && (j_offset < fs_cached_num_cols)
                     && ( ! fs_cached_seg_map[ i_offset ][ j_offset ])
                     && (fs_cached_ip->pixels[ i_offset ][ j_offset ].extra.invalid.pixel == VALID_PIXEL)
                   )
                {
                    int skip = FALSE;
                    float R = fs_cached_ip->pixels[ i_offset ][ j_offset ].r;
                    float G = fs_cached_ip->pixels[ i_offset ][ j_offset ].g;
                    float B = fs_cached_ip->pixels[ i_offset ][ j_offset ].b;
                    float sum_RGB = R + G + B + RGB_SUM_EPSILON;
                    float r_chrom = R / sum_RGB;
                    float g_chrom = G / sum_RGB;

                    if (    (r_chrom < fs_cached_r_chrom_min)
                         && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                       )
                    {
                        skip = TRUE;
                    }
                    else if (    (r_chrom > fs_cached_r_chrom_max)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (g_chrom < fs_cached_g_chrom_min)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (    (g_chrom > fs_cached_g_chrom_max)
                              && (sum_RGB > fs_min_sum_RGB_for_chrom_test)
                            )
                    {
                        skip = TRUE;
                    }
                    else if (sum_RGB < fs_cached_sum_RGB_min)
                    {
                        skip = TRUE;
                    }
                    else if (sum_RGB > fs_cached_sum_RGB_max)
                    {
                        skip = TRUE;
                    }

                    if ( ! skip )
                    {
                        grow_segment_using_rg_and_lum_var_8(i_offset, j_offset);
                    }
                }
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
// Currently, we only use this for small segments. If we use it for large
// segments, then we would want to make seg_num global.
*/
static void remark_segment(int i, int j, int seg_num)
{
    int cur_seg = fs_cached_seg_map[ i ][ j ];
    int di, dj;


    fs_cached_seg_map[ i ][ j ] = seg_num;

#ifdef TEST
    fs_count_remark++;
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
                     && (i_offset < fs_cached_num_rows)
                     && (j_offset < fs_cached_num_cols)
#ifdef CONNECT_CORNER_OPTION
                     && (fs_seg_connect_corners || ((di == 0) || (dj == 0)))
#else
#ifndef DEFAULT_IS_TO_CONNECT_CORNERS
                     && ((di == 0) || (dj == 0))
#endif
#endif
                     && (fs_cached_seg_map[ i_offset ][ j_offset ] == cur_seg)
                   )
                {
                    remark_segment(i_offset, j_offset, seg_num);
                }
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_good_segment_number(int seg_num)
{
    ERE(seg_num = chase_initial_segment_numbers(seg_num));

    return fs_cached_good_segment_numbers[ seg_num];
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int chase_initial_segment_numbers(int seg_num)
{
    int count = 0;
    int num;


    seg_num = MAX_OF(seg_num, 0);

    while ((num = fs_cached_initial_segment_numbers[ seg_num ]) != seg_num)
    {
        seg_num = num;

        count++;

        if (count > 10000)
        {
            set_bug("Likely infinite loop in chase_initial_segment_numbers.");
            return ERROR;
        }
    }

    return num;
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
    verbose_pso(2, "Zeroing seg map with %d error pixels.\n", error_count);
#endif


}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int initialize_caches
(
    int num_rows,
    int num_cols,
    int num_segments
)
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

        kjb_free(fs_cached_pixels);
        NRE(fs_cached_pixels = N_TYPE_MALLOC(Cached_pixel, 4 * num_pixels));
    }

    if (num_segments > 0)
    {
        max_num_segments = num_segments;
    }
    else
    {
        max_num_segments = 1 + num_pixels / fs_seg_min_initial_segment_size;
    }

    /*
    // A few of the structures need one more slot.
    */
    max_num_segments++;

    if (max_num_segments > prev_max_num_segments)
    {
        verbose_pso(2, "Bumping segment oriented caches from %,d to %,d.\n",
                    prev_max_num_segments, max_num_segments);

        kjb_free(fs_cached_pixel_counts);
        kjb_free(fs_cached_initial_segment_numbers);
        kjb_free(fs_cached_good_segment_numbers);
        kjb_free(fs_cached_R_means);
        kjb_free(fs_cached_G_means);
        kjb_free(fs_cached_B_means);
        kjb_free(fs_cached_r_chrom_means);
        kjb_free(fs_cached_g_chrom_means);
        kjb_free(fs_cached_sum_RGB_means);
#ifdef MAINTAIN_SS
        kjb_free(fs_cached_r_chrom_SS);
        kjb_free(fs_cached_g_chrom_SS);
        kjb_free(fs_cached_sum_RGB_SS);
#endif

        NRE(fs_cached_pixel_counts = INT_MALLOC(max_num_segments));
        NRE(fs_cached_initial_segment_numbers = INT_MALLOC(1 + max_num_segments));
        NRE(fs_cached_good_segment_numbers = INT_MALLOC(1 + max_num_segments));
        NRE(fs_cached_R_means = FLT_MALLOC(max_num_segments));
        NRE(fs_cached_G_means = FLT_MALLOC(max_num_segments));
        NRE(fs_cached_B_means = FLT_MALLOC(max_num_segments));
        NRE(fs_cached_r_chrom_means = FLT_MALLOC(max_num_segments));
        NRE(fs_cached_g_chrom_means = FLT_MALLOC(max_num_segments));
        NRE(fs_cached_sum_RGB_means = FLT_MALLOC(max_num_segments));
#ifdef MAINTAIN_SS
        NRE(fs_cached_r_chrom_SS = FLT_MALLOC(max_num_segments));
        NRE(fs_cached_g_chrom_SS = FLT_MALLOC(max_num_segments));
        NRE(fs_cached_sum_RGB_SS = FLT_MALLOC(max_num_segments));
#endif

    }

    prev_max_num_segments = max_num_segments;
    prev_num_rows = num_rows;
    prev_num_cols = num_cols;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int update_segment_pair_storage
(
    int         num_segments,
    const char* mess_str
)
{
    /*
    // This is a lot of memory if num_segments is more than a few thousand.
    // These arrays are very sparse; likely they should be stored differently.
    */
    if (num_segments > fs_max_num_segments)
    {
        verbose_pso(10, "Bumping up storage for %s.\n", mess_str);

        free_2D_int_array(fs_properly_connected_pairs);
        free_2D_int_array(fs_boundary_pair_counts);
        free_2D_int_array(fs_data_pair_counts);

        NRE(fs_data_pair_counts = allocate_2D_int_array(num_segments,
                                                        num_segments));

        NRE(fs_properly_connected_pairs = allocate_2D_int_array(num_segments,
                                                                num_segments));

        NRE(fs_boundary_pair_counts = allocate_2D_int_array(num_segments,
                                                            num_segments));

        fs_max_num_segments = num_segments;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int update_segment_merge_storage
(
    int         num_segments,
    const char* mess_str
)
{

    /*
    // This uses a lot of memory if num_segments is more than a few thousand.
    // These arrays are very sparse; likely they should be stored differently.
    */

    if (num_segments > fs_max_num_segments)
    {
        verbose_pso(10, "Bumping up storage for %s.\n", mess_str);

        free_2D_int_array(fs_merge_candidates);

        NRE(fs_merge_candidates = allocate_2D_int_array(num_segments,
                                                        num_segments));

        ERE(get_target_matrix(&fs_merge_r_sum_mp,
                              num_segments, num_segments));

        ERE(get_target_matrix(&fs_merge_g_sum_mp,
                              num_segments, num_segments));

        ERE(get_target_matrix(&fs_merge_sum_RGM_sum_mp,
                              num_segments, num_segments));

        fs_max_num_segments = num_segments;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_copy_segmentation
(
    Segmentation_t3**      segmentation_ptr_ptr,
    const Segmentation_t3* segmentation_ptr
)
{
    int num_rows = segmentation_ptr->num_rows;
    int num_cols = segmentation_ptr->num_cols;
    int i, j;
    int num_segments = segmentation_ptr->num_segments;
    int** source_seg_map = segmentation_ptr->seg_map;
    int** target_seg_map;
    Segment_t3** source_segments = segmentation_ptr->segments;
    Segment_t3** target_segments;


    /* FIX -- leaks memory on failure. */

    ERE(get_target_segmentation(segmentation_ptr_ptr, num_rows, num_cols));

    (*segmentation_ptr_ptr)->num_segments = num_segments;
    (*segmentation_ptr_ptr)->negative_num_segments =
                                       segmentation_ptr->negative_num_segments;

    target_seg_map = (*segmentation_ptr_ptr)->seg_map;

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            target_seg_map[ i ][ j ] = source_seg_map[ i ][ j ];
        }
    }

    NRE(target_segments = N_TYPE_MALLOC(Segment_t3*, num_segments));
    (*segmentation_ptr_ptr)->segments = target_segments;

    for (i = 0; i < num_segments; i++)
    {
        target_segments[ i ] = NULL;
        ERE(copy_segment(&(target_segments[ i ]), source_segments[ i ]));
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_target_segmentation
(
    Segmentation_t3** segmentation_ptr_ptr,
    int            num_rows,
    int            num_cols
)
{
    Segmentation_t3* segmentation_ptr = *segmentation_ptr_ptr;


    if (segmentation_ptr == NULL)
    {
        NRE(segmentation_ptr = create_segmentation());
        *segmentation_ptr_ptr = segmentation_ptr;
    }
    else
    {
        int i;
        int num_segments = segmentation_ptr->num_segments;

        for (i=0; i<num_segments; i++)
        {
            free_segment(segmentation_ptr->segments[ i ]);
        }

        kjb_free(segmentation_ptr->segments);
        segmentation_ptr->segments = NULL;

        if (    (segmentation_ptr->num_rows != num_rows)
             || (segmentation_ptr->num_cols != num_cols)
           )
        {
            verbose_pso(5,
                      "Changing segmentation size from (%d, %d) to (%d, %d).\n",
                        segmentation_ptr->num_rows,
                        segmentation_ptr->num_cols,
                        num_rows, num_cols);

            free_2D_int_array(segmentation_ptr->seg_map);
            segmentation_ptr->seg_map = NULL;
        }
    }

    if (segmentation_ptr->seg_map == NULL)
    {
        segmentation_ptr->seg_map = allocate_2D_int_array(num_rows, num_cols);

        if (segmentation_ptr->seg_map == NULL)
        {
            kjb_free(segmentation_ptr);
            return ERROR;
        }
    }

    segmentation_ptr->num_rows = num_rows;
    segmentation_ptr->num_cols = num_cols;
    segmentation_ptr->num_segments = 0;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Segmentation_t3* create_segmentation(void)
{
    Segmentation_t3* segmentation_ptr;

    NRN(segmentation_ptr = TYPE_MALLOC(Segmentation_t3));

    segmentation_ptr->segments = NULL;
    segmentation_ptr->seg_map = NULL;

    return segmentation_ptr;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void t3_free_segmentation(Segmentation_t3* segmentation_ptr)
{

    if (segmentation_ptr != NULL)
    {
        int num_segments = segmentation_ptr->num_segments;
        int i;

        for (i=0; i<num_segments; i++)
        {
            free_segment(segmentation_ptr->segments[ i ]);
        }

        kjb_free(segmentation_ptr->segments);
        free_2D_int_array(segmentation_ptr->seg_map);
        kjb_free(segmentation_ptr);
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int copy_segment
(
    Segment_t3**      target_segment_ptr_ptr,
    const Segment_t3* source_segment_ptr
)
{
    Segment_t3* target_segment_ptr;
    int      num_pixels          = source_segment_ptr->num_pixels;
    int      num_boundary_pixels = source_segment_ptr->num_boundary_pixels;
    int      num_neighbours      = source_segment_ptr->num_neighbours;
    int      i;

    if (*target_segment_ptr_ptr != NULL)
    {
        free_segment(*target_segment_ptr_ptr);
    }

    NRE(target_segment_ptr = create_segment(source_segment_ptr->segment_number,
                                            num_pixels));
    *target_segment_ptr_ptr = target_segment_ptr;

    target_segment_ptr->num_boundary_pixels = num_boundary_pixels;
    target_segment_ptr->num_neighbours = num_neighbours;

    target_segment_ptr->R_ave = source_segment_ptr->R_ave;
    target_segment_ptr->G_ave = source_segment_ptr->G_ave;
    target_segment_ptr->B_ave = source_segment_ptr->B_ave;
    target_segment_ptr->r_chrom_ave = source_segment_ptr->r_chrom_ave;
    target_segment_ptr->g_chrom_ave = source_segment_ptr->g_chrom_ave;
    target_segment_ptr->sum_RGB_ave = source_segment_ptr->sum_RGB_ave;
    target_segment_ptr->i_CM = source_segment_ptr->i_CM;
    target_segment_ptr->j_CM = source_segment_ptr->j_CM;
    target_segment_ptr->i_inside = source_segment_ptr->i_inside;
    target_segment_ptr->j_inside = source_segment_ptr->j_inside;
    target_segment_ptr->first_moment = source_segment_ptr->first_moment;
    target_segment_ptr->second_moment = source_segment_ptr->second_moment;
    target_segment_ptr->i_min = source_segment_ptr->i_min;
    target_segment_ptr->i_max = source_segment_ptr->i_max;
    target_segment_ptr->j_min = source_segment_ptr->j_min;
    target_segment_ptr->j_max = source_segment_ptr->j_max;
    target_segment_ptr->boundary_len = source_segment_ptr->boundary_len;

    target_segment_ptr->outside_boundary_len =
                                       source_segment_ptr->outside_boundary_len;
    for (i = 0; i < num_pixels; i++)
    {
        target_segment_ptr->pixels[ i ] = source_segment_ptr->pixels[ i ];
    }

    if (num_boundary_pixels > 0)
    {
        NRE(target_segment_ptr->boundary_pixels = N_TYPE_MALLOC(Pixel_info,
                                                          num_boundary_pixels));

        for (i = 0; i < num_boundary_pixels; i++)
        {
            target_segment_ptr->boundary_pixels[ i ] =
                                       source_segment_ptr->boundary_pixels[ i ];
        }
    }

    ERE(copy_matrix(&(target_segment_ptr->outside_boundary_mp),
                    source_segment_ptr->outside_boundary_mp));

    NRE(target_segment_ptr->neighbours = INT_MALLOC(num_neighbours));
    NRE(target_segment_ptr->connection_counts = INT_MALLOC(num_neighbours));
    NRE(target_segment_ptr->nb_edge_r_diffs = DBL_MALLOC(num_neighbours));
    NRE(target_segment_ptr->nb_edge_g_diffs = DBL_MALLOC(num_neighbours));
    NRE(target_segment_ptr->nb_edge_rel_sum_diffs = DBL_MALLOC(num_neighbours));

    for (i = 0; i < num_neighbours; i++)
    {
        target_segment_ptr->neighbours[ i ] =
                                    source_segment_ptr->neighbours[ i ];
        target_segment_ptr->connection_counts[ i ] =
                                    source_segment_ptr->connection_counts[ i ];
#ifdef COMPUTE_EDGE_STRENGTHS
        target_segment_ptr->nb_edge_r_diffs[ i ] =
                                    source_segment_ptr->nb_edge_r_diffs[ i ];
        target_segment_ptr->nb_edge_g_diffs[ i ] =
                                    source_segment_ptr->nb_edge_g_diffs[ i ];
        target_segment_ptr->nb_edge_rel_sum_diffs[ i ] =
                                source_segment_ptr->nb_edge_rel_sum_diffs[ i ];
#endif
    }

    target_segment_ptr->pixel_count = source_segment_ptr->pixel_count;
    target_segment_ptr->boundary_pixel_count =
                                      source_segment_ptr->boundary_pixel_count;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static Segment_t3* create_segment(int segment_number, int num_pixels)
{
    Segment_t3*     seg_ptr;


    NRN(seg_ptr = TYPE_MALLOC(Segment_t3));

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
    seg_ptr->nb_edge_r_diffs = NULL;
    seg_ptr->nb_edge_g_diffs = NULL;
    seg_ptr->nb_edge_rel_sum_diffs = NULL;
    seg_ptr->connection_counts = NULL;

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

static void free_segment(Segment_t3* segment_ptr)
{

    if (segment_ptr != NULL)
    {
        kjb_free(segment_ptr->pixels);
        kjb_free(segment_ptr->boundary_pixels);
        free_matrix(segment_ptr->outside_boundary_mp);
        kjb_free(segment_ptr->neighbours);
        kjb_free(segment_ptr->connection_counts);
        kjb_free(segment_ptr->nb_edge_r_diffs);
        kjb_free(segment_ptr->nb_edge_g_diffs);
        kjb_free(segment_ptr->nb_edge_rel_sum_diffs);
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
    kjb_free(fs_cached_neighbours);

    free_2D_int_array(fs_data_pair_counts);
    free_2D_int_array(fs_boundary_pair_counts);
    free_2D_int_array(fs_properly_connected_pairs);
    free_2D_int_array(fs_merge_candidates);
    free_matrix(fs_merge_r_sum_mp);
    free_matrix(fs_merge_g_sum_mp);
    free_matrix(fs_merge_sum_RGM_sum_mp);

    kjb_free(fs_cached_pixels);
    kjb_free(fs_cached_pixel_counts);
    kjb_free(fs_cached_initial_segment_numbers);
    kjb_free(fs_cached_good_segment_numbers);
    kjb_free(fs_cached_R_means);
    kjb_free(fs_cached_G_means);
    kjb_free(fs_cached_B_means);
    kjb_free(fs_cached_r_chrom_means);
    kjb_free(fs_cached_g_chrom_means);
    kjb_free(fs_cached_sum_RGB_means);
#ifdef MAINTAIN_SS
    kjb_free(fs_cached_r_chrom_SS);
    kjb_free(fs_cached_g_chrom_SS);
    kjb_free(fs_cached_sum_RGB_SS);
#endif

    free_int_matrix(fs_cached_magnified_boundary_points);
    free_int_matrix(fs_cached_magnified_seg_map);
    kjb_free_image(fs_cached_ip);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef OBSOLETE
int t3_image_draw_segmentation
(
    const KJB_image* ip,
    Segmentation_t3*    segmentation_ptr,
    KJB_image**      out_ip_list
)
{
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int                 result                 = NO_ERROR;
    KJB_image*        out_ip;
    int                 i;
    int                 j;
    Pixel         initial_pixel;
    int                 out_count;


    if (out_ip_list == NULL) return NO_ERROR;

    EPETE(kjb_copy_image(&(out_ip_list[ 0 ]), ip));

    initial_pixel.r = 0;
    initial_pixel.g = 0;
    initial_pixel.b = 0;
    initial_pixel.extra.invalid.pixel = INVALID_PIXEL;

    /*
    // First picture is just the original. Second picture (first result
    // picture, out_count==0) uses the average region colour. Third encodes
    // segment information.
    */
    for (out_count=0; out_count<3; out_count++)
    {
        EPETE(get_initialized_image(&(out_ip_list[ out_count + 1]),
                                          num_rows, num_cols, &initial_pixel));
        out_ip = out_ip_list[ out_count + 1];


        /* Regions */
        for (i=0; i<segmentation_ptr->num_segments; i++)
        {
            Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];

            for (j=0; j<seg_ptr->num_pixels; j++)
            {
                Pixel_info pixel_info;
                float R, G, B;

                pixel_info = seg_ptr->pixels[ j ];

                if ((out_count == 0) || (out_count == 1))
                {
                    R = seg_ptr->R_ave;
                    G = seg_ptr->G_ave;
                    B = seg_ptr->B_ave;
                }
                else if (out_count == 2)
                {
                    if (segmentation_ptr->num_segments > 255)
                    {
                        R = (i + 1)/100;
                        G = (i + 1)%100;
                    }
                    else
                    {
                        R = seg_ptr->R_ave;

                        G = i + 1;

                        if (segmentation_ptr->num_segments <= 25)
                        {
                            G *= FLT_TEN;
                        }
                    }

                    B = MIN_OF(100, seg_ptr->num_pixels);
                }
                else
                {
                    R = ip->pixels[ pixel_info.i ][ pixel_info.j ].r;
                    G = ip->pixels[ pixel_info.i ][ pixel_info.j ].g;
                    B = ip->pixels[ pixel_info.i ][ pixel_info.j ].b;
                }

                out_ip->pixels[ pixel_info.i ][ pixel_info.j ].r = R;
                out_ip->pixels[ pixel_info.i ][ pixel_info.j ].g = G;
                out_ip->pixels[ pixel_info.i ][ pixel_info.j ].b = B;
            }
        }

        if (out_count == 2)
        {
            /*
            // Mark invalid pixels and tiny regions pixels as purple with green
            // being the negative of the map number. Mark regions which are not
            // quite big enough as grey.
            */
            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    int seg_num = segmentation_ptr->seg_map[ i ][ j ];

                    if (seg_num == ERROR)
                    {
                        out_ip->pixels[ i ][ j ].r = 255;
                        out_ip->pixels[ i ][ j ].g = 255;
                        out_ip->pixels[ i ][ j ].b = 255;
                    }
                    else if (seg_num < 0)
                    {
                        out_ip->pixels[ i ][ j ].r = 0;
                        out_ip->pixels[ i ][ j ].g = 0;
                        out_ip->pixels[ i ][ j ].b = -seg_num;
                    }
                }
            }
        }

#ifdef DEF_OUT
        if (out_count == 0)
        {
            /* Outside Boundaries */
            for (i=0; i<segmentation_ptr->num_segments; i++)
            {
                Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];
                Matrix* outside_boundary_mp = seg_ptr->outside_boundary_mp;
                int     count;

                if (outside_boundary_mp == NULL)
                {
                    count = 0;
                }
                else
                {
                    count = outside_boundary_mp->num_rows;
                }

                for (j=0; j<count; j++)
                {
                    float grey = 120 + (j * 128) / count;
                    double x = outside_boundary_mp->elements[ j ][ 0 ];
                    double y = outside_boundary_mp->elements[ j ][ 1 ];
                    int b_i = MAX_OF(0, MIN_OF(num_rows - 1, (int)x));
                    int b_j = MAX_OF(0, MIN_OF(num_cols - 1, (int)y));

                    out_ip->pixels[ b_i ][ b_j ].r = grey;
                    out_ip->pixels[ b_i ][ b_j ].g = grey;
                    out_ip->pixels[ b_i ][ b_j ].b = grey;
                    out_ip->pixels[ b_i ][ b_j ].extra.invalid.pixel = VALID_PIXEL;
                }
            }
        }
        else
#endif
        if ((out_count == 1) || (out_count == 2))
        {
            /* Boundaries */
            for (i=0; i<segmentation_ptr->num_segments; i++)
            {
                Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];

                image_draw_pixels(out_ip, seg_ptr->num_boundary_pixels,
                                  seg_ptr->boundary_pixels,
                                  (int)(255.0 - seg_ptr->R_ave + 0.5),
                                  (int)(255.0 - seg_ptr->G_ave + 0.5),
                                  (int)(255.0 - seg_ptr->B_ave + 0.5));
            }
        }

        if ((out_count == 1) || (out_count == 2))
        {
            /* Draw connectors of interior points */

            for (i=0; i<segmentation_ptr->num_segments; i++)
            {
                Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];
                int            i_inside;
                int            nb_i_inside;
                int            j_inside;
                int            nb_j_inside;

                i_inside = seg_ptr->i_inside;
                j_inside = seg_ptr->j_inside;

                for (j=0; j<seg_ptr->num_neighbours; j++)
                {
                    int neighbour = seg_ptr->neighbours[ j ];
                    Segment_t3* nb_seg_ptr = segmentation_ptr->segments[ neighbour - 1];
                    int R, G, B;

                    nb_i_inside = nb_seg_ptr->i_inside;
                    nb_j_inside = nb_seg_ptr->j_inside;

                    R = 255 - (seg_ptr->R_ave + nb_seg_ptr->R_ave) / 2;
                    G = 255 - (seg_ptr->G_ave + nb_seg_ptr->G_ave) / 2;
                    B = 255 - (seg_ptr->B_ave + nb_seg_ptr->B_ave) / 2;

                    image_draw_segment_2(out_ip, i_inside, j_inside,
                                         nb_i_inside, nb_j_inside, 1, R, G, B);
                }
            }

            /* Interior points of segments */
            for (i=0; i<segmentation_ptr->num_segments; i++)
            {
                Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];
                int R, G, B;

                image_draw_point(out_ip, seg_ptr->i_inside, seg_ptr->j_inside,
                                 1, 0, 200, 0);

#ifdef NOT_USED
                if (out_count == 0)
                {
                    R = seg_ptr->R_ave;
                    G = seg_ptr->G_ave;
                    B = seg_ptr->B_ave;
                }
                else
                {
#endif
                    if (segmentation_ptr->num_segments > 255)
                    {
                        R = (i + 1)/100;
                        G = (i + 1)%100;
                    }
                    else
                    {
                        R = seg_ptr->R_ave;

                        G = i + 1;

                        if (segmentation_ptr->num_segments <= 25)
                        {
                            G *= FLT_TEN;
                        }
                    }
                    B = MIN_OF(100, seg_ptr->num_pixels);
#ifdef NOT_USED
                }
#endif

                image_draw_point(out_ip, seg_ptr->i_inside, seg_ptr->j_inside,0,
                                 R, G, B);
            }
        }
#ifdef DONT_DO_IT
        else if (out_count == 2)
        {
            /* Draw box arround segment. */
            for (i=0; i<segmentation_ptr->num_segments; i++)
            {
                Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];

                image_draw_segment_2(out_ip,
                                     seg_ptr->i_min, seg_ptr->j_min,
                                     seg_ptr->i_min, seg_ptr->j_max,
                                     1,
                                     255 - (int)seg_ptr->R_ave,
                                     255 - (int)seg_ptr->G_ave,
                                     255 - (int)seg_ptr->B_ave);
                image_draw_segment_2(out_ip,
                                     seg_ptr->i_min, seg_ptr->j_max,
                                     seg_ptr->i_max, seg_ptr->j_max,
                                     1,
                                     255 - (int)seg_ptr->R_ave,
                                     255 - (int)seg_ptr->G_ave,
                                     255 - (int)seg_ptr->B_ave);
                image_draw_segment_2(out_ip,
                                     seg_ptr->i_max, seg_ptr->j_max,
                                     seg_ptr->i_max, seg_ptr->j_min,
                                     1,
                                     255 - (int)seg_ptr->R_ave,
                                     255 - (int)seg_ptr->G_ave,
                                     255 - (int)seg_ptr->B_ave);
                image_draw_segment_2(out_ip,
                                     seg_ptr->i_max, seg_ptr->j_min,
                                     seg_ptr->i_min, seg_ptr->j_min,
                                     1,
                                     255 - (int)seg_ptr->R_ave,
                                     255 - (int)seg_ptr->G_ave,
                                     255 - (int)seg_ptr->B_ave);
            }

            /* Connect interior point to top left corner */
            for (i=0; i<segmentation_ptr->num_segments; i++)
            {
                Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];

                image_draw_segment_2(out_ip,
                                     seg_ptr->i_inside, seg_ptr->j_inside,
                                     seg_ptr->i_min, seg_ptr->j_min,
                                     1, 150, 150, 0);
            }

            /* Connect CM to bottem left corner */
            for (i=0; i<segmentation_ptr->num_segments; i++)
            {
                Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];

                image_draw_segment_2(out_ip,
                                     (int)seg_ptr->i_CM, (int)seg_ptr->j_CM,
                                     seg_ptr->i_max, seg_ptr->j_min,
                                     1, 200, 100, 0);
            }

            /* Interior points of segments */
            for (i=0; i<segmentation_ptr->num_segments; i++)
            {
                Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];

                image_draw_point(out_ip, seg_ptr->i_inside, seg_ptr->j_inside,
                                 1, 0, 200, 0);
            }

            /* Segment_t3 centers of masses */
            for (i=0; i<segmentation_ptr->num_segments; i++)
            {
                Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];

                image_draw_point(out_ip, (int)seg_ptr->i_CM, (int)seg_ptr->j_CM,
                                 1, 200, 0, 200);
            }
        }
#endif

    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_image_draw_segments_and_outside_boundaries(const KJB_image* ip, Segmentation_t3* segmentation_ptr,
                                               KJB_image**  out_ipp)
{
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int                 result                 = NO_ERROR;
    KJB_image*        out_ip;
    int                 i;
    int                 j;
    Pixel         initial_pixel;


    initial_pixel.r = 0;
    initial_pixel.g = 0;
    initial_pixel.b = 0;
    initial_pixel.extra.invalid.pixel = INVALID_PIXEL;

    EPETE(get_initialized_image(out_ipp,
                                      num_rows, num_cols, &initial_pixel));
    out_ip = *out_ipp;

    /* Regions */
    for (i=0; i<segmentation_ptr->num_segments; i++)
    {
        Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];
        float R, G, B;

        R = seg_ptr->R_ave;
        G = seg_ptr->G_ave;
        B = seg_ptr->B_ave;

        for (j=0; j<seg_ptr->num_pixels; j++)
        {
            Pixel_info pixel_info;

            pixel_info = seg_ptr->pixels[ j ];

            out_ip->pixels[ pixel_info.i ][ pixel_info.j ].r = R;
            out_ip->pixels[ pixel_info.i ][ pixel_info.j ].g = G;
            out_ip->pixels[ pixel_info.i ][ pixel_info.j ].b = B;
        }
    }

    /* Outside Boundaries */
    for (i=0; i<segmentation_ptr->num_segments; i++)
    {
        Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];
        Matrix* outside_boundary_mp = seg_ptr->outside_boundary_mp;
        int     count;

        if (outside_boundary_mp == NULL)
        {
            count = 0;
        }
        else
        {
            count = outside_boundary_mp->num_rows;
        }

        for (j=0; j<count; j++)
        {
            float grey = 120 + (j * 128) / count;
            double x = outside_boundary_mp->elements[ j ][ 0 ];
            double y = outside_boundary_mp->elements[ j ][ 1 ];
            int b_i = MAX_OF(0, MIN_OF(num_rows - 1, (int)x));
            int b_j = MAX_OF(0, MIN_OF(num_cols - 1, (int)y));

            out_ip->pixels[ b_i ][ b_j ].r = grey;
            out_ip->pixels[ b_i ][ b_j ].g = grey;
            out_ip->pixels[ b_i ][ b_j ].b = MIN_OF(i,255);
            out_ip->pixels[ b_i ][ b_j ].extra.invalid.pixel = VALID_PIXEL;
        }
    }

    return result;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_image_draw_image_segment_outside_boundaries
(
    KJB_image*          ip,
    const Segmentation_t3* segmentation_ptr
)
{
    int          result   = NO_ERROR;
    int          num_rows = ip->num_rows;
    int          num_cols = ip->num_cols;
    int          i;
    int          j;


    for (i=0; i<segmentation_ptr->num_segments; i++)
    {
        Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];
        Matrix* outside_boundary_mp = seg_ptr->outside_boundary_mp;
        int     count;

        if (outside_boundary_mp == NULL)
        {
            count = 0;
        }
        else
        {
            count = outside_boundary_mp->num_rows;
        }

        for (j=0; j<count; j++)
        {
            float grey = 120 + (j * 128) / count;
            double x = outside_boundary_mp->elements[ j ][ 0 ];
            double y = outside_boundary_mp->elements[ j ][ 1 ];
            int b_i = MAX_OF(0, MIN_OF(num_rows - 1, (int)x));
            int b_j = MAX_OF(0, MIN_OF(num_cols - 1, (int)y));

            ip->pixels[ b_i ][ b_j ].r = grey;
            ip->pixels[ b_i ][ b_j ].g = grey;
            ip->pixels[ b_i ][ b_j ].b = MIN_OF(i,255);
            ip->pixels[ b_i ][ b_j ].extra.invalid.r = VALID_PIXEL;
            ip->pixels[ b_i ][ b_j ].extra.invalid.g = VALID_PIXEL;
            ip->pixels[ b_i ][ b_j ].extra.invalid.b = VALID_PIXEL;
            ip->pixels[ b_i ][ b_j ].extra.invalid.pixel = VALID_PIXEL;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_image_draw_image_segment_outside_boundaries_2
(
    KJB_image*          ip,
    const Segmentation_t3* segmentation_ptr
)
{
    int result   = NO_ERROR;
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int i;
    int j;


    for (i=0; i<segmentation_ptr->num_segments; i++)
    {
        Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];
        Matrix* outside_boundary_mp = seg_ptr->outside_boundary_mp;
        int     count;

        if (outside_boundary_mp == NULL)
        {
            count = 0;
        }
        else
        {
            count = outside_boundary_mp->num_rows;
        }

        for (j=0; j<count; j++)
        {
            double x = outside_boundary_mp->elements[ j ][ 0 ];
            double y = outside_boundary_mp->elements[ j ][ 1 ];
            int b_i = MAX_OF(0, MIN_OF(num_rows - 1, (int)x));
            int b_j = MAX_OF(0, MIN_OF(num_cols - 1, (int)y));

            ip->pixels[ b_i ][ b_j ].r = FLT_255;
            ip->pixels[ b_i ][ b_j ].g = FLT_255;
            ip->pixels[ b_i ][ b_j ].b = FLT_255;
            ip->pixels[ b_i ][ b_j ].extra.invalid.r = VALID_PIXEL;
            ip->pixels[ b_i ][ b_j ].extra.invalid.g = VALID_PIXEL;
            ip->pixels[ b_i ][ b_j ].extra.invalid.b = VALID_PIXEL;
            ip->pixels[ b_i ][ b_j ].extra.invalid.pixel = VALID_PIXEL;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_image_draw_image_segment_boundaries
(
    KJB_image*          ip,
    const Segmentation_t3* segmentation_ptr,
    int                 width
)
{
    int   result   = NO_ERROR;
    int   num_rows = ip->num_rows;
    int   num_cols = ip->num_cols;
    int   i, j;
    int** seg_map  = segmentation_ptr->seg_map;


    for (i=0; i<segmentation_ptr->num_segments; i++)
    {
        Segment_t3*    seg_ptr         = segmentation_ptr->segments[ i ];
        int         count           = seg_ptr->num_boundary_pixels;
        int         seg_num         = seg_ptr->segment_number;
        Pixel_info* boundary_pixels = seg_ptr->boundary_pixels;

        for (j=0; j<count; j++)
        {
            int b_i = boundary_pixels[ j ].i;
            int b_j = boundary_pixels[ j ].j;
            int ii, jj;
            int min_ii = MAX_OF(0, b_i - width);
            int max_ii = MIN_OF(b_i + width, num_rows - 1);
            int min_jj = MAX_OF(0, b_j - width);
            int max_jj = MIN_OF(b_j + width, num_cols - 1);

            for (ii = min_ii; ii <= max_ii; ii++)
            {
                for (jj = min_jj; jj <= max_jj; jj++)
                {
                    if (seg_map[ ii ][ jj ] == seg_num)
                    {
                        ip->pixels[ ii ][ jj ].r = FLT_255;
                        ip->pixels[ ii ][ jj ].g = FLT_255;
                        ip->pixels[ ii ][ jj ].b = FLT_255;
                        ip->pixels[ ii ][ jj ].extra.invalid.r = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.g = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.b = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.pixel = VALID_PIXEL;
                    }
                }
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_image_draw_image_segment_fancy_boundaries
(
    KJB_image*          ip,
    const Segmentation_t3* segmentation_ptr,
    int                 width
)
{
    int   result   = NO_ERROR;
    int   num_rows = ip->num_rows;
    int   num_cols = ip->num_cols;
    int   i, j;
    int** seg_map  = segmentation_ptr->seg_map;


    for (i=0; i<segmentation_ptr->num_segments; i++)
    {
        Segment_t3*    seg_ptr         = segmentation_ptr->segments[ i ];
        int         count           = seg_ptr->num_boundary_pixels;
        int         seg_num         = seg_ptr->segment_number;
        Pixel_info* boundary_pixels = seg_ptr->boundary_pixels;

        for (j=0; j<count; j++)
        {
            int b_i = boundary_pixels[ j ].i;
            int b_j = boundary_pixels[ j ].j;
            int ii, jj;
            int min_ii = MAX_OF(0, b_i - width);
            int max_ii = MIN_OF(b_i + width, num_rows - 1);
            int min_jj = MAX_OF(0, b_j - width);
            int max_jj = MIN_OF(b_j + width, num_cols - 1);

            for (ii = min_ii; ii <= max_ii; ii++)
            {
                for (jj = min_jj; jj <= max_jj; jj++)
                {
                    if (seg_map[ ii ][ jj ] == seg_num)
                    {
                        ip->pixels[ ii ][ jj ].r = FLT_255;
                        ip->pixels[ ii ][ jj ].g = FLT_255;
                        ip->pixels[ ii ][ jj ].b = FLT_255;
                        ip->pixels[ ii ][ jj ].extra.invalid.r = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.g = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.b = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.pixel = VALID_PIXEL;
                    }
                }
            }
        }
    }

    width = 0;

    for (i=0; i<segmentation_ptr->num_segments; i++)
    {
        Segment_t3*    seg_ptr         = segmentation_ptr->segments[ i ];
        int         count           = seg_ptr->num_boundary_pixels;
        int         seg_num         = seg_ptr->segment_number;
        Pixel_info* boundary_pixels = seg_ptr->boundary_pixels;

        for (j=0; j<count; j++)
        {
            int b_i = boundary_pixels[ j ].i;
            int b_j = boundary_pixels[ j ].j;
            int ii, jj;
            int min_ii = MAX_OF(0, b_i - width);
            int max_ii = MIN_OF(b_i + width, num_rows - 1);
            int min_jj = MAX_OF(0, b_j - width);
            int max_jj = MIN_OF(b_j + width, num_cols - 1);

            for (ii = min_ii; ii <= max_ii; ii++)
            {
                for (jj = min_jj; jj <= max_jj; jj++)
                {
                    if (seg_map[ ii ][ jj ] == seg_num)
                    {
                        ip->pixels[ ii ][ jj ].r = FLT_255;
                        ip->pixels[ ii ][ jj ].g = 0.0;
                        ip->pixels[ ii ][ jj ].b = 0.0;
                        ip->pixels[ ii ][ jj ].extra.invalid.r = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.g = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.b = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.pixel = VALID_PIXEL;
                    }
                }
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_image_draw_image_segment_boundaries_2
(
    KJB_image*          ip,
    const Segmentation_t3* segmentation_ptr,
    int                 width
)
{
    int   result   = NO_ERROR;
    int   num_rows = ip->num_rows;
    int   num_cols = ip->num_cols;
    int   i, j;
    int** seg_map  = segmentation_ptr->seg_map;


    for (i=0; i<segmentation_ptr->num_segments; i++)
    {
        Segment_t3*    seg_ptr         = segmentation_ptr->segments[ i ];
        int         count           = seg_ptr->num_boundary_pixels;
        float       R               = seg_ptr->R_ave;
        float       G               = seg_ptr->G_ave;
        float       B               = seg_ptr->B_ave;
        int         seg_num         = seg_ptr->segment_number;
        Pixel_info* boundary_pixels = seg_ptr->boundary_pixels;

        for (j=0; j<count; j++)
        {
            int b_i = boundary_pixels[ j ].i;
            int b_j = boundary_pixels[ j ].j;
            int ii, jj;
            int min_ii = MAX_OF(0, b_i - width);
            int max_ii = MIN_OF(b_i + width, num_rows - 1);
            int min_jj = MAX_OF(0, b_j - width);
            int max_jj = MIN_OF(b_j + width, num_cols - 1);

            for (ii = min_ii; ii <= max_ii; ii++)
            {
                for (jj = min_jj; jj <= max_jj; jj++)
                {
                    if (seg_map[ ii ][ jj ] == seg_num)
                    {
                        ip->pixels[ ii ][ jj ].r = R;
                        ip->pixels[ ii ][ jj ].g = G;
                        ip->pixels[ ii ][ jj ].b = B;
                        ip->pixels[ ii ][ jj ].extra.invalid.r = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.g = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.b = VALID_PIXEL;
                        ip->pixels[ ii ][ jj ].extra.invalid.pixel = VALID_PIXEL;
                    }
                }
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_image_draw_non_segments
(
    KJB_image*          ip,
    const Segmentation_t3* segmentation_ptr,
    int                 R,
    int                 G,
    int                 B
)
{
    int   result   = NO_ERROR;
    int   num_rows = ip->num_rows;
    int   num_cols = ip->num_cols;
    int   i, j;
    int** seg_map  = segmentation_ptr->seg_map;
    float f_R = R;
    float f_G = G;
    float f_B = B;
    float inv_f_R = 255 - R;
    float inv_f_G = 255 - G;
    float inv_f_B = 255 - B;


    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (seg_map[ i ][ j ] < 0)
            {
                ip->pixels[ i ][ j ].r = f_R;
                ip->pixels[ i ][ j ].g = f_G;
                ip->pixels[ i ][ j ].b = f_B;
                ip->pixels[ i ][ j ].extra.invalid.r = VALID_PIXEL;
                ip->pixels[ i ][ j ].extra.invalid.g = VALID_PIXEL;
                ip->pixels[ i ][ j ].extra.invalid.b = VALID_PIXEL;
                ip->pixels[ i ][ j ].extra.invalid.pixel = VALID_PIXEL;
            }
            else if (seg_map[ i ][ j ] == 0)
            {
                ip->pixels[ i ][ j ].r = inv_f_R;
                ip->pixels[ i ][ j ].g = inv_f_G;
                ip->pixels[ i ][ j ].b = inv_f_B;
                ip->pixels[ i ][ j ].extra.invalid.r = VALID_PIXEL;
                ip->pixels[ i ][ j ].extra.invalid.g = VALID_PIXEL;
                ip->pixels[ i ][ j ].extra.invalid.b = VALID_PIXEL;
                ip->pixels[ i ][ j ].extra.invalid.pixel = VALID_PIXEL;
            }
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_image_draw_image_segments
(
    KJB_image*          ip,
    const Segmentation_t3* segmentation_ptr
)
{
    int result   = NO_ERROR;
    int i, j;


    for (i=0; i<segmentation_ptr->num_segments; i++)
    {
        Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];
        float R, G, B;

        R = seg_ptr->R_ave;
        G = seg_ptr->G_ave;
        B = seg_ptr->B_ave;

        for (j=0; j<seg_ptr->num_pixels; j++)
        {
            Pixel_info pixel_info;

            pixel_info = seg_ptr->pixels[ j ];

            ip->pixels[ pixel_info.i ][ pixel_info.j ].r = R;
            ip->pixels[ pixel_info.i ][ pixel_info.j ].g = G;
            ip->pixels[ pixel_info.i ][ pixel_info.j ].b = B;
            ip->pixels[ pixel_info.i ][ pixel_info.j ].extra.invalid.r = VALID_PIXEL;
            ip->pixels[ pixel_info.i ][ pixel_info.j ].extra.invalid.g = VALID_PIXEL;
            ip->pixels[ pixel_info.i ][ pixel_info.j ].extra.invalid.b = VALID_PIXEL;
            ip->pixels[ pixel_info.i ][ pixel_info.j ].extra.invalid.pixel = VALID_PIXEL;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int t3_image_draw_image_segment_neighbours
(
    KJB_image*          ip,
    const Segmentation_t3* segmentation_ptr,
    int                 width,
    int                 R,
    int                 G,
    int                 B
)
{
    int result   = NO_ERROR;
    int i, j;


    for (i=0; i<segmentation_ptr->num_segments; i++)
    {
        Segment_t3* seg_ptr = segmentation_ptr->segments[ i ];

        for (j = 0; j < seg_ptr->num_neighbours; j++)
        {
            int nb_index = seg_ptr->neighbours[ j ] - 1;
            Segment_t3* nb_seg_ptr = segmentation_ptr->segments[ nb_index ];
            double r_strenth = seg_ptr->nb_edge_r_diffs[ j ];
            double g_strenth = seg_ptr->nb_edge_g_diffs[ j ];
            double rel_sum_strenth = seg_ptr->nb_edge_rel_sum_diffs[ j ];
            int r = (int)((double)R * 10.0 * r_strenth);
            int g = (int)((double)G * 10.0 * g_strenth);
            int b = (int)((double)B * 5.0 * rel_sum_strenth);

            image_draw_segment_2(ip,
                                 seg_ptr->i_inside, seg_ptr->j_inside,
                                 nb_seg_ptr->i_inside, nb_seg_ptr->j_inside,
                                 width, r, g, b);

            image_draw_point(ip,
                              nb_seg_ptr->i_inside, nb_seg_ptr->j_inside,
                              2 * width,
                              nb_index + 1, nb_index + 1, nb_index+ 1);
        }

        image_draw_point(ip,
                          seg_ptr->i_inside, seg_ptr->j_inside,
                          2 * width, i + 1, i + 1, i + 1);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
// This does not force the regions to have the same numbers. However, we are
// not gaurenteed connected regions as input, so using them as is is also
// dangerous.
*/
static int use_initial_segmentation
(
    const KJB_image* ip,
    int**            initial_map,
    int**            seg_map
)
{
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    int i, j;
    long initial_cpu_time      = get_cpu_time();
    int  min_initial_map_value = INT_MAX;
    int  max_initial_map_value = INT_MIN;


    zero_seg_map(ip, seg_map);

    ERE(kjb_copy_image(&fs_cached_ip, ip));

    /* Normally bad practice, but we want to improve performace! */
    fs_cached_seg_map = seg_map;
    fs_cached_num_rows = num_rows;
    fs_cached_num_cols = num_cols;
    fs_cached_initial_map = initial_map;

    fs_cached_segment_count = 0;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            max_initial_map_value = MAX_OF(max_initial_map_value,
                                           initial_map[ i ][ j ]);

            min_initial_map_value = MIN_OF(min_initial_map_value,
                                           initial_map[ i ][ j ]);

            if (    (seg_map[ i ][ j ] == 0)
                 && (fs_cached_ip->pixels[ i ][ j ].extra.invalid.pixel == VALID_PIXEL)
               )
            {
                fs_cached_segment_count++;
                use_initial_segmentation_helper(i, j);

                if (fs_num_cached_pixels < fs_seg_min_initial_segment_size)
                {
                    /*
                    // Segment_t3 is not big enough. Back off this segment,
                    // and set the map locations to the negative of the
                    // number of pixels.
                    */
#ifdef COLLECT_PIXELS_DURING_INITIAL_SEG
                    {
                        int ii;

                        for (ii = 0; ii < fs_num_cached_pixels; ii++)
                        {
                            int iii = fs_cached_pixels[ ii ].i;
                            int jjj = fs_cached_pixels[ ii ].j;

                            seg_map[ iii ][ jjj ] = -fs_num_cached_pixels;
                        }
                    }
#else
#ifdef TEST
                    fs_count_remark = 0;
#endif
#ifdef REMARK_SMALL_REGIONS_AS_ZERO
                    remark_segment(i, j, 0);
#else
                    remark_segment(i, j, -fs_num_cached_pixels);
#endif
#ifdef TEST
                    ASSERT(fs_count_remark == fs_num_cached_pixels);
#endif
#endif
                    fs_cached_segment_count--;
                }
                else
                {
                    int  num_pixels = fs_num_cached_pixels;
                    int  s = fs_cached_segment_count - 1;
                    double ave_sum_RGB;

                    fs_cached_pixel_counts[ s ] = num_pixels;

                    fs_cached_R_means[ s ] = fs_cached_R_sum / num_pixels;
                    fs_cached_G_means[ s ] = fs_cached_G_sum / num_pixels;
                    fs_cached_B_means[ s ] = fs_cached_B_sum / num_pixels;

                    fs_cached_r_chrom_means[ s ] = fs_cached_r_chrom_sum / num_pixels;
                    fs_cached_g_chrom_means[ s ] = fs_cached_g_chrom_sum / num_pixels;

                    ave_sum_RGB = fs_cached_R_sum + fs_cached_G_sum + fs_cached_B_sum;

                    fs_cached_sum_RGB_means[ s ] = ave_sum_RGB / num_pixels;
#ifdef MAINTAIN_SS
                    fs_cached_r_chrom_SS[ s ] = fs_cached_r_chrom_sqr_sum;
                    fs_cached_g_chrom_SS[ s ] = fs_cached_g_chrom_sqr_sum;
                    fs_cached_sum_RGB_SS[ s ] = fs_cached_sum_RGB_sqr_sum;
#endif
                }
            }
        }
    }

    dbi(min_initial_map_value);
    dbi(max_initial_map_value);

#ifdef TEST
    {
        int num_pixels_in_segments = 0;
        int num_unsegmented_pixels = 0;

        for (i=0; i < fs_cached_segment_count; i++)
        {
            num_pixels_in_segments += fs_cached_pixel_counts[ i ];
        }

        verbose_pso(10, "Image has %d pixels in initial segments.\n",
                    num_pixels_in_segments);

        for (i=0; i<num_rows; i++)
        {
            for (j=0; j<num_cols; j++)
            {
                /* ASSERT(fs_cached_seg_map[ i ][ j ] != 0); */

                if (fs_cached_seg_map[ i ][ j ] <= 0)
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
                fs_cached_segment_count, get_cpu_time() - initial_cpu_time);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void use_initial_segmentation_helper(int i, int j)
{
    float         R, G, B;
    float         sum_RGB;
    float         r_chrom;
    float         g_chrom;


    R = fs_cached_ip->pixels[ i ][ j ].r;
    G = fs_cached_ip->pixels[ i ][ j ].g;
    B = fs_cached_ip->pixels[ i ][ j ].b;
    sum_RGB = R + G + B + RGB_SUM_EPSILON;
    r_chrom = R / sum_RGB;
    g_chrom = G / sum_RGB;

    fs_num_cached_pixels = 0;

    fs_cached_r_chrom_sum     = 0.0;
    fs_cached_g_chrom_sum     = 0.0;
    fs_cached_R_sum           = 0.0;
    fs_cached_G_sum           = 0.0;
    fs_cached_B_sum           = 0.0;
#ifdef MAINTAIN_SS
    fs_cached_r_chrom_sqr_sum = 0.0;
    fs_cached_g_chrom_sqr_sum = 0.0;
    fs_cached_sum_RGB_sqr_sum = 0.0;
#endif

#ifdef USE_NO_VALUE
    if (    (fs_max_abs_RGB_var >= FLT_ZERO)
         && (fs_max_rel_RGB_var >= FLT_ZERO))
    {
        fs_cached_R_min = MAX_OF(R - fs_max_abs_RGB_var,
                              R*(1.0 - fs_max_rel_RGB_var));
        fs_cached_R_max = MIN_OF(R + fs_max_abs_RGB_var,
                              R*(1.0 + fs_max_rel_RGB_var));

        fs_cached_G_min = MAX_OF(G - fs_max_abs_RGB_var,
                              G*(1.0 - fs_max_rel_RGB_var));
        fs_cached_G_max = MIN_OF(G + fs_max_abs_RGB_var,
                              G*(1.0 + fs_max_rel_RGB_var));

        fs_cached_B_min = MAX_OF(B - fs_max_abs_RGB_var,
                              B*(1.0 - fs_max_rel_RGB_var));
        fs_cached_B_max = MIN_OF(B + fs_max_abs_RGB_var,
                              B*(1.0 + fs_max_rel_RGB_var));
    }
    else if (fs_max_abs_RGB_var >= FLT_ZERO)
    {
        fs_cached_R_min = R - fs_max_abs_RGB_var;
        fs_cached_R_max = R + fs_max_abs_RGB_var;
        fs_cached_G_min = G - fs_max_abs_RGB_var;
        fs_cached_G_max = G + fs_max_abs_RGB_var;
        fs_cached_B_min = B - fs_max_abs_RGB_var;
        fs_cached_B_max = B + fs_max_abs_RGB_var;
    }
    else if (fs_max_rel_RGB_var >= FLT_ZERO)
    {
        fs_cached_R_min = R*(1.0 - fs_max_rel_RGB_var);
        fs_cached_R_max = R*(1.0 + fs_max_rel_RGB_var);
        fs_cached_G_min = G*(1.0 - fs_max_rel_RGB_var);
        fs_cached_G_max = G*(1.0 + fs_max_rel_RGB_var);
        fs_cached_B_min = B*(1.0 - fs_max_rel_RGB_var);
        fs_cached_B_max = B*(1.0 + fs_max_rel_RGB_var);
    }
    else
    {
        fs_cached_R_min = FLT_NOT_SET;
        fs_cached_R_max = FLT_NOT_SET;
        fs_cached_G_min = FLT_NOT_SET;
        fs_cached_G_max = FLT_NOT_SET;
        fs_cached_B_min = FLT_NOT_SET;
        fs_cached_B_max = FLT_NOT_SET;
    }

    if (    (fs_max_abs_chrom_var >= FLT_ZERO)
         && (fs_max_rel_chrom_var >= FLT_ZERO))
    {
        fs_cached_r_chrom_min = MAX_OF(r_chrom - fs_max_abs_chrom_var,
                                    r_chrom*(1.0 - fs_max_rel_chrom_var));
        fs_cached_r_chrom_max = MIN_OF(r_chrom + fs_max_abs_chrom_var,
                                    r_chrom*(1.0 + fs_max_rel_chrom_var));

        fs_cached_g_chrom_min = MAX_OF(g_chrom - fs_max_abs_chrom_var,
                                    g_chrom*(1.0 - fs_max_rel_chrom_var));
        fs_cached_g_chrom_max = MIN_OF(g_chrom + fs_max_abs_chrom_var,
                                    g_chrom*(1.0 + fs_max_rel_chrom_var));
    }
    else if (fs_max_abs_chrom_var >= FLT_ZERO)
    {
        fs_cached_r_chrom_min = r_chrom - fs_max_abs_chrom_var;
        fs_cached_r_chrom_max = r_chrom + fs_max_abs_chrom_var;
        fs_cached_g_chrom_min = g_chrom - fs_max_abs_chrom_var;
        fs_cached_g_chrom_max = g_chrom + fs_max_abs_chrom_var;
    }
    else if (fs_max_rel_chrom_var >= FLT_ZERO)
    {
        fs_cached_r_chrom_min = r_chrom*(1.0 - fs_max_rel_chrom_var);
        fs_cached_r_chrom_max = r_chrom*(1.0 + fs_max_rel_chrom_var);
        fs_cached_g_chrom_min = g_chrom*(1.0 - fs_max_rel_chrom_var);
        fs_cached_g_chrom_max = g_chrom*(1.0 + fs_max_rel_chrom_var);
    }
    else
    {
        fs_cached_r_chrom_min = FLT_NOT_SET;
        fs_cached_r_chrom_max = FLT_NOT_SET;
        fs_cached_g_chrom_min = FLT_NOT_SET;
        fs_cached_g_chrom_max = FLT_NOT_SET;
    }

    if (    (fs_max_abs_sum_RGB_var >= FLT_ZERO)
         && (fs_max_rel_sum_RGB_var >= FLT_ZERO))
    {
        fs_cached_sum_RGB_min = MIN_OF(sum_RGB - fs_max_abs_sum_RGB_var,
                                    sum_RGB*(1.0 - fs_max_rel_sum_RGB_var));
        fs_cached_sum_RGB_max = MAX_OF(sum_RGB + fs_max_abs_sum_RGB_var,
                                    sum_RGB*(1.0 + fs_max_rel_sum_RGB_var));
    }
    else if (fs_max_abs_sum_RGB_var >= FLT_ZERO)
    {
        fs_cached_sum_RGB_min = sum_RGB - fs_max_abs_sum_RGB_var;
        fs_cached_sum_RGB_max = sum_RGB + fs_max_abs_sum_RGB_var;
    }
    else if (fs_max_rel_sum_RGB_var >= FLT_ZERO)
    {
        fs_cached_sum_RGB_min = sum_RGB*(1.0 - fs_max_rel_sum_RGB_var);
        fs_cached_sum_RGB_max = sum_RGB*(1.0 + fs_max_rel_sum_RGB_var);
    }
    else
    {
        fs_cached_sum_RGB_min = FLT_NOT_SET;
        fs_cached_sum_RGB_max = FLT_NOT_SET;
    }
#else
    fs_cached_R_min = MAX_OF(R - fs_max_abs_RGB_var,
                          R*(1.0 - fs_max_rel_RGB_var));
    fs_cached_R_max = MIN_OF(R + fs_max_abs_RGB_var,
                          R*(1.0 + fs_max_rel_RGB_var));

    fs_cached_G_min = MAX_OF(G - fs_max_abs_RGB_var,
                          G*(1.0 - fs_max_rel_RGB_var));
    fs_cached_G_max = MIN_OF(G + fs_max_abs_RGB_var,
                          G*(1.0 + fs_max_rel_RGB_var));

    fs_cached_B_min = MAX_OF(B - fs_max_abs_RGB_var,
                          B*(1.0 - fs_max_rel_RGB_var));
    fs_cached_B_max = MIN_OF(B + fs_max_abs_RGB_var,
                          B*(1.0 + fs_max_rel_RGB_var));

    fs_cached_r_chrom_min = MAX_OF(r_chrom - fs_max_abs_chrom_var,
                                r_chrom*(1.0 - fs_max_rel_chrom_var));
    fs_cached_r_chrom_max = MIN_OF(r_chrom + fs_max_abs_chrom_var,
                                r_chrom*(1.0 + fs_max_rel_chrom_var));

    fs_cached_g_chrom_min = MAX_OF(g_chrom - fs_max_abs_chrom_var,
                                g_chrom*(1.0 - fs_max_rel_chrom_var));
    fs_cached_g_chrom_max = MIN_OF(g_chrom + fs_max_abs_chrom_var,
                                g_chrom*(1.0 + fs_max_rel_chrom_var));
    fs_cached_sum_RGB_min = MAX_OF(sum_RGB - fs_max_abs_sum_RGB_var,
                                sum_RGB*(1.0 - fs_max_rel_sum_RGB_var));
    fs_cached_sum_RGB_max = MIN_OF(sum_RGB + fs_max_abs_sum_RGB_var,
                                sum_RGB*(1.0 + fs_max_rel_sum_RGB_var));
#endif

#ifdef TEST
    verbose_pso(300, "Growing (%-3d, %-3d) ... ", i, j);
#endif

    grow_segment_using_initial_map(i, j);

#ifdef TEST
    verbose_pso(300, "%d pixels.\n", fs_num_cached_pixels);
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_segment_using_initial_map(int i, int j)
{
#ifdef CONNECT_CORNER_OPTION
    if (fs_seg_connect_corners)
    {
        grow_segment_using_initial_map_8(i, j);
    }
    else
    {
        grow_segment_using_initial_map_4(i, j);
    }
#else
#ifdef DEFAULT_IS_TO_CONNECT_CORNERS
    grow_segment_using_initial_map_8(i, j);
#endif
    grow_segment_using_initial_map_4(i, j);
#endif
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_segment_using_initial_map_4(int i, int j)
{
    int   count;
    int   i_offset;
    int   j_offset;
    float cur_R;
    float cur_G;
    float cur_B;
    float cur_r_chrom;
    float cur_g_chrom;
    float cur_sum_RGB;
    Pixel cur_image_pixel;


    cur_image_pixel = fs_cached_ip->pixels[ i ][ j ];

    ASSERT(! (fs_cached_seg_map[ i ][ j ]));
    ASSERT(cur_image_pixel.extra.invalid.pixel == VALID_PIXEL);

    fs_cached_pixels[ fs_num_cached_pixels ].i = i;
    fs_cached_pixels[ fs_num_cached_pixels ].j = j;

    cur_R = cur_image_pixel.r;
    cur_G = cur_image_pixel.g;
    cur_B = cur_image_pixel.b;

    cur_sum_RGB = cur_R + cur_G + cur_B + RGB_SUM_EPSILON;
    cur_r_chrom = cur_R / cur_sum_RGB;
    cur_g_chrom = cur_G / cur_sum_RGB;

    fs_cached_r_chrom_sum += cur_r_chrom;
    fs_cached_g_chrom_sum += cur_g_chrom;
    fs_cached_R_sum += cur_R;
    fs_cached_G_sum += cur_G;
    fs_cached_B_sum += cur_B;
#ifdef MAINTAIN_SS
    fs_cached_r_chrom_sqr_sum += (cur_r_chrom * cur_r_chrom);
    fs_cached_g_chrom_sqr_sum += (cur_g_chrom * cur_g_chrom);
    fs_cached_sum_RGB_sqr_sum += (cur_sum_RGB * cur_sum_RGB);
#endif

    fs_num_cached_pixels++;

    ASSERT(fs_num_cached_pixels <= fs_cached_num_rows * fs_cached_num_cols);

    fs_cached_seg_map[ i ][ j ] = fs_cached_segment_count;

    for (count = 0; count < 2; count++)
    {
        int d;

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
                 && (i_offset < fs_cached_num_rows)
                 && (j_offset < fs_cached_num_cols)
                 && (fs_cached_seg_map[ i_offset ][ j_offset ] == 0)
                 && (fs_cached_ip->pixels[ i_offset ][ j_offset ].extra.invalid.pixel == VALID_PIXEL)
                 && (fs_cached_initial_map[ i_offset ][ j_offset ] == fs_cached_initial_map[ i ][ j ])
               )
            {
               grow_segment_using_initial_map_4(i_offset, j_offset);
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void grow_segment_using_initial_map_8(int i, int j)
{
    int   i_offset;
    int   j_offset;
    float cur_R;
    float cur_G;
    float cur_B;
    float cur_r_chrom;
    float cur_g_chrom;
    float cur_sum_RGB;
    Pixel cur_image_pixel;
    int   di, dj;


    cur_image_pixel = fs_cached_ip->pixels[ i ][ j ];

    ASSERT(! (fs_cached_seg_map[ i ][ j ]));
    ASSERT(cur_image_pixel.extra.invalid.pixel == VALID_PIXEL);

    fs_cached_pixels[ fs_num_cached_pixels ].i = i;
    fs_cached_pixels[ fs_num_cached_pixels ].j = j;

    cur_R = cur_image_pixel.r;
    cur_G = cur_image_pixel.g;
    cur_B = cur_image_pixel.b;

    cur_sum_RGB = cur_R + cur_G + cur_B + RGB_SUM_EPSILON;
    cur_r_chrom = cur_R / cur_sum_RGB;
    cur_g_chrom = cur_G / cur_sum_RGB;

    fs_cached_r_chrom_sum += cur_r_chrom;
    fs_cached_g_chrom_sum += cur_g_chrom;
    fs_cached_R_sum += cur_R;
    fs_cached_G_sum += cur_G;
    fs_cached_B_sum += cur_B;
#ifdef MAINTAIN_SS
    fs_cached_r_chrom_sqr_sum += (cur_r_chrom * cur_r_chrom);
    fs_cached_g_chrom_sqr_sum += (cur_g_chrom * cur_g_chrom);
    fs_cached_sum_RGB_sqr_sum += (cur_sum_RGB * cur_sum_RGB);
#endif

    fs_num_cached_pixels++;

    ASSERT(fs_num_cached_pixels <= fs_cached_num_rows * fs_cached_num_cols);

    fs_cached_seg_map[ i ][ j ] = fs_cached_segment_count;

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
                     && (i_offset < fs_cached_num_rows)
                     && (j_offset < fs_cached_num_cols)
                     && (fs_cached_seg_map[ i_offset ][ j_offset ] == 0)
                     && (fs_cached_ip->pixels[ i_offset ][ j_offset ].extra.invalid.pixel == VALID_PIXEL)
                     && (fs_cached_initial_map[ i_offset ][ j_offset ] == fs_cached_initial_map[ i ][ j ])
                   )
                {
                   grow_segment_using_initial_map_8(i_offset, j_offset);
                }
            }
        }
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


