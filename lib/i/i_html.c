
/* $Id: i_html.c 4727 2009-11-16 20:53:54Z kobus $ */

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
#include "i/i_html.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static char fs_html_image_file_format[ 100 ] = {'g', 'i', 'f'} ;

/* -------------------------------------------------------------------------- */

int set_image_html_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result             = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
         || (match_pattern(lc_option, "html-image-file-format"))
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("html-image-file-format = %s\n",
                    fs_html_image_file_format));
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("HTML image files default to type with suffix %s.\n",
                    fs_html_image_file_format));
        }
        else
        {
            BUFF_CPY(fs_html_image_file_format, value);
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              output_image_for_html
 *
 * Makes html for images
 *
 * This routine writes an image into directory "dir", and writes an html line
 * linking to it into the file pointed to by "index_fp". The format of the image
 * file is determined by the internal option "html-image-file-format" which
 * takes as an argument an image file format suffix. The default is "gif". The
 * option can be set by the user in programs which call set_image_html_options()
 * as part of their user option functions, or by the programmer directly
 * throught that function.  Tiff is not that well supported by browsers (yet).
 * If the image location requires a full URL, then the similar routine
 * output_image_for_html_2() should be used instead.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure with an appropriate error
 *     message being set.
 *
 * Index: images, html
 *
 * -----------------------------------------------------------------------------
*/

int output_image_for_html
(
    const KJB_image* ip,
    FILE*            index_fp,
    const char*      dir,
    const char*      file_name
)
{

    return output_image_for_html_2(ip, index_fp, dir, file_name, NULL);
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              output_image_for_html_2
 *
 * Makes html for images
 *
 * This routine is similar to output_image_for_html(), except that there is an
 * additional argument, "location_str" for the URL prefix. (The routine
 * output_image_for_html() assumes relative paths). If location str is NULL,
 * then this routine behaves exactly as output_image_for_html(). The variable
 * location_str might be something like "http://kobus.ca/research/fancy_stuff/".
 * Typically a trailing slash is necessary.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure with an appropriate error
 *     message being set.
 *
 * Index: images, html
 *
 * -----------------------------------------------------------------------------
*/

int output_image_for_html_2
(
    const KJB_image* ip,
    FILE*            index_fp,
    const char*      dir,
    const char*      file_name,
    const char*      location_str
)
{
    char file_path[ MAX_FILE_NAME_SIZE ];


    ERE(kjb_fprintf(index_fp,
                    "<IMG SRC=%s%s.%s HEIGHT=%d WIDTH=%d>\n",
                    (location_str == NULL) ? "" : location_str,
                    file_name, fs_html_image_file_format,
                    ip->num_rows, ip->num_cols));

    ERE(kjb_sprintf(file_path, sizeof(file_path), "%s%s%s.%s",
                    dir, DIR_STR, file_name,
                    fs_html_image_file_format));

    ERE(kjb_write_image(ip, file_path));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

