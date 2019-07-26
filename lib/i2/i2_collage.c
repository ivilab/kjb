
/* $Id: i2_collage.c 21448 2017-06-28 22:00:33Z kobus $ */

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
#include "i2/i2_collage.h"
#include "i2/i2_draw_text.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/*
 * Obsolete, but currently used by "it". This is a dumb routine. We need to decide what functionality is
 * really needed by "it", and modify or get rid of this one.
 */

int ip_output_montage
(
    const char* output_file,
    int         num_images,
    int         num_montage_rows,
    int         num_montage_cols,
    KJB_image** images,
    char        (*labels)[ MAX_MONTAGE_IMAGE_LABEL_SIZE ],
    const char* extra
)
{
    int i, num_rows, num_cols;
    int max_num_rows = 0;
    int max_num_cols = 0;
    char geometry_str[ 100 ];
    char temp_names[ MAX_NUM_MONTAGE_IMAGES ][ MAX_FILE_NAME_SIZE ];
    int  num_images_written = 0;
    int  result  = NO_ERROR;
    int  max_dim;
    int  count   = 0;
    KJB_image* label_ip = NULL;
    KJB_image* image_with_label_ip = NULL;
    int num_label_rows;


    if (num_images > MAX_NUM_MONTAGE_IMAGES)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    for (i = 0; i<num_images; i++)
    {
        if (images[ i ] != NULL)
        {
            num_rows = images[ i ]->num_rows;
            num_cols = images[ i ]->num_cols;

            if (num_rows > max_num_rows) max_num_rows = num_rows;
            if (num_cols > max_num_cols) max_num_cols = num_cols;

            count++;

            if (count >= num_montage_rows * num_montage_cols) break;
        }
    }

    max_dim = MAX_OF(max_num_rows, max_num_cols);

    ERE(kjb_sprintf(geometry_str, sizeof(geometry_str), "%dx%d",
                    max_dim, max_dim));

    for (i = 0; i<num_images; i++)
    {
        if (images[ i ] != NULL)
        {
            ERE(BUFF_GET_TEMP_FILE_NAME(temp_names[ num_images_written ]));

            if ((labels != NULL) && (labels[ i ] != NULL) && (labels[ i ][ 0 ] != '\0'))
            {
                num_rows = images[ i ]->num_rows;
                num_cols = images[ i ]->num_cols;

                result = get_wrapped_text_block_image(&label_ip, labels[ i ],
                                                      num_cols - 10,
                                                      "times14");
                if (result == ERROR) break;

                num_label_rows = label_ip->num_rows;

                result = get_initialized_image_2(&image_with_label_ip,
                                                 num_rows + num_label_rows,
                                                 num_cols, 255, 255, 255);
                if (result == ERROR) break;

                result = image_draw_image(image_with_label_ip, images[ i ],
                                          0, 0, 1);
                if (result == ERROR) break;

                result = image_draw_image(image_with_label_ip, label_ip,
                                          num_rows, 5, 1);
                if (result == ERROR) break;

                result = kjb_write_image(image_with_label_ip,
                                         temp_names[ num_images_written ]);
            }
            else
            {
                result = kjb_write_image(images[ i ],
                                         temp_names[ num_images_written ]);
            }

            if (result == ERROR) break;

            num_images_written++;

            if (num_images_written >= num_montage_rows * num_montage_cols)
            {
                break;
            }
        }
    }

    if ((result != ERROR) && (num_images_written > 0))
    {
        result = output_montage_2(output_file, geometry_str, num_images_written,
                                  num_montage_rows, num_montage_cols,
                                  temp_names, NULL, extra);
    }

    for (i = 0; i<num_images_written; i++)
    {
        EPE(kjb_unlink(temp_names[ i ]));
    }

    kjb_free_image(label_ip);
    kjb_free_image(image_with_label_ip);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int make_image_collage_with_labels
(
    KJB_image** out_ipp,
    int         num_horizontal,
    int         num_vertical,
    KJB_image** ip_list,
    char**      labels
)
{
    int num_images = num_horizontal * num_vertical;
    int i, num_rows, num_cols;
    char temp_name[ MAX_FILE_NAME_SIZE ];
    int  result  = NO_ERROR;
    KJB_image* label_ip = NULL;
    int num_label_rows;
    KJB_image** labeled_ip_list;

    NRE(labeled_ip_list = N_TYPE_MALLOC(KJB_image*, num_images));

    for (i = 0; i < num_images; i++)
    {
        labeled_ip_list[ i ] = NULL;
    }

    for (i = 0; i<num_images; i++)
    {
        if (ip_list[ i ] != NULL)
        {
            ERE(BUFF_GET_TEMP_FILE_NAME(temp_name));

            if ((labels != NULL) && (labels[ i ] != NULL) && (labels[ i ][ 0 ] != '\0'))
            {
                num_rows = ip_list[ i ]->num_rows;
                num_cols = ip_list[ i ]->num_cols;

                result = get_wrapped_text_block_image(&label_ip, labels[ i ],
                                                      num_cols - 10,
                                                      "times14");
                if (result == ERROR) break;

                num_label_rows = label_ip->num_rows;

                result = get_initialized_image_2(&(labeled_ip_list[ i ]),
                                                 num_rows + num_label_rows,
                                                 num_cols, 255, 255, 255);
                if (result == ERROR) break;

                result = image_draw_image(labeled_ip_list[ i ], ip_list[ i ],
                                          0, 0, 1);
                if (result == ERROR) break;

                result = image_draw_image(labeled_ip_list[ i ], label_ip,
                                          num_rows, 5, 1);
            }
            else
            {
                result = kjb_copy_image(&(labeled_ip_list[ i ]), ip_list[ i ]);
            }

            if (result == ERROR) break;
        }
    }

    if (result != ERROR)
    {
        result = make_image_collage(out_ipp, num_horizontal, num_vertical,
                                    (const KJB_image* const*) labeled_ip_list);
    }

    kjb_free_image(label_ip);

    for (i = 0; i < num_images; i++)
    {
        kjb_free_image(labeled_ip_list[ i ]);
    }

    kjb_free(labeled_ip_list);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

