/* $Id$ */

/* Suspect code. The jpeg writer does not necessarly work on the mac. */

/* =========================================================================== *
   |
   |  Copyright (c) 1994-2010 by Kobus Barnard (author)
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

#include <l/l_incl.h>
#include <i/i_float.h>
#include <i/i_float_io.h>
#include <i/i_transform.h>

int main (void)
{
    KJB_image* image = NULL;
    KJB_image* image_half = NULL;
    KJB_image* image_double = NULL;
    int result = NO_ERROR;

    kjb_init();

    EGC(result = kjb_read_image(&image, "image_tr.jpg"));
    EGC(result = scale_image_size(&image_half, image, 0.5));
    EGC(result = scale_image_size(&image_double, image, 2.0));

    EGC(result = kjb_write_image(image_half, "image_tr_half.jpg"));
    EGC(result = kjb_write_image(image_double, "image_tr_double.jpg"));

cleanup:

    if (result == ERROR) { kjb_print_error(); }

    kjb_free_image(image);
    kjb_free_image(image_half);
    kjb_free_image(image_double);


    return EXIT_SUCCESS;
}
