
/* $Id: v_illum.c 4727 2009-11-16 20:53:54Z kobus $ */

/*
     Copyright (c) 1994-2008 by Kobus Barnard (author).

     Personal and educational use of this code is granted, provided
     that this header is kept intact, and that the authorship is not
     misrepresented. Commercial use is not permitted.
*/

#include "i/i_gen.h"     /* Only safe as first include in a ".c" file. */
#include "c/c_projection.h"
#include "v/v_illum.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_allocated_static_data(void);
    static void prepare_memory_cleanup(void);
#endif

/* -------------------------------------------------------------------------- */

static int*** fs_shadow_jumps = NULL;
static int**  fs_illum_jumps  = NULL;

/* -------------------------------------------------------------------------- */

int is_possible_illum_change
(
    double R_illum,
    double G_illum,
    double B_illum,
    double R_non_illum,
    double G_non_illum,
    double B_non_illum,
    double relative_error
)
{
    double r_ratio;
    double g_ratio;
    int is, js, ks, in, jn, kn;
    int ok;
    double sum_illum = R_illum + G_illum + B_illum;
    double sum_non_illum = R_non_illum + G_non_illum + B_non_illum;


    r_ratio = (R_illum / sum_illum) / (R_non_illum / sum_non_illum);
    g_ratio = (G_illum / sum_illum) / (G_non_illum / sum_non_illum);

    ERE(ok = is_possible_illum_ratio(r_ratio, g_ratio));
    if (ok) return TRUE;

    for (is = -1; is <= 1; is += 2)
    {
        double rs = R_illum * (1.0 + is * relative_error);

        for (in = -1; in <= 1; in += 2)
        {
            double rn = R_non_illum * (1.0 + in * relative_error);

            for (js = -1; js <= 1; js += 2)
            {
                double gs = G_illum * (1.0 + js * relative_error);

                for (jn = -1; jn <= 1; jn += 2)
                {
                    double gn = G_non_illum * (1.0 + jn * relative_error);

                    for (ks = -1; ks <= 1; ks += 2)
                    {
                        double bs = B_illum * (1.0 + ks * relative_error);

                        for (kn = -1; kn <= 1; kn += 2)
                        {
                            double bn = B_non_illum * (1.0 + kn * relative_error);

                            sum_illum = rs + gs + bs;
                            sum_non_illum = rn + gn + bn;

                            r_ratio = (rs / sum_illum) / (rn / sum_non_illum);
                            g_ratio = (gs / sum_illum) / (gn / sum_non_illum);

                            ERE(ok = is_possible_illum_ratio(r_ratio, g_ratio));
                            if (ok) return TRUE;
                        }
                    }
                }
            }
        }
    }

    return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int is_possible_illum_ratio(double r_ratio, double g_ratio)
{
    int i, j;
    FILE*      valid_jump_fp;
    static int num_jump_steps;


#define ILLUM_JUMP_STEPS 500
#define ILLUM_RG_STEP    0.002
#define ILLUM_JUMP_FILE "ILLUM_JUMPS"
#define ILLUM_R_FACTOR  4.5
#define ILLUM_G_FACTOR  2.0



    if ((r_ratio < 0.0) || (g_ratio < 0.0))
    {
        return FALSE;
    }

    r_ratio /= ILLUM_R_FACTOR;
    g_ratio /= ILLUM_G_FACTOR;

    if ((r_ratio > 1.0) || (g_ratio > 1.0))
    {
        return FALSE;
    }

    if (fs_illum_jumps == NULL)
    {
        valid_jump_fp = kjb_fopen(ILLUM_JUMP_FILE, "r");

        dbx(valid_jump_fp);

        if (valid_jump_fp != NULL)
        {
            char line[ 1000 ];
            int  read_res;

            ERE(read_res = BUFF_FGET_LINE(valid_jump_fp, line));

            if (read_res == EOF)
            {
                set_error("Unexpected EOF reading %F.\n", valid_jump_fp);
                kjb_fclose(valid_jump_fp);
                return ERROR;
            }
            ERE(ss1pi(line, &num_jump_steps));
        }
        else
        {
            num_jump_steps = ILLUM_JUMP_STEPS;
        }


#ifdef TRACK_MEMORY_ALLOCATION
        prepare_memory_cleanup();
#endif

        NRE(fs_illum_jumps = allocate_2D_int_array(num_jump_steps, num_jump_steps));

        if (valid_jump_fp != NULL)
        {
            char line[ 1000 ];
            int  read_res;

            for (i=0; i<num_jump_steps; i++)
            {
                for (j=0; j<num_jump_steps; j++)
                {
                    ERE(read_res = BUFF_FGET_LINE(valid_jump_fp, line));

                    if (read_res == EOF)
                    {
                        set_error("Unexpected EOF reading %F.\n", valid_jump_fp);
                        kjb_fclose(valid_jump_fp);
                        return ERROR;
                    }
                    ERE(ss1pi(line, &(fs_illum_jumps[i][j])));
                }
            }
            kjb_fclose(valid_jump_fp);
        }
        else
        {
            Matrix* illum_RGB_mp = NULL;
            Matrix* illum_rg_mp = NULL;
            Hull* illum_rg_hp = NULL;
            Vector* rg_vp = NULL;
            double r1, g1;
            int illum_rg_count = 0;
            const int max_num_illum_rg = kjb_rint(1.0 + 1.0 / (ILLUM_RG_STEP * ILLUM_RG_STEP));
            int c1, c2;
            double max_r_ratio = 0.0, max_g_ratio = 0.0;

            for (i=0; i<num_jump_steps; i++)
            {
                for (j=0; j<num_jump_steps; j++)
                {
                    fs_illum_jumps[ i ][ j ] = FALSE;
                }
            }

            ERE(read_matrix(&illum_RGB_mp, "illum_db.rgb"));
            ERE(divide_by_sum_project_matrix(&illum_rg_mp, illum_RGB_mp));
            ERE(get_convex_hull(&illum_rg_hp, illum_rg_mp,
                                DEFAULT_HULL_OPTIONS));
            ERE(get_target_vector(&rg_vp, 2));

            /* Recycle illum_rg_mp */
            ERE(get_target_matrix(&illum_rg_mp, max_num_illum_rg, 2));

            for (r1 = 0.0; r1 < 1.0; r1 += ILLUM_RG_STEP)
            {
                for (g1 = 0.0; g1 < 1.0 - r1; g1 += ILLUM_RG_STEP)
                {
                    rg_vp->elements[ 0 ] = r1;
                    rg_vp->elements[ 1 ] = g1;

                    if (is_point_inside_hull(illum_rg_hp, rg_vp))
                    {
                        illum_rg_mp->elements[ illum_rg_count ][ 0 ] = r1;
                        illum_rg_mp->elements[ illum_rg_count ][ 1 ] = g1;
                        illum_rg_count++;
                    }
                }
            }

            illum_rg_mp->num_rows = illum_rg_count;

            for (c1 = 0; c1 < illum_rg_count; c1++)
            {
                r1 = illum_rg_mp->elements[ c1 ][ 0 ];
                g1 = illum_rg_mp->elements[ c1 ][ 1 ];

                for (c2 = 0; c2 < illum_rg_count; c2++)
                {
                    double r2 = illum_rg_mp->elements[ c2 ][ 0 ];
                    double g2 = illum_rg_mp->elements[ c2 ][ 1 ];

                    i = kjb_rint(num_jump_steps * r1 / (r2 * ILLUM_R_FACTOR));
                    j = kjb_rint(num_jump_steps * g1 / (g2 * ILLUM_G_FACTOR));

                    if (i >= num_jump_steps) i = num_jump_steps - 1;
                    if (j >= num_jump_steps) j = num_jump_steps - 1;

                    fs_illum_jumps[ i ][ j ] = TRUE;

                    if (r1/r2 > max_r_ratio) max_r_ratio = r1/r2;
                    if (g1/g2 > max_g_ratio) max_g_ratio = g1/g2;
                }
            }

            dbf(max_r_ratio);
            dbf(max_g_ratio);

            NRE(valid_jump_fp = kjb_fopen(ILLUM_JUMP_FILE, "w"));

            ERE(kjb_fprintf(valid_jump_fp, "%d\n", num_jump_steps));

            for (i=0; i<num_jump_steps; i++)
            {
                for (j=0; j<num_jump_steps; j++)
                {
                    ERE(kjb_fprintf(valid_jump_fp, "%d\n", fs_illum_jumps[i][j]));
                }
            }

            ERE(kjb_fclose(valid_jump_fp));
        }
    }

    i = kjb_rint(r_ratio * num_jump_steps);
    j = kjb_rint(g_ratio * num_jump_steps);

    if (i >= num_jump_steps) i = num_jump_steps - 1;
    if (j >= num_jump_steps) j = num_jump_steps - 1;

    return fs_illum_jumps[ i ][ j ];
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int is_possible_shadow_change
(
    double R_shadow,
    double G_shadow,
    double B_shadow,
    double R_non_shadow,
    double G_non_shadow,
    double B_non_shadow,
    double relative_error
)
{
    double R_ratio;
    double G_ratio;
    double B_ratio;
    int is, js, ks, in, jn, kn;
    int ok;


    R_ratio = R_shadow / R_non_shadow;
    G_ratio = G_shadow / G_non_shadow;
    B_ratio = B_shadow / B_non_shadow;

    /*
    // We only consider errors due to things like the diagonal model. If this
    // main condition does not hold, then we don't accept the ratio, regardless
    // of the error;
     */
    if ((R_ratio > 1.0) || (G_ratio > 1.0) || (B_ratio > 1.0))
    {
        return FALSE;
    }

    if ((R_ratio < 0.0) || (G_ratio < 0.0) || (B_ratio < 0.0))
    {
        return FALSE;
    }

    ERE(ok = is_possible_shadow_ratio(R_ratio, G_ratio, B_ratio));
    if (ok) return TRUE;

    for (is = -1; is <= 1; is += 2)
    {
        double rs = R_shadow * (1.0 + is * relative_error);

        for (in = -1; in <= 1; in += 2)
        {
            double rn = R_non_shadow * (1.0 + in * relative_error);

            for (js = -1; js <= 1; js += 2)
            {
                double gs = G_shadow * (1.0 + js * relative_error);

                for (jn = -1; jn <= 1; jn += 2)
                {
                    double gn = G_non_shadow * (1.0 + jn * relative_error);

                    for (ks = -1; ks <= 1; ks += 2)
                    {
                        double bs = B_shadow * (1.0 + ks * relative_error);

                        for (kn = -1; kn <= 1; kn += 2)
                        {
                            double bn = B_non_shadow * (1.0 + kn * relative_error);

                            ERE(ok = is_possible_shadow_ratio(rs/rn, gs/gn, bs/bn));
                            if (ok) return TRUE;
                        }
                    }
                }
            }
        }
    }

    return FALSE;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int is_possible_shadow_ratio
(
    double R_ratio,
    double G_ratio,
    double B_ratio
)
{
    int i, j, k;
    FILE*      valid_jump_fp;
    static int num_jump_steps;


#define SHADOW_RG_STEP 0.005
#define SHADOW_JUMP_STEPS 200
#define SHADOW_JUMP_FILE "SHADOW_JUMPS"
#define NUM_SHADOW_LUM_STEPS  (3 * SHADOW_JUMP_STEPS)

    if ((R_ratio > 1.0) || (G_ratio > 1.0) || (B_ratio > 1.0))
    {
        return FALSE;
    }

    if ((R_ratio < 0.0) || (G_ratio < 0.0) || (B_ratio < 0.0))
    {
        return FALSE;
    }

    if (fs_shadow_jumps == NULL)
    {
        valid_jump_fp = kjb_fopen(SHADOW_JUMP_FILE, "r");

        dbx(valid_jump_fp);

        if (valid_jump_fp != NULL)
        {
            char line[ 1000 ];
            int  read_res;

            ERE(read_res = BUFF_FGET_LINE(valid_jump_fp, line));

            if (read_res == EOF)
            {
                set_error("Unexpected EOF reading %F.\n", valid_jump_fp);
                kjb_fclose(valid_jump_fp);
                return ERROR;
            }
            ERE(ss1pi(line, &num_jump_steps));
        }
        else
        {
            num_jump_steps = SHADOW_JUMP_STEPS;
        }


#ifdef TRACK_MEMORY_ALLOCATION
        prepare_memory_cleanup();
#endif

        NRE(fs_shadow_jumps = allocate_3D_int_array(num_jump_steps, num_jump_steps,
                                                num_jump_steps));

        if (valid_jump_fp != NULL)
        {
            char line[ 1000 ];
            int  read_res;

            for (i=0; i<num_jump_steps; i++)
            {
                for (j=0; j<num_jump_steps; j++)
                {
                    for (k=0; k<num_jump_steps; k++)
                    {
                        ERE(read_res = BUFF_FGET_LINE(valid_jump_fp, line));

                        if (read_res == EOF)
                        {
                            set_error("Unexpected EOF reading %F.\n", valid_jump_fp);
                            kjb_fclose(valid_jump_fp);
                            return ERROR;
                        }
                        ERE(ss1pi(line, &(fs_shadow_jumps[i][j][k])));
                    }
                }
            }
            kjb_fclose(valid_jump_fp);
        }
        else
        {
            Matrix* illum_RGB_mp = NULL;
            Matrix* illum_rg_mp = NULL;
            Hull* illum_rg_hp = NULL;
            Vector* rg_vp = NULL;
            double r1, g1, b1;
            int illum_rg_count = 0;
            const int max_num_illum_rg = kjb_rint(1.0 + 1.0 / (SHADOW_RG_STEP * SHADOW_RG_STEP));
            int c1, c2;

            for (i=0; i<num_jump_steps; i++)
            {
                for (j=0; j<num_jump_steps; j++)
                {
                    for (k=0; k<num_jump_steps; k++)
                    {
                        fs_shadow_jumps[ i ][ j ][ k ] = FALSE;
                    }
                }
            }

            ERE(read_matrix(&illum_RGB_mp, "illum_db.rgb"));
            ERE(divide_by_sum_project_matrix(&illum_rg_mp, illum_RGB_mp));
            ERE(get_convex_hull(&illum_rg_hp, illum_rg_mp,
                                DEFAULT_HULL_OPTIONS));
            ERE(get_target_vector(&rg_vp, 2));

            /* Recycle illum_rg_mp */
            ERE(get_target_matrix(&illum_rg_mp, max_num_illum_rg, 2));

            for (r1 = 0.0; r1 < 1.0; r1 += SHADOW_RG_STEP)
            {
                for (g1 = 0.0; g1 < 1.0 - r1; g1 += SHADOW_RG_STEP)
                {
                    rg_vp->elements[ 0 ] = r1;
                    rg_vp->elements[ 1 ] = g1;

                    if (is_point_inside_hull(illum_rg_hp, rg_vp))
                    {
                        illum_rg_mp->elements[ illum_rg_count ][ 0 ] = r1;
                        illum_rg_mp->elements[ illum_rg_count ][ 1 ] = g1;
                        illum_rg_count++;
                    }
                }
            }

            illum_rg_mp->num_rows = illum_rg_count;

            for (c1 = 0; c1 < illum_rg_count; c1++)
            {
                r1 = illum_rg_mp->elements[ c1 ][ 0 ];
                g1 = illum_rg_mp->elements[ c1 ][ 1 ];
                b1 = 1.0 - r1 - g1;

                for (c2 = 0; c2 < illum_rg_count; c2++)
                {
                    double r2 = illum_rg_mp->elements[ c2 ][ 0 ];
                    double g2 = illum_rg_mp->elements[ c2 ][ 1 ];
                    double b2 = 1.0 - r2 - g2;
                    int count;
                    double f_step = 1.0 /  NUM_SHADOW_LUM_STEPS;
                    double f = 0.0;

                    /*
                    // HACK factor of 20 for the max ratio of illum brightness.
                    // More than that, and the ratio will be definitely close to
                    // (0,0,0), so no need to do more.
                    */
                    for (count = 0; count < 5 * NUM_SHADOW_LUM_STEPS; count++)
                    {
                        i = kjb_rint(num_jump_steps * r1 / (f * r2 + r1));
                        j = kjb_rint(num_jump_steps * g1 / (f * g2 + g1));
                        k = kjb_rint(num_jump_steps * b1 / (f * b2 + b1));

                        if (i >= num_jump_steps) i = num_jump_steps - 1;
                        if (j >= num_jump_steps) j = num_jump_steps - 1;
                        if (k >= num_jump_steps) k = num_jump_steps - 1;

                        fs_shadow_jumps[ i ][ j ][ k ] = TRUE;

                        f += f_step;
                    }
                }
            }

            NRE(valid_jump_fp = kjb_fopen(SHADOW_JUMP_FILE, "w"));

            ERE(kjb_fprintf(valid_jump_fp, "%d\n", num_jump_steps));

            for (i=0; i<num_jump_steps; i++)
            {
                for (j=0; j<num_jump_steps; j++)
                {
                    for (k=0; k<num_jump_steps; k++)
                    {
                        ERE(kjb_fprintf(valid_jump_fp, "%d\n", fs_shadow_jumps[i][j][k]));
                    }
                }
            }

            ERE(kjb_fclose(valid_jump_fp));
        }
    }

    i = kjb_rint(R_ratio * SHADOW_JUMP_STEPS);
    j = kjb_rint(G_ratio * SHADOW_JUMP_STEPS);
    k = kjb_rint(B_ratio * SHADOW_JUMP_STEPS);

    if (i == SHADOW_JUMP_STEPS) i--;
    if (j == SHADOW_JUMP_STEPS) j--;
    if (k == SHADOW_JUMP_STEPS) k--;

    return fs_shadow_jumps[ i ][ j ][ k ];
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void prepare_memory_cleanup(void)
{
    static int first_time = TRUE;


    if (first_time)
    {
        add_cleanup_function(free_allocated_static_data);
        first_time = FALSE;
    }
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void free_allocated_static_data(void)
{
    free_3D_int_array(fs_shadow_jumps);
    free_2D_int_array(fs_illum_jumps);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

