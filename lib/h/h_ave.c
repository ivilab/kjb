
/* $Id: h_ave.c 21545 2017-07-23 21:57:31Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author). (Lindsay Martin
|  contributed to the documentation of this code).
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

#include "h/h_gen.h"      /*  Only safe if first #include in a ".c" file  */

#include "h/h_ave.h"
#include "g/g_geometry.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */

static int fs_hull_average_resolution      = 100;

/* -------------------------------------------------------------------------- */

static int approximate_2D_hull_average
(
    const Hull* hp,          /* Address of input hull. */
    int         resolution,  /* Number ofsearch  bins. */
    Vector**    average_vpp, /* Address of vector describing average. */
    double*     area_ptr
);

static int approximate_3D_hull_average
(
    const Hull* hp,          /* Address of input hull. */
    int         resolution,  /* Number of grid units in each dimension. */
    Vector**    average_vpp, /* Address of vector describing average. */
    double*     vol_ptr
);

static int get_2D_hull_CM_and_volume
(
    const Hull* hp,
    Vector**    average_vpp,
    double*     area_ptr
);

static int get_3D_hull_CM_and_volume
(
    const Hull* hp,
    Vector**    average_vpp,
    double*     vol_ptr
);

static int find_2D_weighted_hull_average
(
    const Hull* hp,
    int         resolution,
    double      (*weight_fn) (const Vector*),
    Vector**    average_vpp
);

static int find_3D_weighted_hull_average
(
    const Hull* hp,
    int         resolution,
    double      (*weight_fn) (const Vector*),
    Vector**    average_vpp
);

static int find_2D_constrained_hull_average
(
    const Hull* hp,
    const Hull* constraint_hp,
    int         (*map_fn) (Vector**, const Vector*),
    double      (*weight_fn) (const Vector*),
    int         resolution,
    Vector**    average_vpp,
    Vector**    max_prod_vpp,
    Hull**      result_hp_ptr
);

static int find_3D_constrained_hull_average
(
    const Hull* hp,
    const Hull* constraint_hp,
    int         (*map_fn) (Vector**, const Vector*),
    double      (*weight_fn) (const Vector*),
    int         resolution,
    Vector**    average_vpp,
    Vector**    max_prod_vpp,
    Hull**      result_hp_ptr
);

/* -------------------------------------------------------------------------- */

int set_hull_ave_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  temp_int_value;
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);


    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "har")
          || match_pattern(lc_option, "hull-average-resolution")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Hull average resolution is %d.\n",
                    fs_hull_average_resolution));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hull-average-resolution = %d\n",
                    fs_hull_average_resolution));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_hull_average_resolution = temp_int_value;
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           approximate_hull_average
 *
 * Finds the hull average.
 *
 * This routine finds and approximation of the average (center of mass) of the
 * input hull.
 *
 * The first argument "hp" is the address of the convex hull to compute
 * the average of. Only 2-D or 3-D hulls are currently supported.
 *
 * The second argument "average_vpp" contains the address of a vector
 * describing the computed hull average.
 *
 * Currently hulls averages are computed using a discrete approximation. The
 * resolution of the descrete space is user settable with the
 * "hull-average-resolution" option, provided that the KJB library options are
 * made available to the user (recommended!).
 *
 * Note:
 *     This routine is essentially OBSOLETE. It is kept for testing purposes,
 *     and as a template for routines to estimate more complex quantities. For
 *     most purposes, use get_hull_CM_and_volume().
 *
 * Returns:
 *    NO_ERROR on success: ERROR on failure.
 *
 * Related:
 *    Hull, get_hull_CM_and_volume
 *
 * Index: geometry, convex hulls
 *
 * Documentor:
 *    Lindsay Martin and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
*/
int approximate_hull_average
(
    const Hull* hp,
    Vector**    average_vpp,
    double*     vol_ptr
)
{
    int dim;


    dim = hp->dimension;

    if (*average_vpp != NULL)
    {
        if ((*average_vpp)->length != dim)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if (dim == 2)
    {
        return approximate_2D_hull_average(hp, fs_hull_average_resolution,
                                           average_vpp, vol_ptr);
    }
    else if (dim == 3)
    {
        return approximate_3D_hull_average(hp, fs_hull_average_resolution,
                                           average_vpp, vol_ptr);
    }
    else
    {
        set_bug("Average of %d-D hulls not supported.", dim);
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                  approximate_2D_hull_average
 * -----------------------------------------------------------------------------
*/
static int approximate_2D_hull_average
(
    const Hull* hp,
    int         resolution,
    Vector**    average_vpp,
    double*     area_ptr
)
{
    Matrix* normal_mp;
    Vector* constraint_vp;
    double**  normal_row_ptr;
    double*   normal_elem_ptr;
    double*   constraint_elem_ptr;
    int     dim, i, k;
    int     num_normals;
    double    min_x;
    double    max_x;
    double    min_y;
    double    max_y;
    double    x, y;
    double    x_step;
    double    hull_min_y;
    double    hull_max_y;
    double    temp;
    double    seg_len;
    double    area;
    Vector* average_vp;


    normal_mp = hp->normal_mp;

    dim = 2;
    num_normals = normal_mp->num_rows;

    min_x = hp->min_x;
    max_x = hp->max_x;

    min_y = hp->min_y;
    max_y = hp->max_y;

    constraint_vp = hp->b_value_vp;

    x_step = (max_x - min_x) / resolution;

    x = min_x;

    ERE(get_zero_vector(average_vpp, dim));
    average_vp = *average_vpp;

    area = 0.0;

    for (i=0; i<resolution+1; i++)
    {
        normal_row_ptr = normal_mp->elements;
        constraint_elem_ptr = constraint_vp->elements;
        hull_min_y = min_y;
        hull_max_y = max_y;

        for (k=0; k<num_normals; k++)
        {
            normal_elem_ptr = *normal_row_ptr;
            temp = (*constraint_elem_ptr) - (*normal_elem_ptr)*x;
            normal_elem_ptr++;

#ifdef HOW_IT_WAS
            if (IS_ZERO_DBL(*normal_elem_ptr))
#else
            if (    (*normal_elem_ptr > -HULL_EPSILON)
                 && (*normal_elem_ptr < HULL_EPSILON)
               )
#endif
            {
#ifdef HOW_IT_WAS
                if (temp < 0.0)         /* 13-02-99 */
#else
                if (temp < HULL_EPSILON)
#endif
                {
                    hull_min_y = DBL_MAX;   /* Force no solution */
                    break;
                }
                else
                {
                    /*EMPTY*/
                    ;  /* Do nothing: This normal is irrelavent */
                }
            }
            else if (*normal_elem_ptr > 0.0)
            {
                y = temp / (*normal_elem_ptr);

                if (y < hull_max_y)
                {
                    hull_max_y = y;

                    if (hull_min_y > hull_max_y)    /* 13-02-99 */
                    {
                        break;  /* No solution */
                    }
                }
            }
            else
            {
                y = temp / (*normal_elem_ptr);

                if (y > hull_min_y)
                {
                    hull_min_y = y;

                    if (hull_min_y > hull_max_y)     /* 13-02-99 */
                    {
                        break;  /* No solution */
                    }
                }
            }

            constraint_elem_ptr++;
            normal_row_ptr++;
        }

        if (hull_min_y <= hull_max_y)        /* 13-02-99 */
        {
            seg_len = hull_max_y - hull_min_y;
            area += seg_len;

            (average_vp->elements)[0] += (x * seg_len);
            (average_vp->elements)[1] += ((hull_max_y+hull_min_y)*seg_len)/2.0;
        }

         x += x_step;
     }

    (average_vp->elements)[0] /= area;
    (average_vp->elements)[1] /= area;

    if (area_ptr != NULL)
    {
        area *= x_step;
        *area_ptr = area;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                 approximate_3D_hull_average
 * -----------------------------------------------------------------------------
*/
static int approximate_3D_hull_average
(
    const Hull* hp,
    int         resolution,
    Vector**    average_vpp,
    double*     vol_ptr
)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Matrix*             normal_mp;
    Vector*             constraint_vp;
    double**              normal_row_ptr;
    double*               normal_elem_ptr;
    double*               constraint_elem_ptr;
    int                 dim, i, j, k, m;
    int                 num_normals;
    double                min_x;
    double                max_x;
    double                min_y;
    double                max_y;
    double                min_z;
    double                max_z;
    double                x, y;
    double                x_step;
    double                y_step;
    double                z;
    double                hull_min_z;
    double                hull_max_z;
    double                temp;
    Vector*             x_cache_vp;
    double*               x_cache_ptr;
    double                seg_len;
    double                vol;
    Vector*             average_vp;
    int                 result      = NO_ERROR;


    normal_mp = hp->normal_mp;

    dim = 3;
    num_normals = normal_mp->num_rows;

    min_x = hp->min_x;
    max_x = hp->max_x;

    min_y = hp->min_y;
    max_y = hp->max_y;

    min_z = hp->min_z;
    max_z = hp->max_z;

    constraint_vp = hp->b_value_vp;

    x_step = (max_x - min_x) / resolution;
    y_step = (max_y - min_y) / resolution;

    x = min_x;

    ERE(get_zero_vector(average_vpp, dim));
    average_vp = *average_vpp;

    NRE(x_cache_vp = create_vector(num_normals));

    vol = 0.0;

    for (i=0; i<resolution+1; i++)
    {
        if (io_atn_flag)
        {
            set_error("Processing interrupted.");
            halt_all_output = FALSE;
            result = ERROR;
            break;
        }

        x_cache_ptr = x_cache_vp->elements;
        normal_row_ptr = normal_mp->elements;

        for (m=0; m<num_normals; m++)
        {
            normal_elem_ptr = *normal_row_ptr;
            *x_cache_ptr = (*normal_elem_ptr) * x;
            x_cache_ptr++;
            normal_row_ptr++;
        }

        y = min_y;

        for (j=0; j<resolution+1; j++)
        {
            normal_row_ptr = normal_mp->elements;
            constraint_elem_ptr = constraint_vp->elements;
            x_cache_ptr = x_cache_vp->elements;

            hull_min_z = min_z;
            hull_max_z = max_z;

            for (k=0; k<num_normals; k++)
            {
                normal_elem_ptr = *normal_row_ptr;
                normal_elem_ptr++;
                temp = (*constraint_elem_ptr) - (*x_cache_ptr) -
                                                         (*normal_elem_ptr)*y;
                normal_elem_ptr++;

#ifdef HOW_IT_WAS
                if (IS_ZERO_DBL(*normal_elem_ptr))
#else
                if (    (*normal_elem_ptr > -HULL_EPSILON)
                     && (*normal_elem_ptr < HULL_EPSILON)
                   )
#endif
                {
#ifdef HOW_IT_WAS
                    if (temp < 0.0)       /* 13-02-99 */
#else
                    if (temp < HULL_EPSILON)
#endif
                    {
                        hull_min_z = DBL_MAX;  /* Force no solution */
                        break;
                    }
                    else
                    {
                        /*EMPTY*/
                        ;   /* Do nothing: This normal is irrelavent */
                    }
                }
                else if (*normal_elem_ptr > 0.0)
                {
                    z = temp / (*normal_elem_ptr);

                    if (z < hull_max_z)
                    {
                        hull_max_z = z;

                        if (hull_min_z > hull_max_z)    /* 13-02-99 */
                        {
                            break;  /* No solution */
                        }
                    }
                }
                else
                {
                    z = temp / (*normal_elem_ptr);

                    if (z > hull_min_z)
                    {
                        hull_min_z = z;

                        if (hull_min_z > hull_max_z)     /* 13-02-99 */
                        {
                            break;  /* No solution */
                        }
                    }
                }

                constraint_elem_ptr++;
                normal_row_ptr++;
                x_cache_ptr++;
            }

            if (hull_min_z <= hull_max_z)     /* 13-02-99 */
            {
                seg_len = hull_max_z - hull_min_z;
                vol += seg_len;

                (average_vp->elements)[0] += (x * seg_len);
                (average_vp->elements)[1] += (y * seg_len);
                (average_vp->elements)[2] +=
                                          ((hull_max_z+hull_min_z)*seg_len)/2.0;
            }
             y += y_step;
         }
         x += x_step;
    }

    if (result != ERROR)
    {
        (average_vp->elements)[0] /= vol;
        (average_vp->elements)[1] /= vol;
        (average_vp->elements)[2] /= vol;
    }

    if (vol_ptr != NULL)
    {
        vol *= x_step;
        vol *= y_step;
        *vol_ptr = vol;
    }

    free_vector(x_cache_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_hull_CM_and_volume
 *
 * Finds the hull average and volume.
 *
 * This routine finds the average (center of mass) of the input hull and its
 * volume.
 *
 * The first argument "hp" is the address of the convex hull to compute
 * the average of. Only 2-D or 3-D hulls are currently supported.
 *
 * The second argument "average_vpp" contains the address of a vector
 * for the computed hull average. If the hull average is not required, then it
 * can be NULL.
 *
 * The third argument "volume_ptr" is a pointer to the volume to be computed. If
 * the volume is not needed, this argument can be NULL.
 *
 * Returns:
 *    NO_ERROR on success: ERROR on failure.
 *
 * Related:
 *    Hull
 *
 * Index: geometry, convex hulls
 *
 * Documentor:
 *    Lindsay Martin and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
*/
int get_hull_CM_and_volume
(
    const Hull* hp,
    Vector**    average_vpp,
    double*     volume_ptr
)
{
    int dim;


    dim = hp->dimension;

    if (average_vpp != NULL)
    {
        if (*average_vpp != NULL)
        {
            if ((*average_vpp)->length != dim)
            {
                SET_ARGUMENT_BUG();
                return ERROR;
            }
        }
    }

    if (dim == 2)
    {
        return get_2D_hull_CM_and_volume(hp, average_vpp, volume_ptr);
    }
    else if (dim == 3)
    {
        return get_3D_hull_CM_and_volume(hp, average_vpp, volume_ptr);
    }
    else
    {
        set_bug("Facet average of %d-D hulls not supported.", dim);
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                  get_2D_hull_CM_and_volume
 * -----------------------------------------------------------------------------
*/
static int get_2D_hull_CM_and_volume
(
    const Hull* hp,
    Vector**    average_vpp,
    double*     area_ptr
)
{
    Matrix* vertex_mp;
    Matrix* normal_mp;
    int     i;
    int     num_facets;
    double    cx         = 0.0;
    double    cy         = 0.0;
    double    area       = 0.0;
    Vector* point_vp   = NULL;
    Vector* normal_vp  = NULL;
    Vector* line_to_facet_vp  = NULL;
    Vector* e1_vp      = NULL;
    Vector* e2_vp      = NULL;


    normal_mp = hp->normal_mp;
    vertex_mp = hp->vertex_mp;
    num_facets = hp->num_facets;

    ERE(get_target_vector(&point_vp, 2));

    point_vp->elements[ 0 ] = vertex_mp->elements[ 0 ][ 0 ];
    point_vp->elements[ 1 ] = vertex_mp->elements[ 0 ][ 1 ];

    for (i=0; i<num_facets; i++)
    {
        Matrix* facet_mp    = (hp->facets->elements)[i];
        double    facet_len;
        double    h;
        double    region_area;
        double    region_cx;
        double    region_cy;
        double    facet_mid_x;
        double    facet_mid_y;

        if ( ! vector_is_matrix_row(facet_mp, point_vp))
        {
            ERE(get_matrix_row(&e1_vp, facet_mp, 0));
            ERE(get_matrix_row(&e2_vp, facet_mp, 1));

            facet_len = vector_distance(e1_vp, e2_vp);

            ERE(subtract_vectors(&line_to_facet_vp, point_vp, e1_vp));
            ERE(get_matrix_row(&normal_vp, normal_mp, i));
            ERE(get_dot_product(line_to_facet_vp, normal_vp, &h));
            h = ABS_OF(h);

            region_area = facet_len * h / 2.0;

            facet_mid_x = (e1_vp->elements[ 0 ] + e2_vp->elements[ 0 ]) / 2.0;
            facet_mid_y = (e1_vp->elements[ 1 ] + e2_vp->elements[ 1 ]) / 2.0;

            region_cx = (2.0 * facet_mid_x + point_vp->elements[ 0 ]) / 3.0;
            region_cy = (2.0 * facet_mid_y + point_vp->elements[ 1 ]) / 3.0;

            area += region_area;
            cx += region_cx * region_area;
            cy += region_cy * region_area;
        }
    }

    free_vector(point_vp);
    free_vector(normal_vp);
    free_vector(line_to_facet_vp);
    free_vector(e1_vp);
    free_vector(e2_vp);

    if (average_vpp != NULL)
    {
        if (area < 100.0 * DBL_EPSILON)
        {
            /*
            // If the area is small, then at least some of the points are quite
            // close together, and likely the average of the corners is a
            // reasonable estimate.
            */
            UNTESTED_CODE();
            dbm("Small area noted in get_2D_hull_CM_and_volume.\n");
            ERE(average_matrix_rows(average_vpp, vertex_mp));
        }
        else
        {
            ERE(get_target_vector(average_vpp, 2));
            ((*average_vpp)->elements)[0] = cx / area;
            ((*average_vpp)->elements)[1] = cy / area;
        }
    }

    if (area_ptr != NULL)
    {
        *area_ptr = area;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                 get_3D_hull_CM_and_volume
 * -----------------------------------------------------------------------------
*/
static int get_3D_hull_CM_and_volume
(
    const Hull* hp,
    Vector**    average_vpp,
    double*     vol_ptr
)
{
    Matrix* vertex_mp;
    Matrix* normal_mp;
    int     i;
    int     num_facets;
    double  cx               = 0.0;
    double  cy               = 0.0;
    double  cz               = 0.0;
    double    vol              = 0.0;
    Vector* point_vp         = NULL;
    Vector* normal_vp        = NULL;
    Vector* line_to_facet_vp = NULL;
    Vector* cm_vp            = NULL;
    int     non_vertex_count = 0;


    normal_mp = hp->normal_mp;
    vertex_mp = hp->vertex_mp;
    num_facets = hp->num_facets;

#ifdef HOW_IT_WAS
    ERE(get_target_vector(&point_vp, 3));

    point_vp->elements[ 0 ] = vertex_mp->elements[ 0 ][ 0 ];
    point_vp->elements[ 1 ] = vertex_mp->elements[ 0 ][ 1 ];
    point_vp->elements[ 2 ] = vertex_mp->elements[ 0 ][ 2 ];
#else
    ERE(average_matrix_rows(&point_vp, vertex_mp));
#endif

    for (i=0; i<num_facets; i++)
    {
        Matrix* facet_mp   = (hp->facets->elements)[i];
        double    facet_area;
        double    h;
        double  region_vol;
        double    region_cx;
        double    region_cy;
        double    region_cz;

#ifdef HOW_IT_WAS
        if ( ! vector_is_matrix_row(facet_mp, point_vp))
        {
#endif
            non_vertex_count++;

            ERE(get_polygon_CM_and_area(facet_mp, &cm_vp, &facet_area));

            ERE(subtract_vectors(&line_to_facet_vp, cm_vp, point_vp));
            ERE(get_matrix_row(&normal_vp, normal_mp, i));
            ERE(get_dot_product(line_to_facet_vp, normal_vp, &h));

            ASSERT(h > -HULL_EPSILON);
            h = MAX_OF(h, 0.0);

            region_vol = facet_area * h / 3.0;

            region_cx = (3.0*cm_vp->elements[0] + point_vp->elements[0])/4.0;
            region_cy = (3.0*cm_vp->elements[1] + point_vp->elements[1])/4.0;
            region_cz = (3.0*cm_vp->elements[2] + point_vp->elements[2])/4.0;

            vol += region_vol;
            cx += region_cx * region_vol;
            cy += region_cy * region_vol;
            cz += region_cz * region_vol;
#ifdef HOW_IT_WAS
        }
#endif
    }

    free_vector(cm_vp);
    free_vector(point_vp);
    free_vector(normal_vp);
    free_vector(line_to_facet_vp);

    if (non_vertex_count == 0)
    {
        set_error("Problem finding hull average.");
#ifdef TEST
        dbi(non_vertex_count);
        dbe(vol);
        dbi(num_facets);

        db_mat(vertex_mp);

        for (i=0; i<num_facets; i++)
        {
            Matrix* facet_mp   = (hp->facets->elements)[i];
            db_mat(facet_mp);
        }

        kjb_abort();
#endif
        return ERROR;
    }

    if (average_vpp != NULL)
    {
        if (vol < 100.0 * DBL_EPSILON)
        {
            /*
            // If the area is small, then at least some of the points are quite
            // close together, and likely the average of the corners is a
            // reasonable estimate.
            */
            UNTESTED_CODE();
            dbm("Small area noted in get_3D_hull_CM_and_volume.\n");
            ERE(average_matrix_rows(average_vpp, vertex_mp));
        }
        else
        {
            ERE(get_target_vector(average_vpp, 3));
            ((*average_vpp)->elements)[0] = cx / vol;
            ((*average_vpp)->elements)[1] = cy / vol;
            ((*average_vpp)->elements)[2] = cz / vol;
        }
    }

    if (vol_ptr != NULL)
    {
        *vol_ptr = vol;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                         find_weighted_hull_average
 *
 * Finds a weighted hull average.
 *
 * This routine finds the point describing the average of the input hull
 * weighted by a user supplied call back function.
 *
 * The first argument "hp" is the address of the convex hull to compute
 * the average of.
 *
 * The second argument is a user supplied function to compute the weighting of a
 * point from a vector of its coordinates.
 *
 * The third argument "average_vpp" contains the address of a vector
 * to become thecomputed hull average.
 *
 * Currently hulls averages are computed using a discrete approximation. The
 * resolution of the descrete space is user settable with the "hir" option,
 * provided that the KJB library options are made available to the user
 * (recommended!).
 *
 * Returns:
 *    NO_ERROR on success: ERROR on failure.
 *
 * Related:
 *    approximate_hull_average, Hull
 *
 * Index: geometry, convex hulls
 *
 * Documentor:
 *    Lindsay Martin and Kobus Barnard
 *
 * -----------------------------------------------------------------------------
*/
int find_weighted_hull_average
(
    const Hull* hp,
    double      (*weight_fn) (const Vector *),
    Vector**    average_vpp
)
{
    int dim;


    dim = hp->dimension;

    if (*average_vpp != NULL)
    {
        if ((*average_vpp)->length != dim)
        {
            set_bug("Average vector length is not same as hull dimension.");
            return ERROR;
        }
    }

    if (dim == 2)
    {
        return find_2D_weighted_hull_average(hp, fs_hull_average_resolution,
                                             weight_fn, average_vpp);
    }
    else if (dim == 3)
    {
        return find_3D_weighted_hull_average(hp, fs_hull_average_resolution,
                                             weight_fn, average_vpp);
    }
    else
    {
        set_bug("Average of %d-D hulls not supported.", dim);
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC          find_2D_weighted_hull_average
 * -----------------------------------------------------------------------------
*/
static int find_2D_weighted_hull_average
(
    const Hull* hp,
    int         resolution,
    double      (*weight_fn) (const Vector *),
    Vector**    average_vpp
)
{
    Matrix* normal_mp;
    Vector* constraint_vp;
    double**  normal_row_ptr;
    double*   normal_elem_ptr;
    double*   constraint_elem_ptr;
    int     dim;
    int     i;
    int     j;
    int     k;
    int     num_normals;
    double    min_x;
    double    max_x;
    double    min_y;
    double    max_y;
    double    x;
    double    y;
    double    x_step;
    double    y_step;
    double    hull_min_y;
    double    hull_max_y;
    double    temp;
    Vector* average_vp;
    Vector* xy_vp = NULL;
    double    weight;
    double    weight_contribution;


    normal_mp = hp->normal_mp;

    dim = 2;
    num_normals = normal_mp->num_rows;

    min_x = hp->min_x;
    max_x = hp->max_x;

    min_y = hp->min_y;
    max_y = hp->max_y;

    constraint_vp = hp->b_value_vp;

    ERE(get_zero_vector(average_vpp, dim));
    average_vp = *average_vpp;

    ERE(get_target_vector(&xy_vp, dim));

    weight = 0.0;

    x_step = (max_x - min_x) / resolution;
    x = min_x;

    for (i=0; i<resolution + 1; i++)
    {
        normal_row_ptr = normal_mp->elements;
        constraint_elem_ptr = constraint_vp->elements;
        hull_min_y = min_y;
        hull_max_y = max_y;

        for (k=0; k<num_normals; k++)
        {
            normal_elem_ptr = *normal_row_ptr;
            temp = (*constraint_elem_ptr) - (*normal_elem_ptr)*x;
            normal_elem_ptr++;

#ifdef HOW_IT_WAS
            if (IS_ZERO_DBL(*normal_elem_ptr))
#else
            if (    (*normal_elem_ptr > -HULL_EPSILON)
                 && (*normal_elem_ptr < HULL_EPSILON)
               )
#endif
            {
#ifdef HOW_IT_WAS
                if (temp < 0.0)    /* 13-02-99 */
#else
                if (temp < HULL_EPSILON)
#endif
                {
                    hull_min_y = DBL_MAX;   /* Force no solution */
                    break;
                }
                else
                {
                    /*EMPTY*/
                    ;  /* Do nothing: This normal is irrelavent */
                }
            }
            else if (*normal_elem_ptr > 0.0)
            {
                y = temp / (*normal_elem_ptr);

                if (y < hull_max_y)
                {
                    hull_max_y = y;

                    if (hull_min_y > hull_max_y)    /* 13-02-99 */
                    {
                        break;  /* No solution */
                    }
                }
            }
            else
            {
                y = temp / (*normal_elem_ptr);

                if (y > hull_min_y)
                {
                    hull_min_y = y;

                    if (hull_min_y > hull_max_y)     /* 13-02-99 */
                    {
                        break;  /* No solution */
                    }
                }
            }

            constraint_elem_ptr++;
            normal_row_ptr++;
        }

        if (hull_min_y <= hull_max_y)    /* 13-02-99 */
        {
            y_step = (hull_max_y - hull_min_y) / ((double) resolution);
            y = hull_min_y;

            for (j=0; j<resolution + 1; j++)
            {
                xy_vp->elements[ 0 ] = x;
                xy_vp->elements[ 1 ] = y;
                weight_contribution = (*weight_fn)(xy_vp);

                (average_vp->elements)[0] += x * weight_contribution;
                (average_vp->elements)[1] += y * weight_contribution;
                weight += weight_contribution;
                y += y_step;
            }
        }

         x += x_step;
     }

    free_vector(xy_vp);

    (average_vp->elements)[0] /= weight;
    (average_vp->elements)[1] /= weight;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC          find_3D_weighted_hull_average
 * -----------------------------------------------------------------------------
*/
static int find_3D_weighted_hull_average
(
    const Hull* hp,
    int         resolution,
    double      (*weight_fn) (const Vector *),
    Vector**    average_vpp
)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Matrix*             normal_mp;
    Vector*             constraint_vp;
    double**              normal_row_ptr;
    double*               normal_elem_ptr;
    double*               constraint_elem_ptr;
    int                 dim, i, j, n, m;
    int                 num_normals;
    double                min_x;
    double                max_x;
    double                min_y;
    double                max_y;
    double                min_z;
    double                max_z;
    double                x, y, z;
    double                x_step;
    double                y_step;
    double                z_step;
    double                hull_min_z;
    double                hull_max_z;
    Vector*             x_cache_vp;
    double*               x_cache_ptr;
    double                weight;
    double                total_weight   = 0.0;
    Vector*             average_vp;
    Vector*             xyz_vp         = NULL;
    int                 solution_found = FALSE;
    int                 result         = NO_ERROR;


    normal_mp = hp->normal_mp;

    dim = 3;
    num_normals = normal_mp->num_rows;

    min_x = hp->min_x;
    max_x = hp->max_x;

    min_y = hp->min_y;
    max_y = hp->max_y;

    min_z = hp->min_z;
    max_z = hp->max_z;

    constraint_vp = hp->b_value_vp;

    x_step = (max_x - min_x) / resolution;
    y_step = (max_y - min_y) / resolution;
    z_step = (max_z - min_z) / resolution;

    x = min_x;

    ERE(get_zero_vector(average_vpp, dim));
    average_vp = *average_vpp;

    ERE(get_target_vector(&xyz_vp, dim));
    NRE(x_cache_vp = create_vector(num_normals));

    for (i=0; i<resolution+1; i++)
    {
        if (io_atn_flag)
        {
            set_error("Processing interrupted.");
            halt_all_output = FALSE;
            result = ERROR;
            break;
        }

        x_cache_ptr = x_cache_vp->elements;
        normal_row_ptr = normal_mp->elements;

        for (m=0; m<num_normals; m++)
        {
            normal_elem_ptr = *normal_row_ptr;
            *x_cache_ptr = (*normal_elem_ptr) * x;
            x_cache_ptr++;
            normal_row_ptr++;
        }

        y = min_y;

        for (j=0; j<resolution+1; j++)
        {
            if (io_atn_flag)
            {
                set_error("Processing interrupted.");
                halt_all_output = FALSE;
                result = ERROR;
                break;
            }

            normal_row_ptr = normal_mp->elements;
            constraint_elem_ptr = constraint_vp->elements;
            x_cache_ptr = x_cache_vp->elements;

            hull_min_z = min_z;
            hull_max_z = max_z;

            for (n=0; n<num_normals; n++)
            {
                double temp;

                normal_elem_ptr = *normal_row_ptr;
                normal_elem_ptr++;

                temp = (*constraint_elem_ptr) - (*x_cache_ptr) -
                                                           (*normal_elem_ptr)*y;

                normal_elem_ptr++;

#ifdef HOW_IT_WAS
                if (IS_ZERO_DBL(*normal_elem_ptr))
#else
                if (    (*normal_elem_ptr > -HULL_EPSILON)
                     && (*normal_elem_ptr < HULL_EPSILON)
                   )
#endif
                {
#ifdef HOW_IT_WAS
                    if (temp < 0.0)     /* 13-02-99 */
#else
                    if (temp < HULL_EPSILON)
#endif
                    {
                        hull_min_z = DBL_MAX;  /* Force no solution */
                        break;
                    }
                    else
                    {
                        /*EMPTY*/
                        ;   /* Do nothing: This normal is irrelavent */
                    }
                }
                else if (*normal_elem_ptr > 0.0)
                {
                    z = temp / (*normal_elem_ptr);

                    if (z < hull_max_z)
                    {
                        hull_max_z = z;

                        if (hull_min_z > hull_max_z)   /* 13-02-99 */
                        {
                            break;  /* No solution */
                        }
                    }
                }
                else
                {
                    z = temp / (*normal_elem_ptr);

                    if (z > hull_min_z)
                    {
                        hull_min_z = z;

                        if (hull_min_z > hull_max_z)   /* 13-02-99 */
                        {
                            break;  /* No solution */
                        }
                    }
                }

                constraint_elem_ptr++;
                normal_row_ptr++;
                x_cache_ptr++;
            }

            if (hull_min_z <= hull_max_z)     /* 13-02-99 */
            {
                z = hull_min_z;

                while (z < hull_max_z)
                {
                    solution_found = TRUE;

                    xyz_vp->elements[ 0 ] = x;
                    xyz_vp->elements[ 1 ] = y;
                    xyz_vp->elements[ 2 ] = z;

                    weight = (*weight_fn)(xyz_vp); ;

                    (average_vp->elements)[0] += x * weight;
                    (average_vp->elements)[1] += y * weight;
                    (average_vp->elements)[2] += z * weight;

                    total_weight += weight;

                    z += z_step;
                }
            }
            y += y_step;
         }

        if (result == ERROR) break;

        x += x_step;
    }

    if (result == ERROR)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else if (solution_found)
    {
        (average_vp->elements)[0] /= total_weight;
        (average_vp->elements)[1] /= total_weight;
        (average_vp->elements)[2] /= total_weight;
    }
    else
    {
        set_error("No hull points could be found for weighted average.");
        result = ERROR;
    }

    free_vector(xyz_vp);
    free_vector(x_cache_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        find_constrained_hull_average
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

int find_constrained_hull_average
(
    const Hull* hp,
    const Hull* constraint_hp,
    int         (*map_fn) (Vector **, const Vector *),
    double      (*weight_fn) (const Vector *),
    Vector**    average_vpp,
    Vector**    max_prod_vpp,
    Hull**      result_hp_ptr
)
{
    int dim;


    dim = hp->dimension;

    if (dim == 2)
    {
        return find_2D_constrained_hull_average(hp, constraint_hp, map_fn,
                                                weight_fn,
                                                fs_hull_average_resolution,
                                                average_vpp,
                                                max_prod_vpp,
                                                result_hp_ptr);
    }
    else if (dim == 3)
    {
        return find_3D_constrained_hull_average(hp, constraint_hp, map_fn,
                                                weight_fn,
                                                fs_hull_average_resolution,
                                                average_vpp,
                                                max_prod_vpp,
                                                result_hp_ptr);
    }
    else
    {
        set_bug("Constrained hull %d-D average is not supported.", dim);
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_2D_constrained_hull_average
(
    const Hull* hp,
    const Hull* constraint_hp,
    int         (*map_fn) (Vector **, const Vector *),
    double      (*weight_fn) (const Vector *),
    int         resolution,
    Vector**    average_vpp,
    Vector**    max_prod_vpp,
    Hull**      result_hp_ptr
)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Matrix*             normal_mp;
    Vector*             constraint_vp;
    double**              normal_row_ptr;
    double*               normal_elem_ptr;
    double*               constraint_elem_ptr;
    int                 dim, i, k;
    int                 num_normals;
    double                min_x;
    double                max_x;
    double                min_y;
    double                max_y;
    double                x, y;
    double                x_step;
    double                y_step;
    double                hull_min_y;
    double                hull_max_y;
    double                temp;
    Vector*             average_vp     = NULL;
    Vector*             test_vp        = NULL;
    Vector*             mapped_test_vp = NULL;
    Vector*             derived_test_vp;
    Matrix*             result_mp         = NULL;
    int                 result_matrix_row = 0;
    int                 result            = NO_ERROR;
    double                weight;
    double                total_weight      = 0.0;
    double                max_prod = 0.0;
    double                max_prod_x = DBL_NOT_SET; /* For gcc warning. */
    double                max_prod_y = DBL_NOT_SET; /* For gcc warning. */


    /*
    // Untested since removal of forced filling to make hull code able to
    // advertise constness when appropriate.
    */
    UNTESTED_CODE();

    dim = 2;
    normal_mp = hp->normal_mp;
    num_normals = normal_mp->num_rows;

    min_x = hp->min_x;
    max_x = hp->max_x;

    min_y = hp->min_y;
    max_y = hp->max_y;

    constraint_vp = hp->b_value_vp;

    x_step = (max_x - min_x) / resolution;
    y_step = (max_y - min_y) / resolution;

    x = min_x;

    ERE(get_zero_vector(&average_vp, dim));
    ERE(get_target_vector(&test_vp, dim));
    ERE(get_target_matrix(&result_mp, 2 * (1 + resolution), 2));

#ifdef CANT_AUTO_FILL_AND_ADVERTISE_CONST
    ERE(fill_hull(constraint_hp));
#endif

    for (i=0; i<resolution+1; i++)
    {
        if (result == ERROR) break;

        normal_row_ptr = normal_mp->elements;
        constraint_elem_ptr = constraint_vp->elements;
        hull_min_y = min_y;
        hull_max_y = max_y;

        for (k=0; k<num_normals; k++)
        {
            normal_elem_ptr = *normal_row_ptr;
            temp = (*constraint_elem_ptr) - (*normal_elem_ptr)*x;
            normal_elem_ptr++;

#ifdef HOW_IT_WAS
            if (IS_ZERO_DBL(*normal_elem_ptr))
#else
            if (    (*normal_elem_ptr > -HULL_EPSILON)
                 && (*normal_elem_ptr < HULL_EPSILON)
               )
#endif
            {
#ifdef HOW_IT_WAS
                if (temp < 0.0)     /* 13-02-99 */
#else
                if (temp < HULL_EPSILON)
#endif
                {
                    hull_min_y = DBL_MAX;   /* Force no solution */
                    break;
                }
                else
                {
                    /*EMPTY*/
                    ;  /* Do nothing: This normal is irrelavent */
                }
            }
            else if (*normal_elem_ptr > 0.0)
            {
                y = temp / (*normal_elem_ptr);

                if (y < hull_max_y)
                {
                    hull_max_y = y;

                    if (hull_min_y > hull_max_y)   /* 13-02-99 */
                    {
                        break;  /* No solution */
                    }
                }
            }
            else
            {
                y = temp / (*normal_elem_ptr);

                if (y > hull_min_y)
                {
                    hull_min_y = y;

                    if (hull_min_y > hull_max_y)    /* 13-02-99 */
                    {
                        break;  /* No solution */
                    }
                }
            }

            constraint_elem_ptr++;
            normal_row_ptr++;
        }

        if (hull_min_y <= hull_max_y)    /* 13-02-99 */
        {
            double result_min_y = DBL_NOT_SET;
            double result_max_y = DBL_NOT_SET;
            int  solution_found_in_this_segment = FALSE;

            y = hull_min_y;

            while (y < hull_max_y)
            {
                int inside_hull;

                test_vp->elements[ 0 ] = x;
                test_vp->elements[ 1 ] = y;

                if (map_fn != NULL)
                {
                    result = (*map_fn)(&mapped_test_vp, test_vp);
                    if (result == ERROR) break;
                    derived_test_vp = mapped_test_vp;
                }
                else
                {
                    derived_test_vp = test_vp;
                }

                if (result == NO_SOLUTION)
                {
                    inside_hull = FALSE;
                }
                else
                {
                    inside_hull = is_point_inside_hull(constraint_hp,
                                                       derived_test_vp);
                }

                if (inside_hull == ERROR)
                {
                    result = ERROR;
                    break;
                }
                else if (inside_hull == TRUE)
                {
                    if ( ! solution_found_in_this_segment)
                    {
                        solution_found_in_this_segment = TRUE;
                        result_min_y = y;
                        result_max_y = y;
                    }
                    else if (y > result_max_y) result_max_y = y;

                    if (weight_fn == NULL)
                    {
                        weight = 1.0;
                    }
                    else
                    {
                        test_vp->elements[ 0 ] = x;
                        test_vp->elements[ 1 ] = y;
                        weight = (*weight_fn)(test_vp);
                    }

                    (average_vp->elements)[0] += x * weight;
                    (average_vp->elements)[1] += y * weight;

                    if (max_prod_vpp != NULL)
                    {
                        if (ABS_OF(x * y) > max_prod)
                        {
                            max_prod_x = x;
                            max_prod_y = y;

                            max_prod = ABS_OF(x * y);
                        }
                    }

                    total_weight += weight;
                }
                y += y_step;
            }

            if (solution_found_in_this_segment)
            {
                result_mp->elements[ result_matrix_row ][0] = x;
                result_mp->elements[ result_matrix_row ][1] = result_min_y;

                result_matrix_row++;

                result_mp->elements[ result_matrix_row ][0] = x;
                result_mp->elements[ result_matrix_row ][1] = result_max_y;

                result_matrix_row++;
            }
        }

        x += x_step;

        if (io_atn_flag)
        {
            set_error("Processing interrupted.");
            halt_all_output = FALSE;
            result = ERROR;
            break;
        }

    }

    if (result_matrix_row == 0)
    {
        set_error("Hull and inverse hull intersection is empty.");
        result = NO_SOLUTION;
    }
    else
    {
        result_mp->num_rows = result_matrix_row;

        if (result_hp_ptr != NULL)
        {
            free_hull(*result_hp_ptr);
            *result_hp_ptr = find_convex_hull(result_mp, DEFAULT_HULL_OPTIONS);

            if (*result_hp_ptr == NULL) result = ERROR;
        }

        if (result != ERROR)
        {
            (average_vp->elements)[0] /= total_weight;
            (average_vp->elements)[1] /= total_weight;

            result = copy_vector(average_vpp, average_vp);
        }

        if ((result != ERROR) && (max_prod_vpp != NULL))
        {
            result = get_target_vector(max_prod_vpp, 2);

            if (result != ERROR)
            {
                (*max_prod_vpp)->elements[ 0 ] = max_prod_x;
                (*max_prod_vpp)->elements[ 1 ] = max_prod_y;
            }
        }
    }

    free_matrix(result_mp);
    free_vector(average_vp);
    free_vector(mapped_test_vp);
    free_vector(test_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_3D_constrained_hull_average
(
    const Hull* hp,
    const Hull* constraint_hp,
    int         (*map_fn) (Vector **, const Vector *),
    double      (*weight_fn) (const Vector *),
    int         resolution,
    Vector**    average_vpp,
    Vector**    max_prod_vpp,
    Hull**      result_hp_ptr
)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Matrix*             normal_mp;
    Vector*             constraint_vp;
    double**              normal_row_ptr;
    double*               normal_elem_ptr;
    double*               constraint_elem_ptr;
    int                 dim, i, j, n, m;
    int                 num_normals;
    double                min_x;
    double                max_x;
    double                min_y;
    double                max_y;
    double                min_z;
    double                max_z;
    double                x, y, z;
    double                x_step;
    double                y_step;
    double                z_step;
    double                hull_min_z;
    double                hull_max_z;
    Vector*             x_cache_vp     = NULL;
    double*               x_cache_ptr;
    double                weight;
    double                total_weight   = 0.0;
    Vector*             average_vp     = NULL;
    Vector*             test_vp        = NULL;
    Vector*             mapped_test_vp = NULL;
    Vector*             derived_test_vp;
    Matrix*             result_mp         = NULL;
    int                 result_matrix_row = 0;
    int                 solution_found    = FALSE;
    int                 result;
    double                max_prod = 0.0;
    double                max_prod_x = DBL_NOT_SET; /* For gcc warning. */
    double                max_prod_y = DBL_NOT_SET; /* For gcc warning. */
    double                max_prod_z = DBL_NOT_SET; /* For gcc warning. */


    dim = 3;
    normal_mp     = hp->normal_mp;
    num_normals   = normal_mp->num_rows;
    constraint_vp = hp->b_value_vp;

    min_x = hp->min_x;
    max_x = hp->max_x;

    min_y = hp->min_y;
    max_y = hp->max_y;

    min_z = hp->min_z;
    max_z = hp->max_z;

    x_step = (max_x - min_x) / resolution;
    y_step = (max_y - min_y) / resolution;
    z_step = (max_z - min_z) / resolution;

    result = get_target_matrix(&result_mp,
                               2 * (1 + resolution) * (1 + resolution), 3);

    if (result != ERROR)
    {
        result = get_zero_vector(&average_vp, dim);
    }

    if (result != ERROR)
    {
        result = get_target_vector(&test_vp, dim);
    }

    if (result != ERROR)
    {
        result = get_target_vector(&x_cache_vp, num_normals);
    }

#ifdef CANT_AUTO_FILL_AND_ADVERTISE_CONST
    if (result != ERROR)
    {
        result = fill_hull(constraint_hp);
    }
#endif

    x = min_x;

    for (i=0; i<resolution+1; i++)
    {
        if (result == ERROR) break;

        x_cache_ptr = x_cache_vp->elements;
        normal_row_ptr = normal_mp->elements;

        for (m=0; m<num_normals; m++)
        {
            normal_elem_ptr = *normal_row_ptr;
            *x_cache_ptr = (*normal_elem_ptr) * x;
            x_cache_ptr++;
            normal_row_ptr++;
        }

        y = min_y;

        for (j=0; j<resolution+1; j++)
        {
            if (result == ERROR) break;

            normal_row_ptr = normal_mp->elements;
            constraint_elem_ptr = constraint_vp->elements;
            x_cache_ptr = x_cache_vp->elements;

            hull_min_z = min_z;
            hull_max_z = max_z;

            for (n=0; n<num_normals; n++)
            {
                double temp;

                normal_elem_ptr = *normal_row_ptr;
                normal_elem_ptr++;

                temp = (*constraint_elem_ptr) - (*x_cache_ptr) -
                                                           (*normal_elem_ptr)*y;

                normal_elem_ptr++;

#ifdef HOW_IT_WAS
                if (IS_ZERO_DBL(*normal_elem_ptr))
#else
                if (    (*normal_elem_ptr > -HULL_EPSILON)
                     && (*normal_elem_ptr < HULL_EPSILON)
                   )
#endif
                {
#ifdef HOW_IT_WAS
                    if (temp < 0.0)     /* 13-02-99 */
#else
                    if (temp < HULL_EPSILON)
#endif
                    {
                        hull_min_z = DBL_MAX;  /* Force no solution */
                        break;
                    }
                    else
                    {
                        /*EMPTY*/
                        ;   /* Do nothing: This normal is irrelavent */
                    }
                }
                else if (*normal_elem_ptr > 0.0)
                {
                    z = temp / (*normal_elem_ptr);

                    if (z < hull_max_z)
                    {
                        hull_max_z = z;

                        if (hull_min_z > hull_max_z)     /* 13-02-99 */
                        {
                            break;  /* No solution */
                        }
                    }
                }
                else
                {
                    z = temp / (*normal_elem_ptr);

                    if (z > hull_min_z)
                    {
                        hull_min_z = z;

                        if (hull_min_z > hull_max_z)    /* 13-02-99 */
                        {
                            break;  /* No solution */
                        }
                    }
                }

                constraint_elem_ptr++;
                normal_row_ptr++;
                x_cache_ptr++;
            }

            if (hull_min_z <= hull_max_z)     /* 13-02-99 */
            {
                double result_min_z = DBL_NOT_SET;
                double result_max_z = DBL_NOT_SET;
                int  solution_found_in_this_segment = FALSE;

                z = hull_min_z;

                while (z < hull_max_z)
                {
                    int inside_hull;

                    test_vp->elements[ 0 ] = x;
                    test_vp->elements[ 1 ] = y;
                    test_vp->elements[ 2 ] = z;

                    if (map_fn != NULL)
                    {
                        result = (*map_fn)(&mapped_test_vp, test_vp);
                        if (result == ERROR) break;
                        derived_test_vp = mapped_test_vp;
                    }
                    else
                    {
                        derived_test_vp = test_vp;
                    }

                    if (result == NO_SOLUTION)
                    {
                        inside_hull = FALSE;
                    }
                    else
                    {
                        inside_hull = is_point_inside_hull(constraint_hp,
                                                           derived_test_vp);
                    }

                    if (inside_hull == ERROR)
                    {
                        result = ERROR;
                        break;
                    }
                    else if (inside_hull == TRUE)
                    {
                        if ( ! solution_found_in_this_segment)
                        {
                            solution_found_in_this_segment = TRUE;
                            result_min_z = z;
                            result_max_z = z;
                        }
                        else if (z > result_max_z) result_max_z = z;

                        if (weight_fn == NULL)
                        {
                            weight = 1.0;
                        }
                        else
                        {
                            test_vp->elements[ 0 ] = x;
                            test_vp->elements[ 1 ] = y;
                            test_vp->elements[ 2 ] = z;
                            weight = (*weight_fn)(test_vp);
                        }

                        (average_vp->elements)[0] += x * weight;
                        (average_vp->elements)[1] += y * weight;
                        (average_vp->elements)[2] += z * weight;

                        if (max_prod_vpp != NULL)
                        {
                            if (ABS_OF(x * y * z) > max_prod)
                            {
                                max_prod_x = x;
                                max_prod_y = y;
                                max_prod_z = z;

                                max_prod = ABS_OF(x * y * z);
                            }
                        }

                        total_weight += weight;
                    }
                    z += z_step;
                }

                if (solution_found_in_this_segment)
                {
                    solution_found = TRUE;

                    result_mp->elements[ result_matrix_row ][0] = x;
                    result_mp->elements[ result_matrix_row ][1] = y;
                    result_mp->elements[ result_matrix_row ][2] = result_min_z;

                    result_matrix_row++;

                    result_mp->elements[ result_matrix_row ][0] = x;
                    result_mp->elements[ result_matrix_row ][1] = y;
                    result_mp->elements[ result_matrix_row ][2] = result_max_z;

                    result_matrix_row++;
                }
            }
            y += y_step;

            if (io_atn_flag)
            {
                set_error("Processing interrupted.");
                halt_all_output = FALSE;
                result = ERROR;
            }
         }

        x += x_step;
    }

    if (result == ERROR)
    {
        /*EMPTY*/
        ; /* Do nothing */
    }
    else if (solution_found)
    {
        (average_vp->elements)[0] /= total_weight;
        (average_vp->elements)[1] /= total_weight;
        (average_vp->elements)[2] /= total_weight;

        result_mp->num_rows = result_matrix_row;

        if (result_hp_ptr != NULL)
        {
            free_hull(*result_hp_ptr);
            *result_hp_ptr = find_convex_hull(result_mp, DEFAULT_HULL_OPTIONS);

            if (*result_hp_ptr == NULL) result = ERROR;
        }

        if (result != ERROR)
        {
            result = copy_vector(average_vpp, average_vp);
        }

        if ((result != ERROR) && (max_prod_vpp != NULL))
        {
            result = get_target_vector(max_prod_vpp, 3);

            if (result != ERROR)
            {
                (*max_prod_vpp)->elements[ 0 ] = max_prod_x;
                (*max_prod_vpp)->elements[ 1 ] = max_prod_y;
                (*max_prod_vpp)->elements[ 2 ] = max_prod_z;
            }
        }
    }
    else
    {
        set_error("No solution to constrained hull average.");
        result = NO_SOLUTION;
    }

    free_vector(test_vp);
    free_vector(mapped_test_vp);
    free_vector(average_vp);
    free_vector(x_cache_vp);
    free_matrix(result_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

