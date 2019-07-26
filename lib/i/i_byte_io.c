
/* $Id: i_byte_io.c 4727 2009-11-16 20:53:54Z kobus $ */

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

#include "i/i_display.h"
#include "i/i_byte.h"
#include "i/i_convert.h"
#include "i/i_ras.h"

#include "i/i_byte_io.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define MAX_NUM_DISPLAYED_IMAGES    1000

/* -------------------------------------------------------------------------- */

static int read_raster_byte_image(Byte_image** ip, FILE* fp);

static int write_raster_byte_image(const Byte_image* ip, char* file_name);

static int write_byte_image_for_display(const void* ip, char* title);


/* =============================================================================
 *                              read_byte_image
 *
 *
 *
 *
 * Index: images, Byte images, Byte image I/O
 *
 * -----------------------------------------------------------------------------
*/

int read_byte_image(Byte_image** ipp, char* file_name)
{
    FILE*  fp;
    kjb_uint32 magic_number;
    int    result;


    NRE(fp = kjb_fopen(file_name, "rb"));

    if (FIELD_READ(fp, magic_number) == ERROR)
    {
        (void)kjb_fclose(fp);  /* Ignore return--only reading. */
        return ERROR;
    }

    if (magic_number == RASTER_MAGIC_NUM)
    {
        result = read_raster_byte_image(ipp, fp);
        (void)kjb_fclose(fp);  /* Ignore return--only reading. */
    }
    else
    {
        char temp_file_name[ MAX_FILE_NAME_SIZE ];

        ERE(verbose_pso(3, "%F is not a raster file. Arranging conversion.\n",
                        fp));

        (void)kjb_fclose(fp);  /* Ignore return--only reading. */

        ERE(BUFF_GET_TEMP_FILE_NAME(temp_file_name));
        BUFF_CAT(temp_file_name, ".sun");

        ERE(convert_image_file_to_raster(file_name, temp_file_name));

        NRE(fp = kjb_fopen(temp_file_name, "rb"));

        if (FIELD_READ(fp, magic_number) == ERROR)
        {
            Error_action save_error_action = get_error_action();

            /* We don't want to miss unlinking error. */
            set_error_action(FORCE_ADD_ERROR_ON_ERROR);
            (void)kjb_fclose(fp);
            (void)kjb_unlink(temp_file_name);
            set_error_action(save_error_action);

            return ERROR;
        }

        if (magic_number == 0x59a66a95)
        {
            result = read_raster_byte_image(ipp, fp);
        }
        else
        {
            set_error("Converted tempory image is not a valid raster file.");
            result = ERROR;
        }

        /* We don't want to miss unlinking error. */
        EPE(kjb_fclose(fp));
        EPE(kjb_unlink(temp_file_name));
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int read_raster_byte_image(Byte_image** ipp, FILE* fp)
{
#ifdef NOT_USED
    /*
    // KJB_image stripping is currently only used with float images.
    */
    extern int  strip_images_on_read;
    int         strip_image = FALSE;
#endif
    Byte_image* ip;
    int         i;
    int         j;
    int         num_rows;
    int         num_cols;
    int         row_length;
    Sun_header  sun_header;
    off_t       raster_size;
    off_t       file_size;
    unsigned char*      data_row;
    unsigned char*      data_row_pos;
    Byte_pixel* out_pos;
    int         result;


    ERE(FIELD_READ(fp, sun_header.width));
    ERE(FIELD_READ(fp, sun_header.height));
    ERE(FIELD_READ(fp, sun_header.depth));
    ERE(FIELD_READ(fp, sun_header.length));
    ERE(FIELD_READ(fp, sun_header.type));
    ERE(FIELD_READ(fp, sun_header.maptype));
    ERE(FIELD_READ(fp, sun_header.maplength));

    if (     (sun_header.maptype != RMT_NONE)
          ||
             (    (sun_header.type != RT_STANDARD)
               && (sun_header.type != RT_FORMAT_RGB)
             )
          ||
             (    (sun_header.depth != 24)
               && (sun_header.depth != 32)
             )
       )
    {
        set_error("File is not a 24 or 32 bit standard or RGB raster file.");
        return ERROR;
    }

    /*
    // Documentation suggests that lines are rounded to nearest 16 bits. In the
    // two simple cases of 24 or 32 bit images, this simply means making sure
    // that row_length is even in the case of 24 bits.
    */

    if (sun_header.depth == 24)
    {
        row_length = 3 * sun_header.width;

        if (IS_ODD(row_length))
        {
            row_length++;
        }
    }
    else
    {
        row_length = 4 * sun_header.width;
    }

    raster_size = row_length * sun_header.height + 32;

    ERE(fp_get_byte_size(fp, &file_size));

    if (file_size != raster_size)
    {
        set_error("%F is not a valid raster file.", fp);
        add_error("It has too %s data.",
                  (file_size > raster_size) ? "much" : "little");
        return ERROR;
    }

    NRE(data_row = BYTE_MALLOC(row_length));

    num_rows = sun_header.height;
    num_cols = sun_header.width;

#ifdef NOT_USED
    /*
    // KJB_image stripping is currently only used with float images.
    */
    if (    (strip_images_on_read)
         && (num_rows == FULL_VIDEO_NUM_ROWS)
         && (num_cols == FULL_VIDEO_NUM_COLS)
       )
    {
        strip_image = TRUE;
        num_rows = STRIPPED_VIDEO_NUM_ROWS;
        num_cols = STRIPPED_VIDEO_NUM_COLS;
    }
#endif

    result = get_target_byte_image(ipp, num_rows, num_cols);

    if (result == ERROR)
    {
        kjb_free(data_row);
        return ERROR;
    }

    ip = *ipp;

#ifdef NOT_USED
    /*
    // KJB_image stripping is currently only used with float images.
    */
    if (strip_image)
    {
        for (i=0; i<STRIP_TOP; i++)
        {
            if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
            {
                kjb_free(data_row);
                return ERROR;
            }
        }
    }
#endif

    for(i=0; i<num_rows; i++)
    {
        if (kjb_fread(fp, data_row, (size_t)row_length) == ERROR)
        {
            kjb_free(data_row);
            return ERROR;
        }

        data_row_pos = data_row;

#ifdef NOT_USED
        /*
        // KJB_image stripping is currently only used with float images.
        */
        if (strip_image)
        {
            data_row_pos += 3 * STRIP_LEFT;
        }
#endif

        out_pos = ip->pixels[ i ];

        /*
        // It seems OK to check file type once per line, but once per pixel is
        // too much extra work!
        */

        if (sun_header.type == RT_FORMAT_RGB)
        {
            if (sun_header.depth == 24)
            {
                for(j=0; j<num_cols; j++)
                {
                    out_pos->r = *data_row_pos++;
                    out_pos->g = *data_row_pos++;
                    out_pos->b = *data_row_pos++;

                    out_pos++;
                }
            }
            else
            {
                for(j=0; j<num_cols; j++)
                {
                    data_row_pos++;

                    out_pos->r = *data_row_pos++;
                    out_pos->g = *data_row_pos++;
                    out_pos->b = *data_row_pos++;

                    out_pos++;
                }
            }
        }
        else
        {
            if (sun_header.depth == 24)
            {
                for(j=0; j<num_cols; j++)
                {
                    out_pos->b = *data_row_pos++;
                    out_pos->g = *data_row_pos++;
                    out_pos->r = *data_row_pos++;

                    out_pos++;
                }
            }
            else
            {
                for(j=0; j<num_cols; j++)
                {
                    data_row_pos++;

                    out_pos->b = *data_row_pos++;
                    out_pos->g = *data_row_pos++;
                    out_pos->r = *data_row_pos++;

                    out_pos++;
                }
            }
        }

    }

    kjb_free(data_row);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 write_byte_image
 *
 *
 * Index: images, Byte images, Byte image I/O
 *
 * -----------------------------------------------------------------------------
*/

int write_byte_image(const Byte_image* ip, char* file_name)
{
    char base_name[ MAX_FILE_NAME_SIZE ];
    char suffix[ MAX_FILE_NAME_SIZE ];
    char lc_suffix[ MAX_FILE_NAME_SIZE ];


    if (skip_because_no_overwrite(file_name)) return NO_ERROR;

    ERE(get_image_file_base_path(file_name, base_name, sizeof(base_name),
                                 suffix, sizeof(suffix)));

    EXTENDED_LC_BUFF_CPY(lc_suffix, suffix);

    if (    (suffix[ 0 ] == '\0')
         || (STRCMP_EQ(suffix, "sun"))
         || (STRCMP_EQ(suffix, "ras"))
       )
    {
        return write_raster_byte_image(ip, file_name);
    }
    else
    {
        char temp_file_name[ 1000 ];
        int  result;


        ERE(BUFF_GET_TEMP_FILE_NAME(temp_file_name));
        ERE(write_raster_byte_image(ip, temp_file_name));

        result = convert_image_file_from_raster(temp_file_name, file_name);

        kjb_unlink(temp_file_name);
        return result;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_raster_byte_image(const Byte_image* ip, char* file_name)
{
    FILE*          fp;
    int            i;
    int            j;
    int            row_length;
    Sun_header     sun_header;
    unsigned char  terminator[ 1 ];
    unsigned char* data_row;
    unsigned char* data_row_pos;
    Byte_pixel*    out_pos;


    NRE(fp = kjb_fopen(file_name, "wb"));

    sun_header.magic     = 0x59a66a95;
    sun_header.width     = ip->num_cols;
    sun_header.height    = ip->num_rows;
    sun_header.depth     = 24;
    sun_header.length    = 3 * (ip->num_cols) * (ip->num_rows);
    sun_header.type      = RT_FORMAT_RGB;
    sun_header.maptype   = RMT_NONE;
    sun_header.maplength = 0;

    if (FIELD_WRITE(fp, sun_header) == ERROR)
    {
        (void)kjb_fclose(fp);  /* Ignore return--only reading. */
        return ERROR;
    }

    row_length = 3 * ip->num_cols;
    terminator[ 0 ] = 0;

    if (IS_ODD(row_length)) row_length++;

    data_row = BYTE_MALLOC(row_length);

    if (data_row == NULL)
    {
        (void)kjb_fclose(fp);  /* Ignore return--only reading. */
        return ERROR;
    }

    for(i=0; i<sun_header.height; i++)
    {
        data_row_pos = data_row;

        out_pos = (ip->pixels)[ i ];

        for(j=0; j<ip->num_cols; j++)
        {
            *data_row_pos++ = out_pos->r;
            *data_row_pos++ = out_pos->g;
            *data_row_pos++ = out_pos->b;

            out_pos++;
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
            (void)kjb_fclose(fp);
            set_error_action(save_error_action);
            return ERROR;
        }
    }

    kjb_free(data_row);
    ERE(kjb_fclose(fp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                display_byte_image
 *
 *
 * Index: images, Byte images, Byte image I/O
 *
 * -----------------------------------------------------------------------------
*/

int display_byte_image(const Byte_image* ip, char* title)
{


    return display_any_image((const void*)ip, title,
                             write_byte_image_for_display);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int write_byte_image_for_display(const void* ip, char* title)
{


    return write_byte_image((const Byte_image*)ip, title);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

