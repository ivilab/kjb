
/* $Id: curv_cut.c 21545 2017-07-23 21:57:31Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2003-2008 by members of University of Arizona Computer Vision     |
|  group (the authors) including                                               |
|        Kobus Barnard                                                         |
|        Amy Platt                                                             |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */

/*
 * The cutting implemented here is a bit more complex than needed due to
 * thinking about cutting as removing a pixel. Of course, cutting is removing
 * the connection between pixels. Thus we needed to invent the concept of a
 * terminating pixel which the routine get_all_paths() includes as parts of
 * paths, but not go beyond.
 *
 * This can likely be done much better with the following rewrite: Use an
 * integer matrix matrix structure to record connections between pixels which is
 * used by get_all_paths(). The pixel matrices are 3 by 3, and are null if there
 * is no pixel. Otherwise, if the pixel is on, then the middle entry is set
 * (connected with self)
*/

#include "i/i_incl.h"
#include "i2/i2_incl.h"
#include "m/m_incl.h"
#include "n/n_invert.h"
#include "nr/nr_roots.h"
#include "cgi_draw/cgi_draw.h"
#include "curv/curv_gen.h"
#include "curv/curv_all_paths.h"
#include "curv/curv_curvature.h"
#include "curv/curv_cut.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#define JUNCTION_SIZE 2

#define MAX_NUM_DEEP_PATHS 1
#define DEPTH_FIRST_DEPTH 5

#define MAX_NUM_PATHS  (ipower(9,DEPTH_FIRST_DEPTH) * MAX_NUM_DEEP_PATHS)

#define MIN_BUFFER_LENGTH 8
#define MAX_CUT_PATH_LENGTH  16

#define CURVE_IMAGE_RESOLUTION   300

#define BALANCE_WEIGHT

#define ITERATIVE_CUTTING


/*
*/
#define COMMIT_TO_ENTIRE_PATH

static int fs_commit_size = MAX_CUT_PATH_LENGTH / 2;

/*
#define ITERATIVE_FIT
*/

/* -------------------------------------------------------------------------- */

static int cut_it_up
(
    const KJB_image*   debug_ip,
    int                max_num_paths,
    int                min_len_for_stopping_at_junctions,
    int                max_path_len,
    const Int_matrix*  on_mp,
    const Int_matrix*  term_mp,
    const Matrix*      curvature_mp,
    Int_matrix_vector* i_mvp,
    Int_matrix_vector* j_mvp,
    Matrix_vector*     t_mvp,
    Int_vector_vector* len_vvp,
    int                i,
    int                j,
    int*               dir_1_ptr,
    int*               dir_2_ptr,
    int*               path_1_ptr,
    int*               path_2_ptr,
    double*            quality_ptr
);

static int count_neighbors_without_aligned_neighbors(const Int_matrix* image_mp, int i, int j);

static int pixel_on_path
(
    int               i,
    int               j,
    const Int_vector* path_i_vp,
    const Int_vector* path_j_vp
);

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* -------------------------------------------------------------------------- */

int cut_neuron
(
    KJB_image**  cut_ipp,
    const KJB_image*  ip
)
{
    int             dir;
    int             i;
    int             j;
    KJB_image*      save_debug_ip     = NULL;
    KJB_image*      debug_ip          = NULL;
    KJB_image*      debug_2_ip        = NULL;
    KJB_image*      big_debug_ip      = NULL;
    KJB_image*      big_debug_2_ip      = NULL;
    KJB_image*      middle_debug_ip   = NULL;
    Int_matrix*     cut_ij_dir_mp     = NULL;
    int             dir_1 = NOT_SET;
    int             dir_2 = NOT_SET;
    Indexed_vector* sorted_quality_vp = NULL;
    int             count;
    int             cut_count         = 0;
    int             real_cut_count    = 0;
    double          quality           = DBL_NOT_SET;
    int num_rows = ip->num_rows;
    int num_cols = ip->num_cols;
    Int_matrix* on_mp = NULL;
    Int_matrix* cut_mp = NULL;
    Int_matrix* term_mp = NULL;
    Int_matrix* precious_mp = NULL;
    Int_matrix_vector* i_mvp = NULL;
    Int_matrix_vector* j_mvp = NULL;
    Matrix_vector* t_mvp = NULL;
    Int_vector_vector* len_vvp = NULL;
    Int_vector_vector* cut_paths_1_i_vvp = NULL;
    Int_vector_vector* cut_paths_1_j_vvp = NULL;
    Int_vector_vector* cut_paths_2_i_vvp = NULL;
    Int_vector_vector* cut_paths_2_j_vvp = NULL;
    int max_path_length;
    int path_1 = NOT_SET, path_2 = NOT_SET;
    int loops = 0;
    const int scale = 4;
    Matrix* curvature_mp = NULL;
    int pass;
    int max_path_subset_len;


    EPETE(kjb_copy_image(cut_ipp, ip));

    START_HEAP_CHECK_2(); 

    ERE(image_curvature(ip, &curvature_mp, 0, NULL, NULL));

    EPETE(get_zero_int_matrix(&on_mp, num_rows, num_cols));
    EPETE(get_zero_int_matrix(&term_mp, num_rows, num_cols));
    EPETE(get_zero_int_matrix(&precious_mp, num_rows, num_cols));
    EPETE(get_zero_int_matrix(&cut_mp, num_rows, num_cols));

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (IS_NEURON_PIXEL(ip, i, j ))
            {
                on_mp->elements[ i ][ j ] = TRUE;
            }
        }
    }


    EPETE(kjb_copy_image(&debug_ip, ip));

    EPETE(ow_invert_image(debug_ip));

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            if (IS_NEURON_PIXEL(ip, i, j))
            {
                if (on_mp->elements[ i ][ j ] == 0)
                {
                    debug_ip->pixels[ i ][ j ].r = 255.0;
                    debug_ip->pixels[ i ][ j ].g = 0.0;
                    debug_ip->pixels[ i ][ j ].b = 0.0;
                }
                else
                {
                    on_mp->elements[ i ][ j ] = count_neighbors_without_aligned_neighbors(on_mp, i, j);

                    if (on_mp->elements[ i ][ j ] <= 2)
                    {
                        debug_ip->pixels[ i ][ j ].r = MAX_OF(0, 1000 * curvature_mp->elements[ i ][ j ]);
                        debug_ip->pixels[ i ][ j ].g = 0.0;
                        debug_ip->pixels[ i ][ j ].b = 0.0;
                    }
                    else
                    {
                        debug_ip->pixels[ i ][ j ].r = 0.0;
                        debug_ip->pixels[ i ][ j ].g = 0.0;
                        debug_ip->pixels[ i ][ j ].b = 255.0;
                    }
                }
            }
        }
    }

    if ((i_mvp == NULL) || (j_mvp == NULL) || (t_mvp == NULL) || (len_vvp == NULL))
    {
        EPETE(get_target_int_matrix_vector(&i_mvp, CURV_NUM_DIRECTIONS));
        EPETE(get_target_int_matrix_vector(&j_mvp, CURV_NUM_DIRECTIONS));
        EPETE(get_target_matrix_vector(&t_mvp, CURV_NUM_DIRECTIONS));
        EPETE(get_target_int_vector_vector(&len_vvp, CURV_NUM_DIRECTIONS));
    }

    EPETE(kjb_copy_image(&save_debug_ip, debug_ip));

    dbi(scale);
    max_path_length = (MAX_CUT_PATH_LENGTH * scale) / 2;
    dbi(max_path_length);

    for (dir = 0; dir < CURV_NUM_DIRECTIONS; dir++)
    {
        EPETE(get_target_int_matrix(&(i_mvp->elements[ dir ]),
                                    MAX_NUM_PATHS, max_path_length));
        EPETE(get_target_int_matrix(&(j_mvp->elements[ dir ]),
                                    MAX_NUM_PATHS, max_path_length));
        EPETE(get_target_matrix(&(t_mvp->elements[ dir ]),
                                MAX_NUM_PATHS, max_path_length));
        EPETE(get_target_int_vector(&(len_vvp->elements[ dir ]), MAX_NUM_PATHS));
    }

    /*
     * First pass: "nice" junctions with at least MIN_BUFFER_LENGTH on each side.
     * Second pass, allow less nice junctions. Third pass, allow very short path
     * pieces (this is necessary for cleaning up junctions that are split in a
     * certain way.
    */
    for (pass = 0; pass < 3; pass++)
    {
        dbi(pass);

        while (TRUE)
        {
            cut_count = 0;
            real_cut_count = 0;

            ERE(get_target_int_matrix(&cut_ij_dir_mp, num_rows * num_cols, 4));
            ERE(get_target_indexed_vector(&sorted_quality_vp, num_rows * num_cols));
            ERE(get_target_int_vector_vector(&cut_paths_1_i_vvp, num_rows * num_cols));
            ERE(get_target_int_vector_vector(&cut_paths_1_j_vvp, num_rows * num_cols));
            ERE(get_target_int_vector_vector(&cut_paths_2_i_vvp, num_rows * num_cols));
            ERE(get_target_int_vector_vector(&cut_paths_2_j_vvp, num_rows * num_cols));

            for (i = 0; i < num_rows; i++)
            {
                for (j = 0 ; j < num_cols; j++)
                {
                    if (    (on_mp->elements[ i ][ j ])
                         && ( ! cut_mp->elements[ i ][ j ])
                         && ( ! term_mp->elements[ i ][ j ])
                         && ( ! precious_mp->elements[ i ][ j ])
                         && (    (    (count_neighbors_without_aligned_neighbors(on_mp, i, j) > 2)
                                   && (pass == 0)
                                 )
                              ||
                                 (    (count_neighbors(on_mp, i, j) > 2)
                                   && (pass > 0)
                                 )
                           )
                       )
                    {
                        int num_paths;

                        ERE(num_paths = cut_it_up(save_debug_ip,
                                                  MAX_NUM_PATHS,
                                                  MIN_BUFFER_LENGTH,
                                                  max_path_length,
                                                  on_mp, term_mp, curvature_mp,
                                                  i_mvp, j_mvp, t_mvp, len_vvp,
                                                  i, j, &dir_1, &dir_2, &path_1, &path_2,
                                                  &quality));

                        if (    (num_paths > 0)
                             && (    (     (len_vvp->elements[ dir_1 ]->elements[ path_1 ] > MIN_BUFFER_LENGTH)
                                        && (len_vvp->elements[ dir_2 ]->elements[ path_2 ] > MIN_BUFFER_LENGTH)
                                     )
                                  ||
                                     (pass > 1)
                                 )
                            )
                        {
                            cut_ij_dir_mp ->elements[ cut_count ][ 0 ] = i;
                            cut_ij_dir_mp ->elements[ cut_count ][ 1 ] = j;
                            cut_ij_dir_mp ->elements[ cut_count ][ 2 ] = dir_1;
                            cut_ij_dir_mp ->elements[ cut_count ][ 3 ] = dir_2;

                            ERE(get_int_matrix_row(&(cut_paths_1_i_vvp->elements[ cut_count ]),
                                                   i_mvp->elements[ dir_1 ], path_1));
                            cut_paths_1_i_vvp->elements[ cut_count ]->length = len_vvp->elements[ dir_1 ]->elements[ path_1 ];

                            ERE(get_int_matrix_row(&(cut_paths_1_j_vvp->elements[ cut_count ]),
                                                   j_mvp->elements[ dir_1 ], path_1));
                            cut_paths_1_j_vvp->elements[ cut_count ]->length = len_vvp->elements[ dir_1 ]->elements[ path_1 ];

                            ERE(get_int_matrix_row(&(cut_paths_2_i_vvp->elements[ cut_count ]),
                                                   i_mvp->elements[ dir_2 ], path_2));
                            cut_paths_2_i_vvp->elements[ cut_count ]->length = len_vvp->elements[ dir_2 ]->elements[ path_2 ];

                            ERE(get_int_matrix_row(&(cut_paths_2_j_vvp->elements[ cut_count ]),
                                                   j_mvp->elements[ dir_2 ], path_2));
                            cut_paths_2_j_vvp->elements[ cut_count ]->length = len_vvp->elements[ dir_2 ]->elements[ path_2 ];

                            sorted_quality_vp->elements[ cut_count ].element = quality;
                            cut_count++;
                        }
                    }
                }
            }

            dbi(scale);
            dbi(cut_count);

            if (cut_count > 0)
            {
                sorted_quality_vp->length = cut_count;
                ERE(ascend_sort_indexed_vector(sorted_quality_vp));

                for (count = 0; count < cut_count; count++)
                {
                    int index = sorted_quality_vp->elements[ count ].index;
                    double quality = sorted_quality_vp->elements[ count ].element;

                    i = cut_ij_dir_mp->elements[ index ][ 0 ];
                    j = cut_ij_dir_mp->elements[ index ][ 1 ];
                    dir_1 = cut_ij_dir_mp ->elements[ index ][ 2 ];
                    dir_2 = cut_ij_dir_mp ->elements[ index ][ 3 ];

                    if (quality >= 0.0)
                    {
                        int c2;
                        int k;
                        int path;

                        debug_ip->pixels[ i ][ j ].r = 0.0;
                        debug_ip->pixels[ i ][ j ].g = 255.0;
                        debug_ip->pixels[ i ][ j ].b = 0.0;

                        if (    (! precious_mp->elements[ i + curv_neighbors[ dir_1 ][ 0 ] ][ j + curv_neighbors[ dir_1 ][ 1 ] ])
                             && (! term_mp->elements[ i + curv_neighbors[ dir_1 ][ 0 ] ][ j + curv_neighbors[ dir_1 ][ 1 ] ])
                           )
                        {
                            debug_ip->pixels[ i + curv_neighbors[ dir_1 ][ 0 ] ][ j + curv_neighbors[ dir_1 ][ 1 ] ].r = 0.0;
                            debug_ip->pixels[ i + curv_neighbors[ dir_1 ][ 0 ] ][ j + curv_neighbors[ dir_1 ][ 1 ] ].g = 250.0;
                            debug_ip->pixels[ i + curv_neighbors[ dir_1 ][ 0 ] ][ j + curv_neighbors[ dir_1 ][ 1 ] ].b = 250.0;
                        }

                        if (    (! precious_mp->elements[ i + curv_neighbors[ dir_2 ][ 0 ] ][ j + curv_neighbors[ dir_2 ][ 1 ] ])
                             && (! term_mp->elements[ i + curv_neighbors[ dir_2 ][ 0 ] ][ j + curv_neighbors[ dir_2 ][ 1 ] ])
                           )
                        {
                            debug_ip->pixels[ i + curv_neighbors[ dir_2 ][ 0 ] ][ j + curv_neighbors[ dir_2 ][ 1 ] ].r = 0.0;
                            debug_ip->pixels[ i + curv_neighbors[ dir_2 ][ 0 ] ][ j + curv_neighbors[ dir_2 ][ 1 ] ].g = 250.0;
                            debug_ip->pixels[ i + curv_neighbors[ dir_2 ][ 0 ] ][ j + curv_neighbors[ dir_2 ][ 1 ] ].b = 250.0;
                        }

                        /*
                         * (path == 1) first path, otherwise second path.
                        */
                        for (path = 1; path <= 2; path++)
                        {
                            const Int_vector* cut_paths_i_vp = (path == 1) ? cut_paths_1_i_vvp->elements[ index ] : cut_paths_2_i_vvp->elements[ index ];
                            const Int_vector* cut_paths_j_vp = (path == 1) ? cut_paths_1_j_vvp->elements[ index ] : cut_paths_2_j_vvp->elements[ index ];
                            int path_len = cut_paths_i_vp->length;

                            ASSERT(cut_paths_j_vp->length == path_len);

                            /*
                             * Follow the path in each direction.  Mark the
                             * pixels on that part of the path as precious.
                             * Pixels touching the path are marked as
                             * terminators. We cannot do this too close to the
                             * end of the path, hence the (path_len - 4) instead
                             * of path_len.
                            */

#ifdef COMMIT_TO_ENTIRE_PATH
                            max_path_subset_len = MIN_OF(path_len - 4, fs_commit_size);
#else
                            max_path_subset_len = MIN_OF(path_len - 4, 3);
#endif

                            for (k = 0; k < max_path_subset_len; k++)
                            {
                                int ii = cut_paths_i_vp->elements[ k ];
                                int jj = cut_paths_j_vp->elements[ k ];
                                int min_iii = MAX_OF(0, ii - 1);
                                int min_jjj = MAX_OF(0, jj - 1);
                                int max_iii = MIN_OF(ii + 1, num_rows - 1);
                                int max_jjj = MIN_OF(jj + 1, num_cols - 1);
                                int iii, jjj;

                                if (     (k < max_path_subset_len - 1)
                                      && ((ii != i) || (jj != j))
                                      && ( ! term_mp->elements[ ii ][ jj ])
                                    )
                                {
                                    precious_mp->elements[ ii ][ jj ] = TRUE;

                                    debug_ip->pixels[ ii ][ jj ].r = (path == 1) ? 100.0 + k: 0.0;
                                    debug_ip->pixels[ ii ][ jj ].g = (path == 2) ? 100.0 + k: 0.0;
                                    debug_ip->pixels[ ii ][ jj ].b = path_len;
                                }

                                for (iii = min_iii; iii <= max_iii; iii++)
                                {
                                    for (jjj = min_jjj; jjj <= max_jjj; jjj++)
                                    {
                                        if (    ((iii != ii) || (jjj != jj))
                                             && (on_mp->elements[ iii ][ jjj ])
                                             && ( ! pixel_on_path(iii, jjj, cut_paths_1_i_vvp->elements[ index ], cut_paths_1_j_vvp->elements[ index ]))
                                             && ( ! pixel_on_path(iii, jjj, cut_paths_2_i_vvp->elements[ index ], cut_paths_2_j_vvp->elements[ index ]))
                                             && ( ! precious_mp->elements[ iii ][ jjj ])
                                           )
                                        {
                                            term_mp->elements[ iii ][ jjj ] = TRUE;

                                            debug_ip->pixels[ iii ][ jjj ].r = 255.0;
                                            debug_ip->pixels[ iii ][ jjj ].g = path_len;
                                            debug_ip->pixels[ iii ][ jjj ].b = 255.0;
                                        }
                                        else if (    ((iii != ii) || (jjj != jj))
                                                 && (on_mp->elements[ iii ][ jjj ])
                                                 && ( ! pixel_on_path(iii, jjj, cut_paths_1_i_vvp->elements[ index ], cut_paths_1_j_vvp->elements[ index ]))
                                                 && ( ! pixel_on_path(iii, jjj, cut_paths_2_i_vvp->elements[ index ], cut_paths_2_j_vvp->elements[ index ]))
                                               )
                                        {
                                            debug_ip->pixels[ iii ][ jjj ].r = 255.0;
                                            debug_ip->pixels[ iii ][ jjj ].g = path_len;
                                            debug_ip->pixels[ iii ][ jjj ].b = 0.0;
                                        }
                                    }
                                }
                            }
                        }

                        cut_mp->elements[ i ][ j ] = TRUE;
                        precious_mp->elements[ i ][ j ] = TRUE;

                        for (c2 = count + 1; c2 < cut_count; c2++)
                        {
                            int i2 = sorted_quality_vp->elements[ c2 ].index;
                            int ii = cut_ij_dir_mp->elements[ i2 ][ 0 ];
                            int jj = cut_ij_dir_mp->elements[ i2 ][ 1 ];
                            int di = i - ii;
                            int dj = j - jj;

#ifdef ITERATIVE_CUTTING
#ifdef COMMIT_TO_ENTIRE_PATH
                            if (di*di + dj*dj < (fs_commit_size + max_path_length)*(fs_commit_size + max_path_length))
#else
                            if (di*di + dj*dj < max_path_length*max_path_length)
#endif
#else
                            /*
                            if (di*di + dj*dj < 9)
                            */
#endif
                            {
                                sorted_quality_vp->elements[ c2 ].element = DBL_NOT_SET;
                            }
                        }

                        real_cut_count++;
                    }
                }

                ERE(kjb_copy_image(&debug_2_ip, debug_ip));

                for (count = 0; count < cut_count; count++)
                {
                    int index = sorted_quality_vp->elements[ count ].index;
                    double quality = sorted_quality_vp->elements[ count ].element;

                    i = cut_ij_dir_mp->elements[ index ][ 0 ];
                    j = cut_ij_dir_mp->elements[ index ][ 1 ];

                    if (quality >= 0.0)
                    {
                        image_draw_box(debug_2_ip, i, j, 6, 0,
                                       (loops >= 2) ? 200 : 0, (loops == 1) ? 200 : 0, (loops == 0) ? 200 : 0);
                    }
                }
            }

            dbi(real_cut_count);

            if (real_cut_count > 0)
            {
                ERE(get_image_window(&middle_debug_ip, debug_ip, num_rows / 3, num_cols / 3, num_rows / 3, num_cols / 3));
                ERE(magnify_image(&big_debug_ip, middle_debug_ip, 3, 3));

                ERE(get_image_window(&middle_debug_ip, debug_2_ip, num_rows / 3, num_cols / 3, num_rows / 3, num_cols / 3));
                ERE(magnify_image(&big_debug_2_ip, middle_debug_ip, 3, 3));
            }

#ifdef REALLY_TEST
            if (is_interactive())
            {
                /*
                kjb_display_image(big_debug_2_ip, NULL);
                */
                kjb_display_image(debug_2_ip, NULL);
                prompt_to_continue();
            }
#endif 

            if (real_cut_count == 0) break;

            loops++;
        }
    }

    for (i = 0; i < num_rows; i++)
    {
        for (j = 0; j < num_cols; j++)
        {
            /*
            if (term_mp->elements[ i ][ j ])
            */
            if ((term_mp->elements[ i ][ j ]) && (! precious_mp->elements[ i ][ j ]))
            {
                (*cut_ipp)->pixels[ i ][ j ].r = 0.0;
                (*cut_ipp)->pixels[ i ][ j ].g = 250.0;
                (*cut_ipp)->pixels[ i ][ j ].b = 0.0;
            }
            else if (term_mp->elements[ i ][ j ])
            {
                (*cut_ipp)->pixels[ i ][ j ].r = 0.0;
                (*cut_ipp)->pixels[ i ][ j ].g = 0.0;
                (*cut_ipp)->pixels[ i ][ j ].b = 250.0;
            }
        }
    }

    free_matrix(curvature_mp);

    free_int_matrix(cut_ij_dir_mp);
    free_int_matrix(on_mp);
    free_int_matrix(term_mp);
    free_int_matrix(precious_mp);
    free_int_matrix(cut_mp);
    free_int_vector_vector(len_vvp);
    free_int_matrix_vector(i_mvp);
    free_int_matrix_vector(j_mvp);
    free_matrix_vector(t_mvp);
    free_int_vector_vector(cut_paths_1_i_vvp);
    free_int_vector_vector(cut_paths_1_j_vvp);
    free_int_vector_vector(cut_paths_2_i_vvp);
    free_int_vector_vector(cut_paths_2_j_vvp);

    free_indexed_vector(sorted_quality_vp); 

    kjb_free_image(save_debug_ip);
    kjb_free_image(debug_ip);
    kjb_free_image(debug_2_ip);
    kjb_free_image(big_debug_ip);
    kjb_free_image(big_debug_2_ip);
    kjb_free_image(middle_debug_ip);

    FINISH_HEAP_CHECK_2();

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
#define DEBUG_IT_3
#define DEBUG_IT_2
#define DEBUG_IT
#define DEBUG_CHOOSE
*/



#ifdef DEBUG_CHOOSE
    int fs_choose_it;
#endif

static int cut_it_up
(
    const KJB_image*   __attribute__((unused)) debug_ip,
    int                max_num_paths,
    int                min_len_for_stopping_at_junctions,
    int                max_path_len,
    const Int_matrix*  on_mp,
    const Int_matrix*  term_mp,
    const Matrix*      curvature_mp,
    Int_matrix_vector* i_mvp,
    Int_matrix_vector* j_mvp,
    Matrix_vector*     t_mvp,
    Int_vector_vector* len_vvp,
    int                i,
    int                j,
    int*               dir_1_ptr,
    int*               dir_2_ptr,
    int*               path_1_ptr,
    int*               path_2_ptr,
    double*            quality_ptr
)
{
    int dir, dir_2, dir_3;
    KJB_image* window_ip = NULL;
    KJB_image* mag_ip = NULL;
    KJB_image* mag_2_ip = NULL;
    double selected_weight = DBL_MOST_POSITIVE;
    int k, kk, kkk;
    double t;
    double t_pos;
    double t_neg;
    double selected_t_pos = DBL_NOT_SET;
    double selected_t_neg = DBL_NOT_SET;
    int selected_path = NOT_SET;
    int selected_path_2 = NOT_SET;
    int selected_dir = NOT_SET;
    int selected_dir_2 = NOT_SET;
    double selected_error = DBL_MOST_POSITIVE;
    double error;
    double kappa_stdev = DBL_NOT_SET;
    double kappa_mean = DBL_NOT_SET;
    double t_zero_kappa = DBL_NOT_SET;
    double t_error_at_zero = DBL_NOT_SET;
    double selected_kappa_stdev = DBL_MOST_POSITIVE;
    double selected_kappa_mean = DBL_NOT_SET;
    double selected_t_zero_kappa = DBL_NOT_SET;
    double selected_t_error_at_zero = DBL_MOST_NEGATIVE;
    double selected_quality = DBL_MOST_POSITIVE;
    int selected_jump_junction = NOT_SET;
    double quality = DBL_NOT_SET;
    Matrix* r_mp = NULL;
    Matrix* r1_mp = NULL;
    Matrix* r2_mp = NULL;
    Vector* t_vp = NULL;
    Vector* fit_t_vp = NULL;
    Vector* fit_t1_vp = NULL;
    Vector* fit_t2_vp = NULL;
    Matrix* c_mp = NULL;
    Matrix* selected_c_mp = NULL;
    int len, len_2, len_3;
    int path, path_2, path_3;
    int good_trio = FALSE;  /* Are there three paths in three directions that don't overlap. */
    int count_paths = 0;
    int jump_junction;
    int fit_len;
    int fit_len_2;

#ifdef DEBUG_CHOOSE
    static int p_count = 0;
    fs_choose_it = ((kjb_rand() < 0.02) && (i > debug_ip->num_rows / 3) && (j > debug_ip->num_cols / 3) && (i < 2 * debug_ip->num_rows / 3) && (j < 2 * debug_ip->num_cols / 3));
    fs_choose_it = TRUE;
    fs_choose_it = ((ABS_OF(i - 326) < 5) && (ABS_OF(j - 471) < 5)) || ((ABS_OF(i - 306) < 5) && (ABS_OF(j - 355) < 5)) || ((ABS_OF(i - 407) < 5) && (ABS_OF(j - 458) < 5));
    fs_choose_it = ((ABS_OF(i - 326) < 5) && (ABS_OF(j - 471) < 5));
    fs_choose_it = ((ABS_OF(i - 407) < 5) && (ABS_OF(j - 458) < 5));
    fs_choose_it = (((ABS_OF(i - 537) < 5) && (ABS_OF(j - 295) < 5)) || ((ABS_OF(i - 516) < 5) && (ABS_OF(j - 297) < 5)));
    fs_choose_it = ((ABS_OF(i - 317) < 5) && (ABS_OF(j - 467) < 5));
    fs_choose_it = (((ABS_OF(i - 410) < 5) && (ABS_OF(j - 283) < 5)) || ((ABS_OF(i - 414) < 5) && (ABS_OF(j - 179) < 5)));
    fs_choose_it = (((ABS_OF(i - 414) < 5) && (ABS_OF(j - 179) < 5)));
    fs_choose_it = (((ABS_OF(i - 278) < 5) && (ABS_OF(j - 459) < 5)));
    fs_choose_it = FALSE;
    fs_choose_it = (((ABS_OF(i - 396) < 5) && (ABS_OF(j - 480) < 5)));
    fs_choose_it = (((ABS_OF(i - 373) < 5) && (ABS_OF(j - 458) < 5)));
    fs_choose_it = (((ABS_OF(i - 376) < 5) && (ABS_OF(j - 471) < 5)));
    fs_choose_it = (((ABS_OF(i - 215) < 5) && (ABS_OF(j - 406) < 5)));
    fs_choose_it = ((((ABS_OF(i - 396) < 5) && (ABS_OF(j - 480) < 5))) || (((ABS_OF(i - 408) < 5) && (ABS_OF(j - 458) < 5))));
    fs_choose_it = (((ABS_OF(i - 342) < 5) && (ABS_OF(j - 405) < 5)));
    fs_choose_it = ((ABS_OF(i - 326) < 5) && (ABS_OF(j - 471) < 5)) || ((ABS_OF(i - 306) < 5) && (ABS_OF(j - 355) < 5)) || ((ABS_OF(i - 407) < 5) && (ABS_OF(j - 458) < 5));
    fs_choose_it = FALSE;
    fs_choose_it = (((ABS_OF(i - 336) < 5) && (ABS_OF(j - 553) < 5)) || ((ABS_OF(i - 343) < 5) && (ABS_OF(j - 550) < 5)) );
    fs_choose_it = (((ABS_OF(i - 466) < 5) && (ABS_OF(j - 251) < 5)) );
    fs_choose_it = (((ABS_OF(i - 315) < 5) && (ABS_OF(j - 654) < 5)) );
    fs_choose_it = (((ABS_OF(i - 376) < 5) && (ABS_OF(j - 471) < 5)) );
    fs_choose_it = (((ABS_OF(i - 331) < 2) && (ABS_OF(j - 533) < 2)) );

    if (fs_choose_it)
    {
        EPETE(get_image_window(&window_ip, debug_ip, i - 40, j - 40, 80, 80));
        EPETE(image_draw_point(window_ip, 40, 40, 0, 0, 200, 200));
        EPETE(magnify_image(&mag_ip, window_ip, 3, 3));
        p_count ++;
    }
#endif

    for (dir = 0; dir < CURV_NUM_DIRECTIONS; dir++)
    {
        EPETE(get_all_paths(max_num_paths, min_len_for_stopping_at_junctions,
                            DEPTH_FIRST_DEPTH, max_path_len,
                            on_mp, term_mp, curvature_mp,
                            i_mvp->elements[ dir ], j_mvp->elements[ dir ],
                            t_mvp->elements[ dir ], len_vvp->elements[ dir ],
                            i, j, dir));

#ifdef TEST
        EPETE(verify_paths(len_vvp->elements[ dir ], i_mvp->elements[ dir ], j_mvp->elements[ dir ]));
#endif
    }

    for (dir = 0; dir < CURV_NUM_DIRECTIONS; dir++)
    {
        for (dir_2 = dir + 1; dir_2 < CURV_NUM_DIRECTIONS; dir_2++)
        {
            for (dir_3 = dir_2 + 1; dir_3 < CURV_NUM_DIRECTIONS; dir_3++)
            {
                int num_paths   = len_vvp->elements[ dir ]->length;
                int num_paths_2 = len_vvp->elements[ dir_2 ]->length;
                int num_paths_3 = len_vvp->elements[ dir_3 ]->length;

                for (path = 0; path < num_paths; path++)
                {
                    len = len_vvp->elements[ dir ]->elements[ path ];

                    /* Must be able to have a path of len 2. */
                    if (len < 2) continue;

                    for (path_2 = 0; path_2 < num_paths_2; path_2++)
                    {
                        int duplicate = FALSE;

                        len_2 = len_vvp->elements[ dir_2 ]->elements[ path_2 ];

                        /* Must be able to have a path of len 2. */
                        if (len_2 < 2) continue;

                        /*
                         * We need at least a pair of paths that are more than
                         * MIN_BUFFER_LENGTH.
                        */

                        /*
                         * Maybe we want to consider these after all? Easier to
                         * write about?
                         *
                        if ((len < MIN_BUFFER_LENGTH) && (len_2 < MIN_BUFFER_LENGTH)) continue;
                        */

                        for (k = 1; k < len; k++)
                        {
                            for (kk = 1; kk < len_2; kk++)
                            {
                                if (    (i_mvp->elements[ dir ]->elements[ path ][ k ] == i_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ])
                                     && (j_mvp->elements[ dir ]->elements[ path ][ k ] == j_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ])
                                   )
                                {
                                    duplicate = TRUE;
                                    break;
                                }
                            }
                        }

                        if (duplicate) continue;

                        for (path_3 = 0; path_3 < num_paths_3; path_3++)
                        {
                            int duplicate_2 = FALSE;

                            len_3 = len_vvp->elements[ dir_3 ]->elements[ path_3 ];

                            /*
                             * We need at least a pair of paths that are more than
                             * MIN_BUFFER_LENGTH.
                            */
                            /*
                             * Maybe we want to consider these after all? Easier
                             * to write about?
                             *
                            if ((len < MIN_BUFFER_LENGTH) && (len_3 < MIN_BUFFER_LENGTH)) continue;
                            if ((len_2 < MIN_BUFFER_LENGTH) && (len_3 < MIN_BUFFER_LENGTH)) continue;
                            */

                            for (k = 1; k < len; k++)
                            {
                                for (kkk = 1; kkk < len_3; kkk++)
                                {
                                    if (    (i_mvp->elements[ dir ]->elements[ path ][ k ] == i_mvp->elements[ dir_3 ]->elements[ path_3 ][ kkk ])
                                         && (j_mvp->elements[ dir ]->elements[ path ][ k ] == j_mvp->elements[ dir_3 ]->elements[ path_3 ][ kkk ])
                                       )
                                    {
                                        duplicate_2 = TRUE;
                                        break;
                                    }
                                }
                            }

                            for (kk = 1; kk < len_2; kk++)
                            {
                                if (duplicate_2) break;

                                for (kkk = 1; kkk < len_3; kkk++)
                                {
                                    if (    (i_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ] == i_mvp->elements[ dir_3 ]->elements[ path_3 ][ kkk ])
                                         && (j_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ] == j_mvp->elements[ dir_3 ]->elements[ path_3 ][ kkk ])
                                       )
                                    {
                                        duplicate_2 = TRUE;
                                        break;
                                    }
                                }
                                if (duplicate_2) break;
                            }

                            if (duplicate_2) continue;

                            good_trio = TRUE;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (! good_trio)
    {
        return 0;
    }

    for (dir = 0; dir < CURV_NUM_DIRECTIONS; dir++)
    {
        for (dir_2 = dir + 1; dir_2 < CURV_NUM_DIRECTIONS; dir_2++)
        {
            int num_paths   = len_vvp->elements[ dir ]->length;
            int num_paths_2 = len_vvp->elements[ dir_2 ]->length;
            double e1, e2;

            for (path = 0; path < num_paths; path++)
            {
                for (path_2 = 0; path_2 < num_paths_2; path_2++)
                {
                    int duplicate = FALSE;

                    len = len_vvp->elements[ dir ]->elements[ path ];
                    len_2 = len_vvp->elements[ dir_2 ]->elements[ path_2 ];

                    if ((len < MIN_BUFFER_LENGTH) && (len_2 < MIN_BUFFER_LENGTH)) continue;

                    if ((len <= 1) || (len_2 <= 1)) continue;

                    for (k = 1; k < len; k++)
                    {
                        for (kk = 1; kk < len_2; kk++)
                        {
                            if (    (i_mvp->elements[ dir ]->elements[ path ][ k ] == i_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ])
                                 && (j_mvp->elements[ dir ]->elements[ path ][ k ] == j_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ])
                               )
                            {
                                duplicate = TRUE;
                                break;
                            }
                        }
                        if (duplicate) break;
                    }

                    if (duplicate)
                    {
                        continue;
                    }

#define NO_SELF_TOUCHING_PATHS

#ifdef NO_SELF_TOUCHING_PATHS
                    for (k = 2; k < len; k++)
                    {
                        int i1 = i_mvp->elements[ dir ]->elements[ path ][ k ];
                        int j1 = j_mvp->elements[ dir ]->elements[ path ][ k ];

                        for (kk = 2; kk < len_2; kk++)
                        {
                            int i2 = i_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ];
                            int j2 = j_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ];

                            if ((ABS_OF(i1 - i2) < 2) && (ABS_OF(j1 - j2) < 2))
                            {
                                duplicate = TRUE;
                                break;
                            }
                        }
                        if (duplicate) break;
                    }

                    if (duplicate)
                    {
                        continue;
                    }
#endif

                    EPETE(get_target_vector(&t_vp, len + len_2 - 1));
                    EPETE(get_target_matrix(&r_mp, len + len_2 - 1, 2));
                    EPETE(get_target_vector(&fit_t1_vp, len));
                    EPETE(get_target_matrix(&r1_mp, len, 2));
                    EPETE(get_target_vector(&fit_t2_vp, len_2));
                    EPETE(get_target_matrix(&r2_mp, len_2, 2));

                    t = 0.0;

                    for (k = 0; k < len; k++)
                    {
                        ASSERT_IS_NUMBER_DBL(t);
                        t_vp->elements[ k ] = t;
                        fit_t1_vp->elements[ k ] = t;
                        r_mp->elements[ k ][ 0 ] = j_mvp->elements[ dir ]->elements[ path ][ k ];
                        r_mp->elements[ k ][ 1 ] = i_mvp->elements[ dir ]->elements[ path ][ k ];
                        r1_mp->elements[ k ][ 0 ] = j_mvp->elements[ dir ]->elements[ path ][ k ];
                        r1_mp->elements[ k ][ 1 ] = i_mvp->elements[ dir ]->elements[ path ][ k ];

                        if ((k + 1) < len)
                        {
                            t += t_mvp->elements[ dir ]->elements[ path ][ k + 1];
                        }
                    }

                    fit_t2_vp->elements[ 0 ] = 0.0;
                    r2_mp->elements[ 0 ][ 0 ] = j_mvp->elements[ dir_2 ]->elements[ path_2 ][ 0 ];
                    r2_mp->elements[ 0 ][ 1 ] = i_mvp->elements[ dir_2 ]->elements[ path_2 ][ 0 ];

                    t = -t_mvp->elements[ dir_2 ]->elements[ path_2 ][ 1 ];

                    for (k = 1; k < len_2; k++)
                    {
                        ASSERT_IS_NUMBER_DBL(t);
                        t_vp->elements[ len - 1 + k ] = t;
                        fit_t2_vp->elements[ k ] = t;
                        r_mp->elements[ len - 1 + k ][ 0 ] = j_mvp->elements[ dir_2 ]->elements[ path_2 ][ k ];
                        r_mp->elements[ len - 1 + k ][ 1 ] = i_mvp->elements[ dir_2 ]->elements[ path_2 ][ k ];
                        r2_mp->elements[ k ][ 0 ] = j_mvp->elements[ dir_2 ]->elements[ path_2 ][ k ];
                        r2_mp->elements[ k ][ 1 ] = i_mvp->elements[ dir_2 ]->elements[ path_2 ][ k ];

                        if ((k + 1) < len_2)
                        {
                            t -= t_mvp->elements[ dir_2 ]->elements[ path_2 ][ k + 1 ];
                        }
                    }

                    ERE(copy_vector(&fit_t_vp, t_vp));

                    for (jump_junction = 0; jump_junction < 1; jump_junction++)
                    {
                        if (jump_junction == 0)
                        {
                            fit_len = len;
                            fit_len_2 = len_2;
                        }
                        else if ((len < MIN_BUFFER_LENGTH) || (len_2 < MIN_BUFFER_LENGTH))
                        {
                            break;
                        }
                        else
                        {
                            int i1 = i_mvp->elements[ dir ]->elements[ path ][ JUNCTION_SIZE ];
                            int j1 = j_mvp->elements[ dir ]->elements[ path ][ JUNCTION_SIZE ];
                            int i2 = i_mvp->elements[ dir_2 ]->elements[ path_2 ][ JUNCTION_SIZE ];
                            int j2 = j_mvp->elements[ dir_2 ]->elements[ path_2 ][ JUNCTION_SIZE ];
                            double di = i1 - i2;
                            double dj = j1 - j2;
                            double dt = sqrt(di*di + dj*dj) / 2.0;

                            t = dt;

                            for (k = JUNCTION_SIZE; k < len; k++)
                            {
                                ASSERT_IS_NUMBER_DBL(t);
                                fit_t_vp->elements[ k - JUNCTION_SIZE ] = t;
                                fit_t1_vp->elements[ k - JUNCTION_SIZE ] = t;
                                r_mp->elements[ k - JUNCTION_SIZE ][ 0 ] = j_mvp->elements[ dir ]->elements[ path ][ k ];
                                r_mp->elements[ k - JUNCTION_SIZE ][ 1 ] = i_mvp->elements[ dir ]->elements[ path ][ k ];
                                r1_mp->elements[ k - JUNCTION_SIZE ][ 0 ] = j_mvp->elements[ dir ]->elements[ path ][ k ];
                                r1_mp->elements[ k - JUNCTION_SIZE ][ 1 ] = i_mvp->elements[ dir ]->elements[ path ][ k ];

                                if ((k + 1) < len)
                                {
                                    t += t_mvp->elements[ dir ]->elements[ path ][ k + 1];
                                }
                            }

                            t = -dt;

                            for (k = JUNCTION_SIZE; k < len_2; k++)
                            {
                                ASSERT_IS_NUMBER_DBL(t);
                                fit_t_vp->elements[ len - 2 * JUNCTION_SIZE + k ] = t;
                                fit_t2_vp->elements[ k - JUNCTION_SIZE ] = t;
                                r_mp->elements[ len - 2 * JUNCTION_SIZE + k ][ 0 ] = j_mvp->elements[ dir_2 ]->elements[ path_2 ][ k ];
                                r_mp->elements[ len - 2 * JUNCTION_SIZE + k ][ 1 ] = i_mvp->elements[ dir_2 ]->elements[ path_2 ][ k ];
                                r2_mp->elements[ k - JUNCTION_SIZE ][ 0 ] = j_mvp->elements[ dir_2 ]->elements[ path_2 ][ k ];
                                r2_mp->elements[ k - JUNCTION_SIZE ][ 1 ] = i_mvp->elements[ dir_2 ]->elements[ path_2 ][ k ];

                                if ((k + 1) < len_2)
                                {
                                    t -= t_mvp->elements[ dir_2 ]->elements[ path_2 ][ k + 1 ];
                                }
                            }

                            fit_len = len - JUNCTION_SIZE;
                            fit_len_2 = len_2 - JUNCTION_SIZE;

                            fit_t_vp->length = fit_len + fit_len_2;
                            fit_t1_vp->length = fit_len;
                            fit_t2_vp->length = fit_len_2;
                            r_mp->num_rows = fit_len + fit_len_2;
                            r1_mp->num_rows = fit_len;
                            r2_mp->num_rows = fit_len_2;
                        }

                        count_paths++;

#ifdef ITERATIVE_FIT
                        EPETE(fit_parametric_cubic(fit_t_vp, r_mp, NULL, NULL, &fit_t_vp, &c_mp, &error));
#else
                        EPETE(fit_parametric_cubic(fit_t_vp, r_mp, NULL, NULL, NULL, &c_mp, &error));
#endif

                        EPETE(fit_parametric_cubic(fit_t1_vp, r1_mp, NULL, c_mp, NULL, NULL, &e1));
                        EPETE(fit_parametric_cubic(fit_t2_vp, r2_mp, NULL, c_mp, NULL, NULL, &e2));


                        ERE(clear_data_stats());

                        /*
                        for (k = 0; k < MIN_OF(len, MAX_CUT_PATH_LENGTH / 2); k++)
                        */
                        for (k = 0; k < len; k++)
                        {
                            double t = t_vp->elements[ k ];
                            double t2 = t*t;
                            double xp =  3.0 * c_mp->elements[ 0 ][ 0 ] * t2 + 2.0 * c_mp->elements[ 1 ][ 0 ] * t + c_mp->elements[ 2 ][ 0 ];
                            double xpp = 6.0 * c_mp->elements[ 0 ][ 0 ] * t  + 2.0 * c_mp->elements[ 1 ][ 0 ];
                            double yp =  3.0 * c_mp->elements[ 0 ][ 1 ] * t2 + 2.0 * c_mp->elements[ 1 ][ 1 ] * t + c_mp->elements[ 2 ][ 1 ];
                            double ypp = 6.0 * c_mp->elements[ 0 ][ 1 ] * t  + 2.0 * c_mp->elements[ 1 ][ 1 ];
                            double kappa = 2.0 * (xp*ypp - yp*xpp) / pow((xp*xp + yp*yp), 1.5);

                            if (k == 0)
                            {
                                t_zero_kappa = ABS_OF(kappa);
                            }

                            ERE(add_data_point(ABS_OF(kappa)));
                        }

                        /*
                        for (k = len; k < len + MIN_OF(len_2, MAX_CUT_PATH_LENGTH / 2)  - 1; k++)
                        */
                        for (k = len; k < len + len_2 - 1; k++)
                        {
                            double t = t_vp->elements[ k ];
                            double t2 = t*t;
                            double xp =  3.0 * c_mp->elements[ 0 ][ 0 ] * t2 + 2.0 * c_mp->elements[ 1 ][ 0 ] * t + c_mp->elements[ 2 ][ 0 ];
                            double xpp = 6.0 * c_mp->elements[ 0 ][ 0 ] * t  + 2.0 * c_mp->elements[ 1 ][ 0 ];
                            double yp =  3.0 * c_mp->elements[ 0 ][ 1 ] * t2 + 2.0 * c_mp->elements[ 1 ][ 1 ] * t + c_mp->elements[ 2 ][ 1 ];
                            double ypp = 6.0 * c_mp->elements[ 0 ][ 1 ] * t  + 2.0 * c_mp->elements[ 1 ][ 1 ];
                            double kappa = 2.0 * (xp*ypp - yp*xpp) / pow((xp*xp + yp*yp), 1.5);

                            /*
                            ERE(add_data_point(ABS_OF(kappa)));
                            */
                            ERE(add_data_point((kappa)));
                        }

                        t_pos = t_vp->elements[ len - 1 ];
                        t_neg = t_vp->elements[ len_2 + len - 2 ];

                        ERE(get_data_stats(&kappa_mean, &kappa_stdev, NULL, NULL, NULL));

                        /*
                        error = sqrt((e1*e1 + e2*e2) / 2.0);
                        */

                        /*
                        kappa_stdev /= (0.001 + ABS_OF(kappa_mean));
                        */

                        quality = kappa_stdev * exp(error * error);
                        quality = 5.0 * kappa_stdev + error * 32.0 / (fit_len + fit_len_2 - 4.0);
                        quality = 5.0 * kappa_stdev + error + ((double)(64 - fit_len - fit_len_2)) / 64.0;

                        quality = 5.0 * kappa_stdev + error;
                        quality /= (fit_len + fit_len_2 - 5 + jump_junction);


                        if (quality < selected_quality)
                        {
                            selected_jump_junction = jump_junction;
                            selected_kappa_stdev = kappa_stdev;
                            selected_kappa_mean = kappa_mean;
                            selected_t_zero_kappa = t_zero_kappa;

                            selected_path = path;
                            selected_path_2 = path_2;
                            selected_dir = dir;
                            selected_dir_2 = dir_2;
                            selected_error = error;
                            selected_t_error_at_zero = t_error_at_zero;

#ifdef BALANCE_WEIGHT
                            selected_weight = (t_mvp->elements[ dir ]->elements[ path ][ 1 ] + t_mvp->elements[ dir_2 ]->elements[ path_2 ][ 1]) / 2.0;
#endif

                            EPETE(copy_matrix(&selected_c_mp, c_mp));
                            selected_t_pos = t_pos;
                            selected_t_neg = t_neg;

                            selected_quality = quality;


#ifdef DEBUG_IT_2
                            if (fs_choose_it)
                            {
                                int tt;
                                double prev_ttt;

                                ERE(clear_data_stats());

                                EPETE(kjb_copy_image(&mag_2_ip, mag_ip));

                                prev_ttt = DBL_MOST_NEGATIVE;

                                for (tt = 0; tt < CURVE_IMAGE_RESOLUTION; tt++)
                                {
                                    double ttt = t_neg + (tt * (t_pos - t_neg)) / (CURVE_IMAGE_RESOLUTION - 1.0);
                                    double d_j = c_mp->elements[ 0 ][ 0 ]*ttt*ttt*ttt + c_mp->elements[ 1 ][ 0 ]*ttt*ttt + c_mp->elements[ 2 ][ 0 ]*ttt + c_mp->elements[ 3 ][ 0 ];
                                    double d_i = c_mp->elements[ 0 ][ 1 ]*ttt*ttt*ttt + c_mp->elements[ 1 ][ 1 ]*ttt*ttt + c_mp->elements[ 2 ][ 1 ]*ttt + c_mp->elements[ 3 ][ 1 ];
                                    int ii  = kjb_rint(3.0 * (d_i - i) + 121.0);
                                    int jj  = kjb_rint(3.0 * (d_j - j) + 121.0);
                                    double t2 = ttt*ttt;
                                    double xp =  3.0 * c_mp->elements[ 0 ][ 0 ] * t2 + 2.0 * c_mp->elements[ 1 ][ 0 ] * ttt + c_mp->elements[ 2 ][ 0 ];
                                    double xpp = 6.0 * c_mp->elements[ 0 ][ 0 ] * ttt  + 2.0 * c_mp->elements[ 1 ][ 0 ];
                                    double yp =  3.0 * c_mp->elements[ 0 ][ 1 ] * t2 + 2.0 * c_mp->elements[ 1 ][ 1 ] * ttt + c_mp->elements[ 2 ][ 1 ];
                                    double ypp = 6.0 * c_mp->elements[ 0 ][ 1 ] * ttt  + 2.0 * c_mp->elements[ 1 ][ 1 ];
                                    double kappa = 2.0 * (xp*ypp - yp*xpp) / pow((xp*xp + yp*yp), 1.5);

                                    /*
                                    ERE(add_data_point(ABS_OF(kappa)));
                                    */
                                    ERE(add_data_point((kappa)));


                                    /*
                                    dbi(d_i);
                                    dbi(d_j);
                                     */

                                    if (ttt > 0.0)
                                    {
                                        EPETE(image_draw_point(mag_2_ip, ii, jj, 0, 0, 200, 0));
                                    }
                                    else
                                    {
                                        EPETE(image_draw_point(mag_2_ip, ii, jj, 0, 0, 0, 200));
                                    }

                                    prev_ttt = ttt;
                                }

                                ERE(get_data_stats(&kappa_mean, &kappa_stdev, NULL, NULL, NULL));

                                for (kk = jump_junction*JUNCTION_SIZE; kk<len; kk++)
                                {
                                    int ii = i_mvp->elements[dir ]->elements[ path ][ kk ];
                                    int jj = j_mvp->elements[dir ]->elements[ path ][ kk ];

                                    ASSERT(on_mp->elements[ ii ][ jj ] != 0);

                                    if (kk == 0)
                                    {
                                        EPETE(image_draw_point(mag_2_ip, 1 + 3 * (ii - i) + 120, 1 + 3 * (jj - j) + 120, 0, 0, 0, 255));
                                    }
                                    else
                                    {
                                        EPETE(image_draw_point(mag_2_ip, 1 + 3 * (ii - i) + 120, 1 + 3 * (jj - j) + 120, 0, 255, 0, 0));
                                    }
                                }
                                for (kk = jump_junction*JUNCTION_SIZE + 1 - jump_junction; kk<len_2; kk++)
                                {
                                    int ii = i_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ];
                                    int jj = j_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ];

                                    ASSERT(on_mp->elements[ ii ][ jj ] != 0);

                                    EPETE(image_draw_point(mag_2_ip, 1 + 3 * (ii - i) + 120, 1 + 3 * (jj - j) + 120, 0, 255, 0, 100));
                                }

                                {
                                    char buff[ 1000 ];

                                    ERE(kjb_sprintf(buff, sizeof(buff), "er: %.3f", error));
                                    ERE(image_draw_text_top_left(mag_2_ip, buff, 5, 5, NULL));
                                    ERE(kjb_sprintf(buff, sizeof(buff), "SD: %.3f", selected_kappa_stdev));
                                    ERE(image_draw_text_top_left(mag_2_ip, buff, 20, 5, NULL));
                                    ERE(kjb_sprintf(buff, sizeof(buff), "QU: %.3f", quality));
                                    ERE(image_draw_text_top_left(mag_2_ip, buff, 35, 5, NULL));
                                    ERE(kjb_sprintf(buff, sizeof(buff), "(%d, %d)", i, j));
                                    ERE(image_draw_text_top_left(mag_2_ip, buff, 50, 5, NULL));
                                    ERE(kjb_sprintf(buff, sizeof(buff), "JJ: %d", jump_junction));
                                    ERE(image_draw_text_top_left(mag_2_ip, buff, 65, 5, NULL));
                                    ERE(kjb_sprintf(buff, sizeof(buff), "SD2: %.3f", kappa_stdev));
                                    ERE(image_draw_text_top_left(mag_2_ip, buff, 80, 5, NULL));
                                }

                                EPETE(image_draw_rectangle(mag_2_ip, mag_2_ip->num_rows - 10, mag_2_ip->num_cols - 10, 10, 10, 200, 0, 0));

                                kjb_display_image(mag_2_ip, NULL);
                                prompt_to_continue();

                            }
#endif
                        }
                    }
                }
            }
        }
    }

#ifdef DEBUG_IT
    if ((fs_choose_it) && (count_paths > 0))
    {
        int tt;
        double prev_ttt;

        path = selected_path;
        path_2 = selected_path_2;
        dir = selected_dir;
        dir_2 = selected_dir_2;
        len = len_vvp->elements[ dir ]->elements[ path ];
        len_2 = len_vvp->elements[ dir_2 ]->elements[ path_2 ];

        EPETE(copy_matrix(&c_mp, selected_c_mp));

        EPETE(kjb_copy_image(&mag_2_ip, mag_ip));

        prev_ttt = DBL_MOST_NEGATIVE;

        for (tt = 0; tt < CURVE_IMAGE_RESOLUTION; tt++)
        {
            double ttt = selected_t_neg + (tt * (selected_t_pos - selected_t_neg)) / (CURVE_IMAGE_RESOLUTION - 1.0);
            double d_j = c_mp->elements[ 0 ][ 0 ]*ttt*ttt*ttt + c_mp->elements[ 1 ][ 0 ]*ttt*ttt + c_mp->elements[ 2 ][ 0 ]*ttt + c_mp->elements[ 3 ][ 0 ];
            double d_i = c_mp->elements[ 0 ][ 1 ]*ttt*ttt*ttt + c_mp->elements[ 1 ][ 1 ]*ttt*ttt + c_mp->elements[ 2 ][ 1 ]*ttt + c_mp->elements[ 3 ][ 1 ];
            int ii  = kjb_rint(3.0 * (d_i - i) + 121.0);
            int jj  = kjb_rint(3.0 * (d_j - j) + 121.0);

            /*
            dbi(d_i);
            dbi(d_j);
             */

            if (ttt > 0.0)
            {
                EPETE(image_draw_point(mag_2_ip, ii, jj, 0, 0, 200, 0));
            }
            else
            {
                EPETE(image_draw_point(mag_2_ip, ii, jj, 0, 0, 0, 200));
            }

            prev_ttt = ttt;
        }

        for (kk = 0; kk<len; kk++)
        {
            int ii = i_mvp->elements[dir ]->elements[ path ][ kk ];
            int jj = j_mvp->elements[dir ]->elements[ path ][ kk ];

            ASSERT(on_mp->elements[ ii ][ jj ] != 0);

            if (kk == 0)
            {
                EPETE(image_draw_point(mag_2_ip, 1 + 3 * (ii - i) + 120, 1 + 3 * (jj - j) + 120, 0, 0, 0, 200));
            }
            else
            {
                EPETE(image_draw_point(mag_2_ip, 1 + 3 * (ii - i) + 120, 1 + 3 * (jj - j) + 120, 0, 200 + kk, 0, 0));
            }
        }
        for (kk = 1; kk<len_2; kk++)
        {
            int ii = i_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ];
            int jj = j_mvp->elements[ dir_2 ]->elements[ path_2 ][ kk ];

            ASSERT(on_mp->elements[ ii ][ jj ] != 0);

            EPETE(image_draw_point(mag_2_ip, 1 + 3 * (ii - i) + 120, 1 + 3 * (jj - j) + 120, 0, 0, 200 + kk, 0));
        }

        /*
        dbe(selected_t_error_at_zero);
        dbe(selected_error);
        dbe(selected_t_zero_kappa);
        dbe(selected_kappa_mean);
        dbe(selected_kappa_stdev);
        dbi(selected_dir);
        dbi(selected_dir_2);
        dbe(selected_quality);
        */

        {
            char buff[ 1000 ];

            ERE(kjb_sprintf(buff, sizeof(buff), "er: %.3f", selected_error));
            ERE(image_draw_text_top_left(mag_2_ip, buff, 5, 5, NULL));
            ERE(kjb_sprintf(buff, sizeof(buff), "SD: %.3f", selected_kappa_stdev));
            ERE(image_draw_text_top_left(mag_2_ip, buff, 20, 5, NULL));
            /*
            ERE(kjb_sprintf(buff, sizeof(buff), "ZE: %.3f", 100.0 * ABS_OF(selected_t_zero_kappa - selected_kappa_mean)));
            ERE(image_draw_text_top_left(mag_2_ip, buff, 35, 5, NULL));
            */
            ERE(kjb_sprintf(buff, sizeof(buff), "QU: %.3f", selected_quality));
            ERE(image_draw_text_top_left(mag_2_ip, buff, 35, 5, NULL));
            ERE(kjb_sprintf(buff, sizeof(buff), "(%d, %d)", i, j));
            ERE(image_draw_text_top_left(mag_2_ip, buff, 50, 5, NULL));
            ERE(kjb_sprintf(buff, sizeof(buff), "JJ: %d", selected_jump_junction));
            ERE(image_draw_text_top_left(mag_2_ip, buff, 80, 5, NULL));
        }

        EPETE(image_draw_rectangle(mag_2_ip, mag_2_ip->num_rows - 10, mag_2_ip->num_cols - 10, 10, 10, 0, 200, 0));

        kjb_display_image(mag_2_ip, NULL);
        prompt_to_continue();

    }
#endif

    if (count_paths > 0)
    {
        *dir_1_ptr = selected_dir;
        *dir_2_ptr = selected_dir_2;
        *path_1_ptr = selected_path;
        *path_2_ptr = selected_path_2;
        *quality_ptr = selected_quality;
    }

    kjb_free_image(mag_2_ip);
    kjb_free_image(mag_ip);
    kjb_free_image(window_ip);
    free_vector(t_vp);
    free_vector(fit_t_vp);
    free_vector(fit_t1_vp);
    free_vector(fit_t2_vp);
    free_matrix(c_mp);
    free_matrix(r_mp);
    free_matrix(r1_mp);
    free_matrix(r2_mp);
    free_matrix(selected_c_mp);

    return count_paths;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* Counts neigbors that are not axis parallel neighbors among themselves. */
static int count_neighbors_without_aligned_neighbors
(
    const Int_matrix* image_mp,
    int               i,
    int               j
)
{
    int num_rows = image_mp->num_rows;
    int num_cols = image_mp->num_cols;
    int min_ii = MAX_OF(0, i - 1);
    int min_jj = MAX_OF(0, j - 1);
    int max_ii = MIN_OF(i + 1, num_rows - 1);
    int max_jj = MIN_OF(j + 1, num_cols - 1);
    int ii, jj;
    int count = 0;
    int nb_ii[ 8 ];
    int nb_jj[ 8 ];
    int pass;

    for (pass = 0; pass < 2; pass++)
    {
        for (ii = min_ii; ii <= max_ii; ii++)
        {
            for (jj = min_jj; jj <= max_jj; jj++)
            {
                if ((ii != i) || (jj != j))
                {
                    if (image_mp->elements[ ii ][ jj ])
                    {
                        if (pass == 0)
                        {
                            /*
                             * Pass one: restrict to adding neigbours that don't
                             * have two neighbors.  This is so that we don't
                             * arbitrarily choose an on pixel in the middle of
                             * two other on pixels, when we could chose the two
                             * outside ones instead.
                             */
                            int count_nb_nb = 0;

                            if ((ii > min_ii) && ((ii - 1 != i) || (jj != j)) && (image_mp->elements[ ii - 1 ][ jj ]))
                            {
                                count_nb_nb++;
                            }

                            if ((ii < max_ii) && ((ii + 1 != i) || (jj != j)) && (image_mp->elements[ ii + 1 ][ jj ]))
                            {
                                count_nb_nb++;
                            }

                            if ((jj > min_jj) && ((ii != i) || (jj - 1 != j)) && (image_mp->elements[ ii ][ jj  - 1]))
                            {
                                count_nb_nb++;
                            }

                            if ((jj < max_jj) && ((ii != i) || (jj + 1 != j)) && (image_mp->elements[ ii ][ jj + 1 ]))
                            {
                                count_nb_nb++;
                            }

                            if (count_nb_nb > 1) continue;
                        }

                        {
                            int found = TRUE;
                            int c2;

                            for (c2 = 0; c2 < count; c2++)
                            {
                                if ((ABS_OF(nb_ii[ c2 ] - ii) + ABS_OF(nb_jj[ c2 ] - jj)) < 2)
                                {
                                    found = FALSE;
                                    break;
                                }
                            }

                            if (found)
                            {
                                nb_ii[ count ] = ii;
                                nb_jj[ count ] = jj;
                                count++;
                            }
                        }
                    }
                }
            }
        }
    }

    return count;
}


/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int pixel_on_path
(
    int               i,
    int               j,
    const Int_vector* path_i_vp,
    const Int_vector* path_j_vp
)
{
    int len = path_j_vp->length;
    int k;

    for (k = 0; k < len; k++)
    {
        if ((path_i_vp->elements[ k ] == i) && (path_j_vp->elements[ k ] == j))
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

