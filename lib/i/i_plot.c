
/* $Id: i_plot.c 4727 2009-11-16 20:53:54Z kobus $ */

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
#include "i/i_plot.h"
#include "i/i_draw.h"
#include "p/p_plot.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

int write_plot_image(int plot_num, const char* file_name, int reduction)
{
    KJB_image*       ip         = NULL;
    KJB_image*       reduced_ip = NULL;
    const KJB_image* output_ip;
    int              result     = NO_ERROR;


    ERE(make_image_from_plot(&ip, plot_num));

    if (reduction > 1)
    {
        result = ave_image(&reduced_ip, ip, reduction, reduction);
        output_ip = reduced_ip;
    }
    else
    {
        output_ip = ip;
    }

    if (result != ERROR)
    {
        result = kjb_write_image(ip, file_name);
    }

    kjb_free_image(ip);
    kjb_free_image(reduced_ip);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_plot_image(KJB_image** ipp, int plot_num, int reduction)
{
    KJB_image* ip     = NULL;
    int        result = NO_ERROR;


    if (reduction > 1)
    {
        ERE(make_image_from_plot(&ip, plot_num));

        result = ave_image(ipp, ip, reduction, reduction);
    }
    else
    {
        ERE(make_image_from_plot(ipp, plot_num));
    }

    kjb_free_image(ip);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int make_image_from_plot(KJB_image** ipp, int plot_num)
{
    char temp_plot_file_name[ MAX_FILE_NAME_SIZE ];
    int result;
    Error_action save_error_action = get_error_action();

    BUFF_GET_TEMP_FILE_NAME(temp_plot_file_name);
    BUFF_CAT(temp_plot_file_name, ".pbm");
    ERE(save_plot_as_pbm(plot_num, temp_plot_file_name));

    result = kjb_read_image_2(ipp, temp_plot_file_name);

#ifdef IMPLEMENT_REVERSING_COLORS
    /*
     *  When we need this is hard to figure out. Perhaps an option is needed.
    */

    if (result != ERROR)
    {
        KJB_image* ip = *ipp;
        int i;
        int j;
        int num_rows = (*ipp)->num_rows;
        int num_cols = (*ipp)->num_cols;
        double r, g, b;

        for (i = 0; i < num_rows; i++)
        {
            Pixel* ip_pos = (*ipp)->pixels[ i ];

            for (j = 0; j < num_cols; j++)
            {
                r = ip_pos->r;
                g = ip_pos->g;
                b = ip_pos->b;

#ifdef DEF_OUT
                if (  ( ! ((r < 0.1) && (g < 0.1) && (b < 0.1)))
                    &&(! ((r > 250.0) && (g > 250.0) && (b > 250.0)))
                   )

                {
                    ip_pos->r = 255.0 - ip_pos->r;
                    ip_pos->g = 255.0 - ip_pos->g;
                    ip_pos->b = 255.0 - ip_pos->b;
                }
#endif

/*
#define INVERT
*/

#ifdef INVERT
                /* Is it black? Make it white. */
                if ((r < 0.1) && (g < 0.1) && (b < 0.1))
                {
                    (*ipp)->pixels[i][j].r = 255.0;
                    (*ipp)->pixels[i][j].g = 255.0;
                    (*ipp)->pixels[i][j].b = 255.0;
                }
                /* Is it while? Make it black. */
                else if ((r > 250.0) && (g > 250.0) && (b > 250.0))
                {
                    (*ipp)->pixels[i][j].r = 0.0;
                    (*ipp)->pixels[i][j].g = 0.0;
                    (*ipp)->pixels[i][j].b = 0.0;
                }
#endif

                ip_pos++;
            }
        }
    }
#endif

    if (result == ERROR)
    {
        set_error_action(FORCE_ADD_ERROR_ON_ERROR);
    }

    /*
    // If we can't remove the file, then we want to report this, but if we are
    // about to return error, the message will be attached to the more critical
    // error message.
    */
    if (kjb_unlink(temp_plot_file_name) == ERROR)
    {
        if (result != ERROR)
        {
            kjb_print_error();
        }
    }

    set_error_action(save_error_action);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

