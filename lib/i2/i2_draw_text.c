
/* $Id: i2_draw_text.c 21596 2017-07-30 23:33:36Z kobus $ */

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

#include "i2/i2_gen.h"     /* Only safe as first include in a ".c" file. */
#include "i2/i2_draw_text.h"

#define FONT_SUB_DIR         ".fonts"
#define LEGACY_FONT_SUB_DIR   "fonts"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int get_text_block_image
(
    KJB_image** ipp,
    const char* text_in,
    const char* font_str
);

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                             image_draw_text_top_left
 *
 * Draws text onto an image.
 *
 * This routine draws text at loction (i,j) (top left corner) onto an image.
 * This routine is very simple and uses a very old collection of bitmapped
 * fonts. 
 *
 * If the "font_str" argument is NULL, it defaults to times14. Other
 * possibilities in place of "times" are ariel, courier, monaco, and timesb.
 * Other possibilities in place of "14" are 9,10,12,14,18. 
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_text_top_left
(
    KJB_image*  ip,
    const char* text,
    int         i,
    int         j,
    const char* font_str
)
{
    KJB_image* text_block_ip = NULL;
    int result;


    ERE(get_text_block_image(&text_block_ip, text, font_str));

    result = image_draw_image(ip, text_block_ip, i, j, 1);

    kjb_free_image(text_block_ip);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     image_draw_wrapped_text_top_left
 *
 * Like image_draw_text_top_left but text is wrapped.
 *
 * This routine draws text at loction (i,j) (top left corner) onto an image.
 *
 * The size of the text box is specified by the width parameter. 
 *
 * This routine is very simple and uses a very old collection of bitmapped
 * fonts. 
 *
 * If the "font_str" argument is NULL, it defaults to times14. Other
 * possibilities in place of "times" are ariel, courier, monaco, and timesb.
 * Other possibilities in place of "14" are 9,10,12,14,18. 
 *
 * Note: The semantics of the wrapping are not currently well specified. 
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

/* Kobus, 12-08-03. I added this routine to test a small change I made to the
 * get_wrapped_text_block_image() routine added by someone else. I like the idea
 * of a wrapping version, but I am not sure what the intended semantics of
 * get_wrapped_text_block_image() are, or even if it is working corectly. 
*/

int image_draw_wrapped_text_top_left
(
    KJB_image*  ip,
    const char* text,
    int         i,
    int         j,
    int         width, 
    const char* font_str
)
{
    KJB_image* text_block_ip = NULL;
    int result;


    ERE(get_wrapped_text_block_image(&text_block_ip, text, width, font_str));

    result = image_draw_image(ip, text_block_ip, i, j, 1);

    kjb_free_image(text_block_ip);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             image_draw_text_center
 *
 * Draws text onto an image.
 *
 * This routine draws text at loction (i,j) (text block center) onto an image.
 * This routine is very simple and uses a very old collection of bitmapped
 * fonts.
 *
 * If the "font_str" argument is NULL, it defaults to times14. Other
 * possibilities in place of "times" are ariel, courier, monaco, and timesb.
 * Other possibilities in place of "14" are 9,10,12,14,18. 
 *
 * Index: images, image output
 *
 * -----------------------------------------------------------------------------
*/

int image_draw_text_center
(
    KJB_image*  ip,
    const char* text,
    int         i,
    int         j,
    const char* font_str
)
{
    KJB_image* text_block_ip = NULL;
    int result;
    int tb_num_rows;
    int tb_num_cols;


    ERE(get_text_block_image(&text_block_ip, text, font_str));

    tb_num_rows = text_block_ip->num_rows;
    tb_num_cols = text_block_ip->num_cols;

    result = image_draw_image(ip, text_block_ip,
                              i - tb_num_rows / 2,
                              j - tb_num_cols / 2,
                              1);

    kjb_free_image(text_block_ip);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_text_block_image
(
    KJB_image** ipp,
    const char* text_in,
    const char* font_str
)
{
    FILE*        font_fp;
    BITMAP       bitmap = NOT_SET;
    FONT         font;
    int          height = NOT_SET;
    int          width = NOT_SET;
    static unsigned char black[ 8 ]   = { 0, 0, 0, 255, 255, 255, 1, 0};
    const int    no_underline = 0;
    const int    no_italics   = 0;
    const int    no_wordwrap  = 0;
    int          result       = NO_ERROR;
    char         text[ 10000 ];
    char         font_file_name[ 1000 ];


    /*
    // The routine cd_drawtext will modify its text argument (to do word
    // wrapping)!
    */
    BUFF_CPY(text, text_in);

    cd_init();

    if (font_str == NULL)
    {
        font_str = "times14";
    }

    BUFF_CPY(font_file_name, font_str);
    BUFF_CAT(font_file_name, ".bfnt");

    font_fp = open_config_file((const char*)NULL, FONT_SUB_DIR,
                                   font_file_name, "font");

    if (font_fp == NULL)
    {
        static int first_time = TRUE;

        NRE(font_fp = open_config_file((const char*)NULL, LEGACY_FONT_SUB_DIR,
                                       font_file_name, "font"));

        if (first_time)
        {
            warn_pso("Using legacy font sub dir %q. It is now %q.\n", 
                     LEGACY_FONT_SUB_DIR, FONT_SUB_DIR);
            first_time = FALSE; 
        }
    }

    font = cd_loadfont(font_fp);

    if (font == -1)
    {
        set_error("Unable to load font.");
        result = ERROR;
    }

    kjb_fclose(font_fp);

    if (result != ERROR)
    {
        /*
         * How it was:
                 int font_height = cd_font_height[font];
        */
        int font_height = cd_get_font_height(font);

        height = font_height + 2;

        /*
        // Big enough. width won't exceed height.
        */
        width = 4 + font_height * strlen(text);

        bitmap = cd_newbitmap(width, height);

        if (bitmap == -1)
        {
            set_error("Unable to create bitmap for text.");
            result = ERROR;
        }
    }

    if (result != ERROR)
    {
        /* Actual width (does it include extra? */
        width = cd_drawtext(bitmap, font, 2, 2, black, text,
                            no_underline, no_italics, no_wordwrap);

        width += 2;

        if (width == -1)
        {
            set_error("Unable to draw text bitmap.");
            result = ERROR;
        }
    }

    if (result != ERROR)
    {
        result = get_target_image(ipp, height, width);
    }

    if (result != ERROR)
    {
        int i,j;

        for (i = 0; i < height; i++)
        {
            Pixel* pos = (*ipp)->pixels[ i ];

            for (j = 0; j < width; j++)
            {
                /*
                 * How it was:
                        pos->r = cd_map[bitmap][(i * cd_bitmapwidth[bitmap] + j) * 3];
                        pos->g = cd_map[bitmap][(i * cd_bitmapwidth[bitmap] + j) * 3 + 1 ];
                        pos->b = cd_map[bitmap][(i * cd_bitmapwidth[bitmap] + j) * 3 + 2 ];
                */
                pos->r = cd_get_map_value(bitmap, (i * cd_get_bitmap_width(bitmap) + j) * 3);
                pos->g = cd_get_map_value(bitmap, (i * cd_get_bitmap_width(bitmap) + j) * 3 + 1 );
                pos->b = cd_get_map_value(bitmap, (i * cd_get_bitmap_width(bitmap) + j) * 3 + 2 );
                pos->extra.invalid = valid_pixel_constant;

                pos++;
            }
        }
    }

    if ((KJB_IS_SET(bitmap)) && (bitmap != -1))
    {
        cd_freebitmap(bitmap);
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_wrapped_text_block_image
(
    KJB_image** ipp,
    const char* text_in,
    int         width,
    const char* font_str
)
{
    FILE*        font_fp;
    FONT         font;
    int          height = NOT_SET;
    int          result       = NO_ERROR;
    char         text[ 10000 ];
    char         font_file_name[ 1000 ];
    KJB_image*   word_ip = NULL;
    KJB_image*   blank_ip = NULL;
    KJB_image*   ip = NULL;
    int          blank_width = NOT_SET;
    char         line_buff[ 100000 ];
    char         word_buff[ 1000 ];
    const char*  text_pos;
    const char*  line_pos;
    int line_height = NOT_SET;
    int lines_written = 0;
    int cur_col = 0;
    int cur_row = 0;


    /*
    // The routine cd_drawtext will modify its text argument (to do word
    // wrapping)!
    */
    BUFF_CPY(text, text_in);

    cd_init();

    if (font_str == NULL)
    {
        font_str = "times14";
    }

    BUFF_CPY(font_file_name, font_str);
    BUFF_CAT(font_file_name, ".bfnt");

    font_fp = open_config_file((const char*)NULL, FONT_SUB_DIR,
                                   font_file_name, "font");

    if (font_fp == NULL)
    {
        static int first_time = TRUE;

        NRE(font_fp = open_config_file((const char*)NULL, LEGACY_FONT_SUB_DIR,
                                       font_file_name, "font"));

        if (first_time)
        {
            warn_pso("Using legacy font sub dir %q. It is now %q.\n", 
                     LEGACY_FONT_SUB_DIR, FONT_SUB_DIR);
            first_time = FALSE; 
        }
    }

    font = cd_loadfont(font_fp);

    if (font == -1)
    {
        set_error("Unable to load font.");
        result = ERROR;
    }

    kjb_fclose(font_fp);

    if (result != ERROR)
    {
        /*
         * How it was:
                 int font_height = cd_font_height[font];
        */
        int font_height = cd_get_font_height(font);

        line_height = font_height + 2;

        /* Big upper bound on what we will need. */
        height = 5 * line_height * (1 + font_height * strlen(text_in) / width);

        result = get_initialized_image_2(ipp, height, width, 255, 255, 255);
        ip = *ipp;
    }

    if (result != ERROR)
    {
        result = get_text_block_image(&blank_ip, " ", font_str);
        blank_width = blank_ip->num_cols;
    }

    if (result != ERROR)
    {
        text_pos = text_in;

        while (BUFF_CONST_GEN_GET_TOKEN_OK(&text_pos, line_buff, "\n"))
        {
            line_pos = line_buff;

            while (BUFF_CONST_GET_TOKEN_OK(&line_pos, word_buff))
            {
                if (result == ERROR) break;

                result = get_text_block_image(&word_ip, word_buff, font_str);
                if (result == ERROR) break;

                if (cur_col + word_ip->num_cols + blank_width > width)
                {
                    cur_row += line_height;
                    cur_col = 0;
                }

                result = image_draw_image(ip, word_ip, cur_row, cur_col, 1);
                if (result == ERROR) break;

                /* If we are at the begining of a line, and have actually
                // written someting, then the number of lines written is
                // increased.
                */
                if ((cur_col == 0) && (word_ip->num_cols > 0))
                {
                    lines_written++;
                }

                cur_col += word_ip->num_cols;

                result = image_draw_image(ip, blank_ip, cur_row, cur_col, 1);
                if (result == ERROR) break;
                cur_col += blank_width;
            }

            if (*text_pos != '\0')
            {
                cur_row += line_height;
                cur_col = 0;
            }
        }

        if (result != ERROR)
        {
            result = ip->num_rows = lines_written * line_height;
        }
    }

    kjb_free_image(blank_ip);
    kjb_free_image(word_ip);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

