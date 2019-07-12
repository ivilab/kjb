
/* $Id: curv_all_paths.c 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) by members of University of Arizona Computer Vision group     |
|  (the authors) including                                                     |
|        Kobus Barnard                                                         |
|                                                                              |
|  For use outside the University of Arizona Computer Vision group please      |
|  contact Kobus Barnard.                                                      |
|                                                                              |
* =========================================================================== */


#include "curv/curv_gen.h"
#include "curv/curv_all_paths.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */

/*
 * For efficiency in recursion we don't put stuff on the stack that does not
 * change.
*/
static int               fs_max_num_paths;
static int               fs_min_len_for_stopping_at_junctions;
static int               fs_max_path_len;
static const Int_matrix* fs_on_mp;
static const Int_matrix* fs_term_mp;
static const Matrix*     fs_curvature_mp;
static Int_matrix*       fs_i_mp;
static Int_matrix*       fs_j_mp;
static Matrix*           fs_t_mp;
static Int_vector*       fs_len_vp;
static int               fs_num_rows;
static int               fs_num_cols;
static int               fs_num_paths;
static int               fs_depth;
static int               fs_max_num_deep_paths;
static int               fs_max_num_paths_2;
static int               fs_depth_first_depth;

/* -------------------------------------------------------------------------- */

static int get_all_paths_2
(
    const int i,
    int       j,
    int       cur_path_index,
    int       cur_len,
    int       prev_dir
);


/* -------------------------------------------------------------------------- */

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
*/
#define INCLUDE_STOPPING_AT_JUNCTIONS

/*
 * If defined, LOOK_BACK must be at least one.
*/
#define LOOK_BACK 7

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int get_all_paths
(
    int               max_num_paths,
    int               min_len_for_stopping_at_junctions,
    int               depth_first_depth,
    int               max_path_len,
    const Int_matrix* on_mp,
    const Int_matrix* term_mp,
    const Matrix*     curvature_mp,
    Int_matrix*       i_mp,
    Int_matrix*       j_mp,
    Matrix*           t_mp,
    Int_vector*       len_vp,
    int               i,
    int               j,
    int               dir
)
{
    int new_i = i + curv_neighbors[dir][0];
    int new_j = j + curv_neighbors[dir][1];

    ASSERT_IS_EQUAL_INT(i_mp->num_rows, j_mp->num_rows);
    ASSERT_IS_EQUAL_INT(t_mp->num_rows, j_mp->num_rows);
    ASSERT_IS_EQUAL_INT(i_mp->num_cols, j_mp->num_cols);
    ASSERT_IS_EQUAL_INT(t_mp->num_cols, j_mp->num_cols);

    ASSERT_IS_NOT_LESS_INT(i_mp->max_num_rows, max_num_paths);

    fs_num_rows = on_mp->num_rows;
    fs_num_cols = on_mp->num_cols;

    if (    (i < 0) || (i >= fs_num_rows) || (j < 0) || (j >= fs_num_cols)
         || ( ! on_mp->elements[ i ][ j ])
         || (new_i < 0) || (new_i >= fs_num_rows) || (new_j < 0) || (new_j >= fs_num_cols)
         || ( ! on_mp->elements[ new_i ][ new_j ])
       )
    {
        i_mp->num_rows = 0;
        j_mp->num_rows = 0;
        t_mp->num_rows = 0;
        len_vp->length = 0;

        return 0;
    }

    i_mp->elements[ 0 ][ 0 ] = i;
    j_mp->elements[ 0 ][ 0 ] = j;
    t_mp->elements[ 0 ][ 0 ] = 0.0;

    i_mp->elements[ 0 ][ 1 ] = new_i;
    j_mp->elements[ 0 ][ 1 ] = new_j;
    t_mp->elements[ 0 ][ 1 ] = ((dir%2)==0)?1.0:M_SQRT2;

    fs_max_num_paths = max_num_paths;
    fs_min_len_for_stopping_at_junctions = min_len_for_stopping_at_junctions;
    fs_max_path_len = max_path_len;

    fs_depth_first_depth = depth_first_depth;

    if (depth_first_depth > 0)
    {
        fs_max_num_deep_paths = max_num_paths / (ipower(9, depth_first_depth));
    }

    fs_num_paths = 1;

    fs_i_mp = i_mp;
    fs_j_mp = j_mp;
    fs_t_mp = t_mp;
    fs_len_vp = len_vp;
    fs_on_mp = on_mp;
    fs_term_mp = term_mp;
    fs_curvature_mp = curvature_mp;

    fs_depth = 0;

    if (get_all_paths_2(new_i, new_j, 0, 2, dir) == ERROR)
    {
        fs_num_paths = ERROR;
    }
    else
    {
        i_mp->num_rows = fs_num_paths;
        j_mp->num_rows = fs_num_paths;
        t_mp->num_rows = fs_num_paths;
        len_vp->length = fs_num_paths;
    }

#ifdef TEST
    ERE(verify_paths(len_vp, i_mp, j_mp));
#endif

    return fs_num_paths;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_all_paths_2
(
    int i,
    int j,
    int cur_path_index,
    int cur_len,
    int prev_dir
)
{
    IMPORT const int curv_neighbors[ CURV_NUM_DIRECTIONS ][ 2 ];
    int d, dir;
    int point;
    int new_i = NOT_SET;
    int new_j = NOT_SET;
    int found = FALSE;
    /* sorted_dir_vp must be local. */
    Indexed_vector*   sorted_dir_vp = NULL;


    if (fs_term_mp->elements[ i ][ j ])
    {
        fs_len_vp->elements[ cur_path_index ] = cur_len;
        return NO_ERROR;
    }

    if ((fs_depth_first_depth > 0) && (fs_depth == fs_depth_first_depth))
    {
        fs_max_num_paths_2 = fs_num_paths + fs_max_num_deep_paths;
    }

    fs_depth++;

    ERE(get_target_indexed_vector(&sorted_dir_vp, CURV_NUM_DIRECTIONS));

    while (cur_len < fs_max_path_len)
    {
        int    count_dir       = 0;
        double dot_product = DBL_NOT_SET;
#ifdef LOOK_BACK
        int    ref_i           = fs_i_mp->elements[ cur_path_index ][ MAX_OF(0, cur_len - LOOK_BACK) ];
        int    ref_j           = fs_j_mp->elements[ cur_path_index ][ MAX_OF(0, cur_len - LOOK_BACK) ];
        int    di              = i - ref_i;
        int    dj              = j - ref_j;
#endif

        for (d = 0; d < CURV_NUM_DIRECTIONS; d++)
        {
            int diff = ABS_OF(prev_dir - d);

            if (diff > 4) diff = 8 - diff;

            new_i = i + curv_neighbors[d][0];
            new_j = j + curv_neighbors[d][1];

            if (
                    (new_i >= 0) && (new_i < fs_num_rows) && (new_j >= 0) && (new_j < fs_num_cols)
                 && (fs_on_mp->elements[ new_i ][ new_j ])
                 && (diff < 3)
               )
            {
                found = FALSE;

                /*
                 * Check if we have already been here. Tricks with arrays
                 * are probably too complicated because of recursion
                 */
                for (point = cur_len - 1; point >= 0; point--)
                {
                    if (     (fs_i_mp->elements[ cur_path_index ][ point ] == new_i)
                          && (fs_j_mp->elements[ cur_path_index ][ point ] == new_j)
                       )
                    {
                        found = TRUE;
                        break;
                    }
                }

                if (found) continue;

#define NO_SELF_TOUCHING_PATHS


#ifdef NO_SELF_TOUCHING_PATHS
                {
                    int min_ii = MAX_OF(0, new_i - 1);
                    int min_jj = MAX_OF(0, new_j - 1);
                    int max_ii = MIN_OF(new_i + 1, fs_num_rows - 1);
                    int max_jj = MIN_OF(new_j + 1, fs_num_cols - 1);
                    int ii, jj;

                    for (ii = min_ii; ii <= max_ii; ii++)
                    {
                        for (jj = min_jj; jj <= max_jj; jj++)
                        {
                            if ((ii == i) && (jj == j)) continue;
                            if ((ii == new_i) && (jj == new_j)) continue;

                            for (point = cur_len - 1; point >= 0; point--)
                            {
                                if (     (fs_i_mp->elements[ cur_path_index ][ point ] == ii)
                                      && (fs_j_mp->elements[ cur_path_index ][ point ] == jj)
                                   )
                                {
                                    found = TRUE;
                                    break;
                                }
                            }

                            if (found) break;
                        }
                        if (found) break;
                    }

                    if (found) continue;
                }

#endif

#ifdef LOOK_BACK
                dot_product = di * curv_neighbors[ d ][ 0 ] + dj * curv_neighbors[ d ][ 1 ];

                if (IS_ODD(d))
                {
                    dot_product /= M_SQRT2;
                }
#endif

                sorted_dir_vp->elements[ count_dir ].index = d;
                sorted_dir_vp->elements[ count_dir ].element = dot_product;
                count_dir++;
            }
        }

        sorted_dir_vp->length = count_dir;

        /* End of chain. No more points. */
        if (count_dir <= 0)
        {
            break;
        }
        else if (count_dir == 1)
        {
            if (    (fs_min_len_for_stopping_at_junctions > 0)
                 && (cur_len >= fs_min_len_for_stopping_at_junctions)
                 && (fs_curvature_mp != NULL)
                 && (fs_curvature_mp->elements[ i ][ j ] > 0.10)
               )
            {
                fs_len_vp->elements[ cur_path_index ] = cur_len;

                if (fs_num_paths >= fs_max_num_paths - 1)
                {
                    verbose_pso(5, "More than %d total paths. Skipping the rest\n",
                                fs_max_num_paths);
                    fs_depth--;
                    free_indexed_vector(sorted_dir_vp);
                    return NO_ERROR;
                }
                else if ((fs_depth_first_depth > 0) && (fs_depth > fs_depth_first_depth) && (fs_num_paths >= fs_max_num_paths_2 - 1))
                {
                    verbose_pso(10, "Too many paths at depth %d. Skipping the rest\n",
                                fs_depth_first_depth);
                    fs_depth--;
                    free_indexed_vector(sorted_dir_vp);
                    return NO_ERROR;
                }

                /* We need to duplicate the path so far. */
                for (point = 0; point < cur_len; point++)
                {
                    ASSERT(fs_on_mp->elements[ fs_i_mp->elements[ cur_path_index ][ point ] ][ fs_j_mp->elements[ cur_path_index ][ point ] ] != 0);

                    fs_i_mp->elements[ fs_num_paths ][ point ] = fs_i_mp->elements[ cur_path_index ][ point ];
                    fs_j_mp->elements[ fs_num_paths ][ point ] = fs_j_mp->elements[ cur_path_index ][ point ];
                    fs_t_mp->elements[ fs_num_paths ][ point ] = fs_t_mp->elements[ cur_path_index ][ point ];
                }

                cur_path_index = fs_num_paths;

                fs_num_paths++;
            }

            dir = sorted_dir_vp->elements[ 0 ].index;

            new_i = i + curv_neighbors[dir][0];
            new_j = j + curv_neighbors[dir][1];

            ASSERT(fs_on_mp->elements[ new_i ][ new_j ] != 0);

            fs_i_mp->elements[ cur_path_index ][ cur_len ] = new_i;
            fs_j_mp->elements[ cur_path_index ][ cur_len ] = new_j;
            fs_t_mp->elements[ cur_path_index ][ cur_len ] = ((dir%2)==0)?1.0:M_SQRT2;

            i = new_i;
            j = new_j;

            prev_dir = dir;

            cur_len++;

            if (fs_term_mp->elements[ i ][ j ])
            {
                fs_len_vp->elements[ cur_path_index ] = cur_len;
                free_indexed_vector(sorted_dir_vp);
                return NO_ERROR;
            }

        }
        else if (count_dir > 1)
        {
#ifdef LOOK_BACK
            ERE(descend_sort_indexed_vector(sorted_dir_vp));
#endif

            if (    (fs_min_len_for_stopping_at_junctions > 0)
                 && (cur_len >= fs_min_len_for_stopping_at_junctions)
               )
            {
                fs_len_vp->elements[ cur_path_index ] = cur_len;

                if (fs_num_paths >= fs_max_num_paths - 1)
                {
                    verbose_pso(5, "More than %d total paths. Skipping the rest\n",
                                fs_max_num_paths);
                    fs_depth--;
                    free_indexed_vector(sorted_dir_vp);
                    return NO_ERROR;
                }
                else if ((fs_depth_first_depth > 0) && (fs_depth > fs_depth_first_depth) && (fs_num_paths >= fs_max_num_paths_2 - 1))
                {
                    verbose_pso(10, "Too many paths at depth %d. Skipping the rest\n",
                                fs_depth_first_depth);
                    fs_depth--;
                    free_indexed_vector(sorted_dir_vp);
                    return NO_ERROR;
                }

                /* We need to duplicate the path so far. */
                for (point = 0; point < cur_len; point++)
                {
                    ASSERT(fs_on_mp->elements[ fs_i_mp->elements[ cur_path_index ][ point ] ][ fs_j_mp->elements[ cur_path_index ][ point ] ] != 0);

                    fs_i_mp->elements[ fs_num_paths ][ point ] = fs_i_mp->elements[ cur_path_index ][ point ];
                    fs_j_mp->elements[ fs_num_paths ][ point ] = fs_j_mp->elements[ cur_path_index ][ point ];
                    fs_t_mp->elements[ fs_num_paths ][ point ] = fs_t_mp->elements[ cur_path_index ][ point ];
                }

                cur_path_index = fs_num_paths;

                fs_num_paths++;
            }

            for (d = 0; d < count_dir; d++)
            {
                dir = sorted_dir_vp->elements[ d ].index;

                new_i = i + curv_neighbors[dir][0];
                new_j = j + curv_neighbors[dir][1];

                ASSERT(fs_on_mp->elements[ new_i ][ new_j ] != 0);

                if (d == 0)
                {
                    ASSERT(fs_on_mp->elements[ new_i ][ new_j ] != 0);

                    fs_i_mp->elements[ cur_path_index ][ cur_len ] = new_i;
                    fs_j_mp->elements[ cur_path_index ][ cur_len ] = new_j;
                    fs_t_mp->elements[ cur_path_index ][ cur_len ] = ((dir%2)==0)?1.0:M_SQRT2;

                    ERE(get_all_paths_2(new_i, new_j, cur_path_index, 1 + cur_len, dir));
                }
                else
                {
                    if (fs_num_paths >= fs_max_num_paths - 1)
                    {
                        verbose_pso(5, "More than %d paths. Skipping the rest\n",
                                    fs_max_num_paths);
                        fs_depth--;
                        free_indexed_vector(sorted_dir_vp);
                        return NO_ERROR;
                    }
                    else if ((fs_depth_first_depth > 0) && (fs_depth > fs_depth_first_depth) && (fs_num_paths >= fs_max_num_paths_2 - 1))
                    {
                        verbose_pso(10, "Too many paths at depth %d. Skipping the rest\n",
                                    fs_depth_first_depth);
                        fs_depth--;
                        free_indexed_vector(sorted_dir_vp);
                        return NO_ERROR;
                    }

                    /* We need to duplicate the path so far. */
                    for (point = 0; point < cur_len; point++)
                    {
                        ASSERT(fs_on_mp->elements[ fs_i_mp->elements[ cur_path_index ][ point ] ][ fs_j_mp->elements[ cur_path_index ][ point ] ] != 0);

                        fs_i_mp->elements[ fs_num_paths ][ point ] = fs_i_mp->elements[ cur_path_index ][ point ];
                        fs_j_mp->elements[ fs_num_paths ][ point ] = fs_j_mp->elements[ cur_path_index ][ point ];
                        fs_t_mp->elements[ fs_num_paths ][ point ] = fs_t_mp->elements[ cur_path_index ][ point ];
                    }

                    ASSERT(fs_on_mp->elements[ new_i ][ new_j ] != 0);

                    fs_i_mp->elements[ fs_num_paths ][ cur_len ] = new_i;
                    fs_j_mp->elements[ fs_num_paths ][ cur_len ] = new_j;
                    fs_t_mp->elements[ fs_num_paths ][ cur_len ] = ((dir%2)==0)?1.0:M_SQRT2;

                    fs_num_paths++;

                    /* We have to recurse now. */

                    ERE(get_all_paths_2(new_i, new_j, fs_num_paths - 1, 1 + cur_len, dir));
                }
            }

            fs_depth--;
            free_indexed_vector(sorted_dir_vp);
            return NO_ERROR;
        }
    }

    fs_len_vp->elements[ cur_path_index ] = cur_len;
    fs_depth--;
    free_indexed_vector(sorted_dir_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TEST
int verify_paths
(
    const Int_vector* len_vp,
    const Int_matrix* i_mp,
    const Int_matrix* j_mp
)
{
    int num_paths = len_vp->length;
    int path;
    int k, i, j;

    for (path = 0; path < num_paths; path++)
    {
        int len = len_vp->elements[ path ];
        int prev_i = i_mp->elements[ path ][ 0 ];
        int prev_j = j_mp->elements[ path ][ 0 ];

        for (k = 1; k < len; k++)
        {
            i = i_mp->elements[ path ][ k ];
            j = j_mp->elements[ path ][ k ];

            if ((ABS_OF(i - prev_i) > 1) || (ABS_OF(j - prev_j) > 1))
            {
                dbi(i_mp->elements[ path ][ 0 ]);
                dbi(j_mp->elements[ path ][ 1 ]);
                set_bug("Path verification failed.");
                return ERROR;
            }

            prev_i = i;
            prev_j = j;
        }
    }

    return NO_ERROR;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif





