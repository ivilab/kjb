
/* $Id: i_driver.c 21596 2017-07-30 23:33:36Z kobus $ */

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

#include "i/i_gen.h"

#include "i/i_driver.h"

#include "i/i_arithmetic.h"
#include "i/i_ave.h"
#include "i/i_colour.h"
#include "i/i_convolve.h"
#include "i/i_float_io.h"
#include "i/i_gamma.h"
#include "i/i_map.h"
#include "i/i_set.h"
#include "i/i_stat.h"
#include "i/i_transform.h"
#include "i/i_draw.h"

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define EGCRE(x)      if ((x) == ERROR)                                         \
                      {                                                        \
                          extern int kjb_debug_level;                          \
                                                                               \
                          if (kjb_debug_level > 0)                             \
                          {                                                    \
                              add_error("(EGCRE on line %d of %s.)",            \
                                        __LINE__, __FILE__);                   \
                          }                                                    \
                          result = ERROR;                                      \
                          goto cleanup;                                        \
                      }

/* -------------------------------------------------------------------------- */

#define MAX_NUM_OUTPUT_FILES  1000

static int process_option(const char* option, const char* value);

static int process_tri_value_option(Vector** vpp, char* value_buff);

/* -------------------------------------------------------------------------- */

int kjb_image_program_driver
(
    int    argc,
    char** argv,
    int    (*option_fn) (const char *, const char *),
    int    (*program_fn) (const KJB_image *,
                          const char *,
                          KJB_image **,
                          const char *,
                          const Int_matrix *)
)
{
    static int      invert               = FALSE;
    static int      display_image_flag   = FALSE;
    static int      gamma_correct        = FALSE;
    static int      invert_gamma         = FALSE;
    static int      channel_exp          = FALSE;
    static int      do_gm_logarithm      = FALSE;
    static int      do_gm_logarithm_2    = FALSE;
    static int      do_sum_logarithm     = FALSE;
    static int      do_sum_logarithm_2   = FALSE;
    static int      do_lum_logarithm     = FALSE;
    static int      do_lum_logarithm_2   = FALSE;
    static int      do_channel_logarithm = FALSE;
    static int      linearize_pcd        = FALSE;
    static int      convert_pcd_ycc      = FALSE;
    static int      read_pcd             = FALSE;
    static int      apply_pcd_lut        = FALSE;
    static int      black_and_white      = FALSE;
    static int      scale_by_max         = FALSE;
    static int      rotate_left          = FALSE;
    static int      rotate_right         = FALSE;
    static int      horizontal_flip      = FALSE;
    static int      vertical_flip        = FALSE;
    static int      make_even            = FALSE;
    static int      report_average       = FALSE;
    static int      report_stdev         = FALSE;
    static int      report_average_chrom = FALSE;

    static const struct Option long_options[] =
    {
        { "even",          NO_ARGUMENT,       &make_even,            TRUE  },
        { "left",          NO_ARGUMENT,       &rotate_left,          TRUE  },
        { "right",         NO_ARGUMENT,       &rotate_right,         TRUE  },
        { "horizontal_flip", NO_ARGUMENT,     &horizontal_flip,      TRUE  },
        { "vertical_flip", NO_ARGUMENT,       &vertical_flip,        TRUE  },
        { "d",             NO_ARGUMENT,       &display_image_flag,   TRUE  },
        { "i",             NO_ARGUMENT,       &invert,               TRUE  },
        { "n",             NO_ARGUMENT,       &gamma_correct,        FALSE },
        { "m",             NO_ARGUMENT,       &scale_by_max,         TRUE  },
        { "channel_exp",   NO_ARGUMENT,       &channel_exp,          TRUE  },
        { "lum_log",       NO_ARGUMENT,       &do_lum_logarithm,     TRUE  },
        { "lum_log_2",     NO_ARGUMENT,       &do_lum_logarithm_2,   TRUE  },
        { "sum_log",       NO_ARGUMENT,       &do_sum_logarithm,     TRUE  },
        { "sum_log_2",     NO_ARGUMENT,       &do_sum_logarithm_2,   TRUE  },
        { "gm_log",        NO_ARGUMENT,       &do_sum_logarithm,     TRUE  },
        { "gm_log_2",      NO_ARGUMENT,       &do_sum_logarithm_2,   TRUE  },
        { "channel_log",   NO_ARGUMENT,       &do_channel_logarithm, TRUE  },
        { "convert_pcd_ycc", NO_ARGUMENT,     &convert_pcd_ycc,      TRUE  },
        { "read_pcd",      NO_ARGUMENT,       &read_pcd,             TRUE  },
        { "linearize_pcd", NO_ARGUMENT,       &linearize_pcd,        TRUE  },
        { "apply_pcd_lut", NO_ARGUMENT,       &apply_pcd_lut,        TRUE  },
        { "BW",            NO_ARGUMENT,       &black_and_white,      TRUE  },
        { "A",             NO_ARGUMENT,       &report_average,       TRUE  },
        { "D",             NO_ARGUMENT,       &report_stdev,         TRUE  },
        { "C",             NO_ARGUMENT,       &report_average_chrom, TRUE  },
        { "match_sum",     REQUIRED_ARGUMENT, NULL,                  '4'   },
        { "match_lum",     REQUIRED_ARGUMENT, NULL,                  '5'   },
        { "match_chrom",   REQUIRED_ARGUMENT, NULL,                  '6'   },
        { "map_border",    REQUIRED_ARGUMENT, NULL,                  '7'   },
        { NULL,            0,                 NULL,                  0     }
    };

    static char*        input_files[ 3 ] = { NULL, NULL, NULL };
    static KJB_image* input_ip_array[ 2 ] = { NULL, NULL };
    static char*        non_option_args[ 5 ] = { NULL, NULL, NULL, NULL, NULL };
    char*        output_file      = NULL;
    KJB_image* output_ip        = NULL;
    KJB_image* input_ip;
    KJB_image* mapped_ip        = NULL;
    KJB_image* merge_ip         = NULL;
    KJB_image* reduced_ip       = NULL;
    KJB_image* left_rotated_ip  = NULL;
    KJB_image* right_rotated_ip = NULL;
    KJB_image* luminance_ip     = NULL;
    KJB_image* sum_ip           = NULL;
    KJB_image* chrom_ip         = NULL;
    KJB_image* sampled_ip       = NULL;
    KJB_image* median_ip       = NULL;
    Vector*      max_vp           = NULL;
    double       max;
    int          block_size       = NOT_SET;
    int          x                = NOT_SET;
    int          y                = NOT_SET;
    int          width            = NOT_SET;
    int          height           = NOT_SET;
    int          num_rows, num_cols;
    Vector*      gamma_vp         = NULL;
    Vector*      factor_vp        = NULL;
    Vector*      offset_vp        = NULL;
    Vector*      ave_vp           = NULL;
    Vector*      stdev_vp         = NULL;
    Matrix*      transform_mp     = NULL;
    int          option;
    char         value_buff[ MAX_OF(1000, MAX_FILE_NAME_SIZE) ];
    int          roll = NOT_SET;
    double       chrom_intensity       = NOT_SET;
    const char*  option_str;
    int          num_non_option_args   = 0;
    double       merge_factors[ 2 ];
    char         display_title[ 1000 ];
    int          count;
    int          log_op_count;
    double       sigma        = DBL_NOT_SET;
    int          sample_scale = NOT_SET;
    int          median_scale = NOT_SET;
    Int_matrix*  region_map_mp = NULL;
    Int_matrix*  windowed_region_map_mp = NULL;
    const Int_matrix*  derived_region_map_mp;
    int          num_output_files = 0;
    char         output_files[ MAX_NUM_OUTPUT_FILES ][ MAX_FILE_NAME_SIZE ];
    int i, j;
    int result = NO_ERROR;
    int map_border = 0;


    if (option_fn == NULL)
    {
        option_fn = process_option;
    }

    /*
     * The dash at the begining of the option string tells GNU getopt to return
     * a non option argument as the argument to an option with the character
     * code NON_OPTION_ARGUMENT, which is #define as 1. (kjb_getopts is just a
     * front end to a copy of GNU getopt).
    */
    while ((option = kjb_getopts(argc, argv,
                                 "-R:M:vlow:h:x:y:b:s:a:I:r:S:L::g::c:4:5:6:7:PG:B:N:O:",
                                 long_options, value_buff, sizeof(value_buff)))
               != EOF
          )
    {
        switch (option)
        {
            case ERROR :
                set_error("Image program has invalid arguments.");
                return ERROR;
            case 0 : /* Long option. Nothing more to do. */
                break;
            case 'P' :
                EGCRE(process_option_string("", option_fn));
                pso("\n\n");
                break;
            case '4' :
                EGCRE(kjb_read_image(&sum_ip, value_buff));
                break;
            case '5' :
                EGCRE(kjb_read_image(&luminance_ip, value_buff));
                break;
            case '6' :
                EGCRE(kjb_read_image(&chrom_ip, value_buff));
                break;
            case '7' :
                EGCRE(ss1pi(value_buff, &map_border));
                break;
            case 'M' :
                EGCRE(read_matrix(&transform_mp, value_buff));

                if (    (transform_mp->num_rows != 3)
                     || (    (transform_mp->num_cols != 3)
                          && (transform_mp->num_cols != 4)
                        )
                    )
                {
                    set_error("The transform matrix must be 3x3 or 3x4.");
                    return ERROR;
                }
                break;
            case 'R' :
                EGCRE(read_int_matrix(&region_map_mp, value_buff));
                break;
            case 'S' :
                EGCRE(process_option_string(value_buff, option_fn));
                break;
            case 'v' :
                kjb_set_verbose_level(INT_MAX);
                break;
            case 'o' :
                option_str = "cvof=on";

                if (process_option_string(option_str, option_fn) == ERROR)
                {
                    set_bug("%q is an invalid option string.", option_str);
                    return ERROR;
                }
                break;
            case 'l' :
                option_str = "linearization=on";

                if (process_option_string(option_str, option_fn) == ERROR)
                {
                    set_bug("%q is an invalid option string.", option_str);
                    return ERROR;
                }
                break;
            case 'g' :
                gamma_correct = TRUE;

                /*
                 * Gamma is shared with invert gamma option.
                */
                if (value_buff[ 0 ] != '\0')
                {
                    EGCRE(process_tri_value_option(&gamma_vp, value_buff));
                }
                break;
            case 'L' :
                invert_gamma = TRUE;

                /*
                 * Gamma is shared with gamma option.
                */
                if (value_buff[ 0 ] != '\0')
                {
                    EGCRE(process_tri_value_option(&gamma_vp, value_buff));
                }
                break;
            case 'I' :
                EGCRE(kjb_i_set("ipc", value_buff));
                break;
            case 'c' :
                EGCRE(ss1d(value_buff, &chrom_intensity));
                break;
            case 'w' :
                EGCRE(ss1pi(value_buff, &width));
                break;
            case 'h' :
                EGCRE(ss1pi(value_buff, &height));
                break;
            case 'x' :
                EGCRE(ss1pi(value_buff, &x));
                break;
            case 'y' :
                EGCRE(ss1pi(value_buff, &y));
                break;
            case 'r' :
                EGCRE(ss1i(value_buff, &roll));
                break;
            case 'a' :
                EGCRE(process_tri_value_option(&offset_vp, value_buff));
                break;
            case 's' :
                EGCRE(process_tri_value_option(&factor_vp, value_buff));
                break;
            case 'G' :
                EGCRE(ss1d(value_buff, &sigma));
                break;
            case 'B' :
                EGCRE(ss1pi(value_buff, &sample_scale));
                break;
            case 'N' :
                EGCRE(ss1pi(value_buff, &median_scale));
                break;
            case 'O' :
                if (num_output_files >= MAX_NUM_OUTPUT_FILES)
                {
                    set_error("The number of output files exceeds the maximum allowed (%d).",
                              MAX_NUM_OUTPUT_FILES);
                    return ERROR;
                }
                BUFF_CPY(output_files[ num_output_files ], value_buff);
                num_output_files++;
                break;
            case 'b' :
                EGCRE(ss1pi(value_buff, &block_size));

                if (block_size == 0)
                {
                    set_error("Block size must be positive.");
                    return ERROR;
                }

                break;
            case NON_OPTION_ARGUMENT :
                if (num_non_option_args < 5)
                {
                    non_option_args[ num_non_option_args ] =
                                                       kjb_strdup(value_buff);
                }
                num_non_option_args++;

                break;
            default :
                break;
        }
    }

    if (linearize_pcd)
    {
        option_str = "linearize-pcd=on";

        if (process_option_string(option_str, option_fn) == ERROR)
        {
            set_bug("%q is an invalid option string.", option_str);
            return ERROR;
        }
    }

    if (read_pcd)
    {
        option_str = "read-pcd=on";

        if (process_option_string(option_str, option_fn) == ERROR)
        {
            set_bug("%q is an invalid option string.", option_str);
            return ERROR;
        }
    }

    if (convert_pcd_ycc)
    {
        option_str = "convert-pcd-ycc=on";

        if (process_option_string(option_str, option_fn) == ERROR)
        {
            set_bug("%q is an invalid option string.", option_str);
            return ERROR;
        }
    }

    if ((num_non_option_args == 2) || (num_non_option_args == 5))
    {
        num_non_option_args--;
        output_file = non_option_args[ num_non_option_args ];
    }

    if (num_non_option_args == 1)
    {
        input_files[ 0 ] = non_option_args[ 0 ];
        BUFF_CPY(display_title, input_files[ 0 ]);
    }
    else if (num_non_option_args == 4)
    {
        EGCRE(ss1d(non_option_args[ 0 ], &(merge_factors[ 0 ])));
        input_files[ 0 ] = non_option_args[ 1 ];
        EGCRE(ss1d(non_option_args[ 2 ], &(merge_factors[ 1 ])));
        input_files[ 1 ] = non_option_args[ 3 ];

        EGCRE(kjb_sprintf(display_title, sizeof(display_title), "(%s)%s+(%s)%s",
                          non_option_args[ 0 ], non_option_args[ 1 ],
                          non_option_args[ 2 ], non_option_args[ 3 ]));
    }
    else
    {
        const char* image_program = (argc == 0) ? "[image_program] " : argv[ 0 ];

        set_error("Invalid number of non-option arguments.");
        add_error("Basic usage is one of the following:");
        add_error("    %s [input_file] {output_file}", image_program);
        add_error("    %s [factor] [input_file] ", image_program);
        cat_error("[factor] [input_file] {output_file}");

        return ERROR;
    }

    count = 0;

    while (input_files[ count ] != NULL)
    {
        EGCRE(kjb_read_image(&input_ip_array[ count ], input_files[ count]));

        if ((make_even) && (IS_ODD(input_ip_array[ count ]->num_rows)))
        {
            (input_ip_array[ count ]->num_rows)--;
        }

        if ((make_even) && (IS_ODD(input_ip_array[ count ]->num_cols)))
        {
            (input_ip_array[ count ]->num_cols)--;
        }

        if (KJB_IS_SET(roll))
        {
            Pixel* row;

            num_rows = input_ip_array[ count ]->num_rows;
            num_cols = input_ip_array[ count ]->num_cols;

            NPETE(row = N_TYPE_MALLOC(Pixel, num_cols));

            while (roll < 0) roll += num_cols;

            for (i=0; i<num_rows; i++)
            {
                for (j=0; j<num_cols; j++)
                {
                    row[ j ] = input_ip_array[ count ]->pixels[ i ][ j ];
                }

                for (j=0; j<num_cols; j++)
                {
                    input_ip_array[ count ]->pixels[ i ][ j ] =
                                                  row[ (j + roll) % num_cols ];
                }
            }

            kjb_free(row);
        }

        if (KJB_IS_SET(height) || KJB_IS_SET(width) || KJB_IS_SET(x) || KJB_IS_SET(y))
        {
            KJB_image* window_ip = NULL;


            num_rows = input_ip_array[ count ]->num_rows;
            num_cols = input_ip_array[ count ]->num_cols;

            if (KJB_IS_SET(height) && (width ==  NOT_SET)) width = height;
            if (KJB_IS_SET(width)  && (height == NOT_SET)) height = width;

            if (KJB_IS_SET(height) && KJB_IS_SET(width) && (x==NOT_SET) && (y==NOT_SET))
            {
                x = (num_cols - width)  / 2;
                y = (num_rows - height) / 2;
            }
            else if (    (height == NOT_SET) || (width == NOT_SET)
                      || (x == NOT_SET)      || (y == NOT_SET)
                     )
            {
                set_error("Window size is either width and height ");
                cat_error("(2 parameters).");
                add_error("                   OR width and height and ");
                cat_error("X and Y (4 parameters).");

                return ERROR;
            }

            if (    (x < 0) || (y < 0)
                 || (x + width > num_cols)
                 || (y + height > num_rows)
                 || (width < 1) || (height < 1)
               )
            {
                set_error("Invalid window size for %d by %d image.",
                          num_rows, num_cols);

                return ERROR;
            }

            EGCRE(get_image_window(&window_ip, input_ip_array[ count ],
                                         y, x, height, width));

            kjb_free_image(input_ip_array[ count ]);

            input_ip_array[ count ] = window_ip;
        }
        count++;
    }

    if (count == 1)
    {
        input_ip = input_ip_array[ 0 ];
    }
    else
    {
        EGCRE(ow_scale_image(input_ip_array[ 0 ], merge_factors[ 0 ]));
        EGCRE(ow_scale_image(input_ip_array[ 1 ], merge_factors[ 1 ]));
        EGCRE(add_images(&merge_ip, input_ip_array[ 0 ], input_ip_array[ 1 ]));

        input_ip = merge_ip;
    }

    if (block_size > 1)
    {
        EGCRE(ave_image_without_invalid(&reduced_ip, input_ip, block_size,
                                        block_size, 1));
        input_ip = reduced_ip;
    }

    if ((sample_scale > 1) && (sigma > 0.0))
    {
        EGCRE(gauss_sample_image(&sampled_ip, input_ip, sample_scale, sigma));
        input_ip = sampled_ip;
    }
    else if (sample_scale > 1)
    {
        EGCRE(sample_image(&sampled_ip, input_ip, sample_scale, sample_scale));
        input_ip = sampled_ip;
    }
    else if (sigma > 0.0)
    {
        EGCRE(ow_gauss_convolve_image(input_ip, sigma));
    }

    if (median_scale > 1)
    {
        EGCRE(median_filter_image(&median_ip, input_ip, median_scale));
        input_ip = median_ip;
    }

    if (rotate_left)
    {
        EGCRE(rotate_image_left(&left_rotated_ip, input_ip));
        input_ip = left_rotated_ip;
    }

    if (rotate_right)
    {
        EGCRE(rotate_image_right(&right_rotated_ip, input_ip));
        input_ip = right_rotated_ip;
    }

    if (transform_mp != NULL)
    {
        EGCRE(pre_map_image(&mapped_ip, input_ip, transform_mp));
        input_ip = mapped_ip;
    }

    if (horizontal_flip)
    {
        EGCRE(ow_horizontal_flip_image(input_ip));
    }

    if (vertical_flip)
    {
        EGCRE(ow_vertical_flip_image(input_ip));
    }

    if (program_fn == NULL)
    {
        EGCRE(kjb_copy_image(&output_ip, input_ip));
    }
    else
    {
        if (map_border > 0)
        {
            num_rows = region_map_mp->num_rows - 2 * map_border;
            num_cols = region_map_mp->num_cols - 2 * map_border;

            if ((num_rows <= 0) || (num_cols <= 0))
            {
                set_error("Border is too large for image.");
                result = ERROR;
                goto cleanup;
            }

            EGCRE(copy_int_matrix_block(&windowed_region_map_mp,
                                        region_map_mp,
                                        map_border, map_border,
                                        num_rows, num_cols));
            derived_region_map_mp = windowed_region_map_mp;

        }
        else
        {
            derived_region_map_mp = region_map_mp;
        }

        EGCRE((*program_fn)(input_ip, display_title, &output_ip, output_file,
                            derived_region_map_mp));
    }

    if (output_ip == NULL)
    {
        goto cleanup;
    }

    if (channel_exp)
    {
        EGCRE(ow_exponantiate_image(output_ip));
    }

    log_op_count = 0;
    if (do_channel_logarithm) log_op_count++;
    if (do_lum_logarithm)     log_op_count++;
    if (do_lum_logarithm_2)   log_op_count++;
    if (do_sum_logarithm)     log_op_count++;
    if (do_sum_logarithm_2)   log_op_count++;
    if (do_gm_logarithm)      log_op_count++;
    if (do_gm_logarithm_2)    log_op_count++;

    if (log_op_count > 1)
    {
        set_error("Specifying more than one logarithm operation is invalid.");
        return ERROR;
    }
    else if (do_channel_logarithm)
    {
        EGCRE(ow_log_one_plus_image(output_ip));
    }
    else if (do_lum_logarithm)
    {
        EGCRE(ow_log_brightness_image(output_ip, sRGB_Y_brightness, 1.0));
    }
    else if (do_lum_logarithm_2)
    {
        EGCRE(ow_log_brightness_image(output_ip, sRGB_Y_brightness, 2.2));
    }
    else if (do_sum_logarithm)
    {
        EGCRE(ow_log_brightness_image(output_ip, rgb_sum_brightness, 1.0));
    }
    else if (do_sum_logarithm_2)
    {
        EGCRE(ow_log_brightness_image(output_ip, rgb_sum_brightness, 2.2));
    }
    else if (do_gm_logarithm)
    {
        EGCRE(ow_log_brightness_image(output_ip, rgb_gm_brightness, 1.0));
    }
    else if (do_gm_logarithm_2)
    {
        EGCRE(ow_log_brightness_image(output_ip, rgb_gm_brightness, 2.2));
    }

    if (sum_ip != NULL)
    {
        EGCRE(ow_match_brightness(output_ip, sum_ip, rgb_sum_brightness));
    }

    if (luminance_ip != NULL)
    {
        EGCRE(ow_match_brightness(output_ip, luminance_ip, sRGB_Y_brightness));
    }

    if (chrom_ip != NULL)
    {
        EGCRE(ow_match_chromaticity(output_ip, chrom_ip));
    }

    if (offset_vp != NULL)
    {
        EGCRE(ow_add_vector_to_image(output_ip, offset_vp));
    }

    if (scale_by_max)
    {
        EGCRE(get_max_rgb(&max_vp, output_ip));

        max = MAX_OF(MAX_OF(max_vp->elements[ 0 ],
                            max_vp->elements[ 1 ]) ,
                     max_vp->elements[ 2 ]);

        ow_scale_image(output_ip, 255.0 / max);

        verbose_pso(1, "Unclipped R max is %.2f\n", max_vp->elements[ 0 ]);
        verbose_pso(1, "Unclipped G max is %.2f\n", max_vp->elements[ 1 ]);
        verbose_pso(1, "Unclipped B max is %.2f\n\n", max_vp->elements[ 2 ]);
        verbose_pso(1, "Overall unclipped max is %.2f\n", max);
    }

    if (factor_vp != NULL)
    {
        EGCRE(ow_scale_image_by_channel(output_ip, factor_vp));
    }

    if (invert)
    {
        EGCRE(ow_invert_image(output_ip));
    }

    if (chrom_intensity >= 0.0)
    {
        EGCRE(ow_make_chromaticity_image(output_ip, chrom_intensity));
    }

    if (black_and_white)
    {
        EGCRE(ow_make_black_and_white_image(output_ip));
    }

    if (gamma_correct)
    {
        EGCRE(ow_gamma_correct_image(output_ip, gamma_vp));
    }

    if (invert_gamma)
    {
        EGCRE(ow_invert_image_gamma(output_ip, gamma_vp));
    }

    if (apply_pcd_lut)
    {
        EGCRE(ow_apply_pcd_output_lut(output_ip));
    }

    if ((report_average) && (report_stdev))
    {
        EGCRE(get_image_stats(NULL, &ave_vp, &stdev_vp, output_ip));
        EGCRE(pso("%.2f  %.2f  %.2f    %.3f  %.3f  %.3f\n",
                  ave_vp->elements[ 0 ],
                  ave_vp->elements[ 1 ],
                  ave_vp->elements[ 2 ],
                  stdev_vp->elements[ 0 ],
                  stdev_vp->elements[ 1 ],
                  stdev_vp->elements[ 2]));
    }
    else if (report_average)
    {
        EGCRE(get_ave_rgb(&ave_vp, output_ip));
        EGCRE(pso("%.2f  %.2f  %.2f\n",
                  ave_vp->elements[ 0 ],
                  ave_vp->elements[ 1 ],
                  ave_vp->elements[ 2 ]));
    }
    else if (report_stdev)
    {
        EGCRE(get_image_stats(NULL, (Vector**)NULL, &stdev_vp, output_ip));
        EGCRE(pso("%.3f  %.3f  %.3f\n",
                  stdev_vp->elements[ 0 ],
                  stdev_vp->elements[ 1 ],
                  stdev_vp->elements[ 2]));
    }

    if (report_average_chrom)
    {
        double sum;

        EGCRE(get_ave_rgb(&ave_vp, output_ip));
        sum = ave_vp->elements[ 0 ] + ave_vp->elements[ 1 ] +
                                                          ave_vp->elements[ 2 ];
        if (sum > 0.0)
        {
            EGCRE(pso("%.3f  %.3f\n",
                      ave_vp->elements[ 0 ] / sum,
                      ave_vp->elements[ 1 ] / sum));
        }
        else
        {
            EGCRE(pso("%.3f  %.3f\n", -1.0, -1.0));
        }
    }

    if (display_image_flag)
    {
        if (output_file != NULL)
        {
            BUFF_CPY(display_title, output_file);
        }

        EGCRE(fork_display_image(output_ip, display_title));
    }

    if (output_file != NULL)
    {
        EGCRE(kjb_write_image(output_ip, output_file));
    }

    for (i = 0; i < num_output_files; i++)
    {
        EGCRE(kjb_write_image(output_ip, output_files[ i ]));
    }

cleanup:

    free_vector(max_vp);
    free_vector(gamma_vp);
    free_vector(factor_vp);
    free_vector(offset_vp);
    free_matrix(transform_mp);
    free_vector(ave_vp);
    free_vector(stdev_vp);
    kjb_free_image(input_ip_array[ 0 ]);
    kjb_free_image(input_ip_array[ 1 ]);
    kjb_free_image(output_ip);
    kjb_free_image(mapped_ip);
    kjb_free_image(merge_ip);
    kjb_free_image(reduced_ip);
    kjb_free_image(sampled_ip);
    kjb_free_image(median_ip);
    kjb_free_image(left_rotated_ip);
    kjb_free_image(right_rotated_ip);
    kjb_free_image(luminance_ip);
    kjb_free_image(sum_ip);
    kjb_free_image(chrom_ip);
    free_int_matrix(region_map_mp);
    free_int_matrix(windowed_region_map_mp);

    kjb_free(non_option_args[ 0 ]);
    kjb_free(non_option_args[ 1 ]);
    kjb_free(non_option_args[ 2 ]);
    kjb_free(non_option_args[ 3 ]);
    kjb_free(non_option_args[ 4 ]);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int process_tri_value_option(Vector** vpp, char* value_buff)
{

    char    word[ 100 ];
    double    temp;
    Vector* vp;
    char*   value_buff_pos;


    ERE(get_target_vector(vpp, 3));
    vp = *vpp;

    value_buff_pos = value_buff;

    BUFF_GEN_GET_TOKEN(&value_buff_pos, word, ",");

    EPETE(ss1d(word, &temp));
    (vp->elements)[0] = temp;

    if (BUFF_GEN_GET_TOKEN(&value_buff_pos, word, ","))
    {
        EPETE(ss1d(word, &temp));
    }

    (vp->elements)[1] = temp;

    if (BUFF_GEN_GET_TOKEN(&value_buff_pos, word, ","))
    {
        EPETE(ss1d(word, &temp));
    }

    (vp->elements)[2] = temp;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define MAX_NUM_SET_FN  100

static int process_option(const char* option, const char* value)
{
    int  result = NOT_FOUND;
    int  temp_result;
    int  (*set_fn_list[ MAX_NUM_SET_FN ])(const char* opt, const char* val);

    set_fn_list[ 0 ] = kjb_l_set;
    set_fn_list[ 1 ] = kjb_i_set;

    ERE(temp_result = call_set_fn(2, set_fn_list, "Options",
                                      option, value));
    if (temp_result == NO_ERROR) result = NO_ERROR;

    if (result == NOT_FOUND)
    {
        set_error("%q is an invalid option.", option);
        return ERROR;
    }

    return NO_ERROR;
}

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif


