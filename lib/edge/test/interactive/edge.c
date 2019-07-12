/**
 * This work is licensed under a Creative Commons 
 * Attribution-Noncommercial-Share Alike 3.0 United States License.
 * 
 *    http://creativecommons.org/licenses/by-nc-sa/3.0/us/
 * 
 * You are free:
 * 
 *    to Share - to copy, distribute, display, and perform the work
 *    to Remix - to make derivative works
 * 
 * Under the following conditions:
 * 
 *    Attribution. You must attribute the work in the manner specified by the
 *    author or licensor (but not in any way that suggests that they endorse you
 *    or your use of the work).
 * 
 *    Noncommercial. You may not use this work for commercial purposes.
 * 
 *    Share Alike. If you alter, transform, or build upon this work, you may
 *    distribute the resulting work only under the same or similar license to
 *    this one.
 * 
 * For any reuse or distribution, you must make clear to others the license
 * terms of this work. The best way to do this is with a link to this web page.
 * 
 * Any of the above conditions can be waived if you get permission from the
 * copyright holder.
 * 
 * Apart from the remix rights granted under this license, nothing in this
 * license impairs or restricts the author's moral rights.
 */


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <inttypes.h>

#include "l/l_incl.h"
#include "m/m_vector.h"
#include "m/m_matrix.h"
#include "i/i_matrix.h"
#include "i/i_float.h"
#include "i/i_float_io.h"
#include "i/i_draw.h"
#include "edge/edge_incl.h"

int main(int argc, char** argv)
{
    float    sigma;
    float    begin_mag_thresh;
    float    end_mag_thresh;
    float    dp_thresh;

    const char* fname_in;
    const char* fname_out;

    KJB_image*  img = NULL;
    Matrix*     m   = NULL;
    Edge_set*   edges = NULL;
    Edge*       edge_p = NULL;
    Pixel       pxl = {0, 0, 0, {{0, 0, 0, 0}}};
    double      fit_error = 0.0;
    uint32_t    i;
    Line_segment_s* line = NULL;
    double start_x, start_y, end_x, end_y;

    kjb_init();

    /* Not ready to do anything in batch mode.  */
    if (! is_interactive()) 
    {
        p_stderr("This test program needs to be adjusted for batch testing.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    assert(argc == 3);
    fname_in  = argv[ 1 ];
    fname_out = argv[ 2 ];


    sigma = 1.0;
   /* begin_mag_thresh = 0.10 * 255;
    end_mag_thresh   = 0.05 * 255;
    dp_thresh        = 0.95 * 255;
    */
    begin_mag_thresh = 0.01 * 255;
    end_mag_thresh   = 0.008 * 255;
    dp_thresh        = 0.95 * 255;

    EPETE(kjb_read_image_2(&img, fname_in));

   /* EPETE(image_to_matrix_2(img, 1.0, 1.0, 1.0, &m)); */
    EPETE(image_to_matrix_2(img, 0.3, 0.59, 0.11, &m));
    EPETE(detect_matrix_edge_set(&edges, m, sigma, begin_mag_thresh,
                    end_mag_thresh, 200, 0, 0));
    break_edges_at_corners(edges,0.7,4);
    break_edges_at_corners(edges,1,50);
    remove_short_edges(edges,20);
    
    /*sample_edge_set(&edges, edges, 0.9);*/
    /* test the fit_line_segment_to_edge_by_least_square */
    edge_p = edges->edges;
    printf("Number of edges is: %d\n", edges->num_edges);
    for (i = 0; i < edges->num_edges; i++)
    {
        assert(line = TYPE_MALLOC(Line_segment_s));
        fit_error = fit_line_segment_to_edge_by_least_square(line, edge_p);
        if(fit_error < 1) 
        {                     
            compute_extrema(&start_x, &start_y, &end_x, &end_y, line);
            printf("draw\n");
            image_draw_segment_2(img, start_y, start_x,end_y, end_x, 0.5, 255.0,0,0); 
        }
            
    /*    printf("%d's error = %f\n", i, fit_error);*/
        edge_p++;
        kjb_free(line);
    }
    pxl.r = 255.0;
   /* EPETE(color_edge_set(&img, img, edges, &pxl));*/

    EPETE(kjb_write_image(img, fname_out));

    free_matrix(m);
    kjb_free_image(img);
    free_edge_set(edges);

    kjb_cleanup();

    return EXIT_SUCCESS;
}
