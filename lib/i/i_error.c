
/* $Id: i_error.c 4727 2009-11-16 20:53:54Z kobus $ */

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


#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                         check_same_size_image
 *
 * Checks that two images are the same size
 *
 * This routine cheks that two images are the same size, and returns ERROR if
 * they are not, setting an error message to that effect.
 *
 * Returns:
 *     NO_ERROR on sucess and ERROR on failure.
 *
 * Index: images, image error routines
 *
 * -----------------------------------------------------------------------------
*/

int check_same_size_image(const KJB_image* ip1, const KJB_image* ip2)
{


    if (is_same_size_image(ip1, ip2))
    {
        return NO_ERROR;
    }
    else
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         is_same_size_image
 *
 * Predicate function testing if two images arethe same size
 *
 * This routine returns TRUE if the images pointed to by ip1 and ip2 are the
 * same size, and FALSE otherwise.
 *
 * Returns:
 *     TRUE if the images are the same size, and FALSE otherwise.
 *
 * Index: images, image error routines
 *
 * -----------------------------------------------------------------------------
*/

int is_same_size_image(const KJB_image* ip1, const KJB_image* ip2)
{


    if (    (ip1->num_rows == ip2->num_rows)
         && (ip1->num_cols == ip2->num_cols)
       )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

