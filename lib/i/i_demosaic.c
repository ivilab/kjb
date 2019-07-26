
/* $Id: i_demosaic.c 8780 2011-02-27 23:42:02Z predoehl $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author). (Lindsay Martin
|  contributed the interface to his hdrc code).
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

#include "i/i_demosaic.h"
#include "i/i_hdrc.h"      /* Lindsay */

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

typedef enum Demosaic_method
{
    DONT_DEMOSAIC,
    DUMB_DEMOSAIC,
    HDRC_DEMOSAIC        /* Lindsay */
}
Demosaic_method;

/* -------------------------------------------------------------------------- */

static Demosaic_method fs_demosaic_method = DONT_DEMOSAIC;

/* -------------------------------------------------------------------------- */

int set_demosaic_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result             = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if ((lc_option[ 0 ] == '\0') || match_pattern(lc_option, "demosaic"))
    {
        const char* str;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '?')
        {
            if      (fs_demosaic_method == DONT_DEMOSAIC) str = "none";
            else if (fs_demosaic_method == DUMB_DEMOSAIC) str = "dumb";
            else if (fs_demosaic_method == HDRC_DEMOSAIC) str = "hdrc"; /*Lindsay*/
            else
            {
                SET_CANT_HAPPEN_BUG();
                return ERROR;
            }

            ERE(pso("demosaic-method = %s\n", str));
        }
        else if (value[ 0 ] == '\0')
        {
            if      (fs_demosaic_method == DONT_DEMOSAIC) str = "none";
            else if (fs_demosaic_method == DUMB_DEMOSAIC) str = "dumb";
            else if (fs_demosaic_method == HDRC_DEMOSAIC) str = "hdrc";/*Lindsay*/
            else
            {
                SET_CANT_HAPPEN_BUG();
                return ERROR;
            }

            ERE(pso("Domosaic method is %s.\n", str));
        }
        else
        {
            if (STRCMP_EQ(value, "none"))
            {
                fs_demosaic_method = DONT_DEMOSAIC;
            }
            else if (STRCMP_EQ(value, "dumb"))
            {
                fs_demosaic_method = DUMB_DEMOSAIC;
            }
            else if (STRCMP_EQ(value, "hdrc"))
            {
                fs_demosaic_method = HDRC_DEMOSAIC;
            }
            else
            {
                set_error("%q is an invalid demosaic method designator.", value);
                return ERROR;
            }
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              ow_demosaic
 *
 * Demosaics an image using the method currently set
 *
 * This routine demosaics an image using the method currently set. The image is
 * overwritten with the result.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index:
 *    images, image demosaicing
 *
 * -----------------------------------------------------------------------------
*/

int ow_demosaic(KJB_image* ip)
{
    if (fs_demosaic_method == DONT_DEMOSAIC)
    {
        return NO_ERROR;
    }
    else if (fs_demosaic_method == DUMB_DEMOSAIC)
    {
        return ow_dumb_demosaic(ip);
    }
    /* Lindsay - Nov 7, 1999 */
    else if (fs_demosaic_method == HDRC_DEMOSAIC)
    {
        return ow_hdrc_demosaic(ip);
    }
    /* End Lindsay - Nov 7, 1999 */
    else
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    /*NOTREACHED*/
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                              ow_dumb_demosaic
 *
 * Demosaics an image in a really dumb way
 *
 * This routine demosaics an image in a really dumb way. The image is
 * overwritten with the result.
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure, with an error message being set.
 *
 * Index:
 *    images, image demosaicing
 *
 * -----------------------------------------------------------------------------
*/

int ow_dumb_demosaic(KJB_image* ip)
{
    int   num_rows, num_cols, i, j, m, n;
    float r, g1, g2, b;


    verbose_pso(10, "Demosaicing the image with a dumb algorithm.\n");

    num_rows = ip->num_rows / 2;
    num_cols = ip->num_cols / 2;

    for (i=0; i<num_rows; i++)
    {
        for (j=0; j<num_cols; j++)
        {
            r = ip->pixels[ 2*i + 1 ][ 2*j + 1 ].r;  /* Could be any of R,G,B */
            g1 = ip->pixels[ 2*i + 1 ][ 2*j  ].g;    /* Could be any of R,G,B */
            g2 = ip->pixels[ 2*i ][ 2*j + 1].g;      /* Could be any of R,G,B */
            b = ip->pixels[ 2*i ][ 2*j ].b;          /* Could be any of R,G,B */

            for (m=0; m<2; m++)
            {
                for (n=0; n<2; n++)
                {
                    ip->pixels[ 2*i + m ][ 2*j + n ].r = r;
                    ip->pixels[ 2*i + m ][ 2*j + n ].g = (g1 + g2)/2;
                    ip->pixels[ 2*i + m ][ 2*j + n ].b = b;
                }
            }
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

