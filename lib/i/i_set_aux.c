
/* $Id: i_set_aux.c 5831 2010-05-02 21:52:24Z ksimek $ */

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
#include "i/i_set_aux.h"


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static void (*fs_image_data_invalidatation_fn)(void) = NULL;

/* -------------------------------------------------------------------------- */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int set_image_data_invalidation_fn(void (*fn) (void))
{
    fs_image_data_invalidatation_fn = fn;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

void call_image_data_invalidation_fn(void)
{
    if (fs_image_data_invalidatation_fn != NULL)
    {
        (*fs_image_data_invalidatation_fn)();
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int string_scan_float_rgb(const char* value, Pixel* float_pixel_ptr)
{
    Pixel  local_float_pixel;
    char   value_copy[ 200 ];
    char   pixel_val_buff[ 100 ];
    char*  input_pos;
    double temp_real;


    BUFF_CPY(value_copy, value);
    input_pos = value_copy;

    BUFF_GEN_GET_TOKEN(&input_pos, pixel_val_buff, " ,");

    ERE(ss1d(pixel_val_buff, &temp_real));
    if (temp_real > 255.0) temp_real = 255.0;

    /* If there is only one value, then we are going to be gray. */
    local_float_pixel.r = temp_real;
    local_float_pixel.g = temp_real;
    local_float_pixel.b = temp_real;
    local_float_pixel.extra.invalid.r = VALID_PIXEL;
    local_float_pixel.extra.invalid.g = VALID_PIXEL;
    local_float_pixel.extra.invalid.b = VALID_PIXEL;
    local_float_pixel.extra.invalid.pixel = VALID_PIXEL;

    if (BUFF_GEN_GET_TOKEN_OK(&input_pos, pixel_val_buff, " ,"))
    {
        ERE(ss1d(pixel_val_buff, &temp_real));
        if (temp_real > 255.0) temp_real = 255.0;

        local_float_pixel.g = temp_real;

        if (BUFF_GEN_GET_TOKEN_OK(&input_pos, pixel_val_buff, " ,"))
        {
            ERE(ss1d(pixel_val_buff, &temp_real));
            if (temp_real > 255.0) temp_real = 255.0;

            local_float_pixel.b = temp_real;
        }
    }

    *float_pixel_ptr = local_float_pixel;

    return NO_ERROR;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

