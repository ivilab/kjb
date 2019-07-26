
/* $Id: h_misc.c 21545 2017-07-23 21:57:31Z kobus $ */

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

#include "h/h_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "h/h_misc.h"
#include "g/g_geometry.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/*
// Orginally, we expanded hulls exactly, which is fast and logical, but leads to
// precision problems under the normal use. Thus I coded up expansion which is
// slower, and not exaclty what is wanted, but does not increase the number of
// hull facets. Because it is so slow, we leave the default as before. Note that
// some of the precision problems have since been mitigated by other means.
*/
static int fs_expand_hulls_exactly = TRUE;

/* -------------------------------------------------------------------------- */

static int get_distance_to_2D_hull
(
    const Hull*   hp,
    const Vector* test_vp,
    double*       distance_ptr
);

static int get_distance_to_3D_hull
(
    const Hull*   hp,
    const Vector* test_vp,
    double*       distance_ptr
);

/* -------------------------------------------------------------------------- */

int set_hull_misc_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "expand-hulls-exactly")
          || match_pattern(lc_option, "exact-hull-expansion")
       )
    {
        int temp_boolean_value;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Convex hulls %s expanded exactly.\n",
                    fs_expand_hulls_exactly ? "are" : "are not"));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("expand-hulls-exactly = %s\n",
                    fs_expand_hulls_exactly ? "t" : "f"));
        }
        else
        {
            ERE(temp_boolean_value = get_boolean_value(value));
            fs_expand_hulls_exactly = temp_boolean_value;
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                      get_interior_distance_to_hull
 *
 * Determines the distance of an interior point to hull
 *
 * This function computes the distance of a point to a hull boundary on the
 * assumption that the point is inside the hull. If the point is in fact outside
 * the hull, then the distance will be set to a negative value, which has no
 * meaning other than the point is outside the hull. If the distance of an
 * exterior point to the hull is needed, then use the function
 * get_distance_to_hull which computes both.
 *
 * The first argument "hp" is the address of the convex hull
 *
 * The second argument "test_vp" contains the address of a the query location.
 *
 * The argument distance_ptr is a pointer to the distance to be computed.
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure with an error message being set.
 *    Note that it is not an error to query about a point that is outside the
 *    hull. The negative distance is a valid indicator of this, and is useful
 *    for some calling routines.
 *
 * Note:
 *    If the point is outside the hull, then the distance will be negative, but
 *    the magnitude of the negative distance has no meaning. The distance from
 *    an exterior point to the hull is NOT computed--use get_distance_to_hull()
 *    for that.
 *
 * Note:
 *    The point of using this routine instead of get_distance_to_hull() is that
 *    this one is faster and generalizes to all dimensions. Hence if the extra
 *    flexibility of get_distance_to_hull() is not required, then use this
 *    routined.
 *
 * Note:
 *    The sense (sign) of the distance is the opposite of that computed by
 *    get_distance_to_hull().
 *
 * Related:
 *    Hull, get_distance_to_hull
 *
 * Index: geometry, convex hulls
 *
 * -----------------------------------------------------------------------------
*/
int get_interior_distance_to_hull
(
    const Hull*   hp,
    const Vector* test_vp,
    double*       distance_ptr
)
{
    Vector* constraint_vp;
    Vector* n_dot_x_vp     = NULL;
    Matrix* normal_mp;
    int     i;
    double    d;
    double    min_d;


    if (hp->dimension != test_vp->length)
    {
        set_bug("Test point and hull dimensions disagree in get_interior_distance_to_hull.");
        return FALSE;
    }

    if (hp->normal_mp == NULL)
    {
        set_bug("Hulls sent to interior_distance must have HULL_NORMALS.");
        return ERROR;
    }

    if (hp->b_value_vp == NULL)
    {
        set_bug("Hulls sent to interior_distance must have HULL_B_VALUES.");
        return ERROR;
    }

    constraint_vp = hp->b_value_vp;
    normal_mp = hp->normal_mp;

    if (multiply_matrix_and_vector(&n_dot_x_vp, normal_mp, test_vp)==ERROR)
    {
        kjb_print_error();
        set_bug("Unable to multiply matrix and vector in interior_distance.");
        return ERROR;
    }

    min_d = constraint_vp->elements[0] - n_dot_x_vp->elements[0];

    for (i=1; i<constraint_vp->length; i++)
    {
        d = constraint_vp->elements[i] - n_dot_x_vp->elements[i];

        if (d < min_d) min_d = d;
    }

    *distance_ptr = min_d;

    free_vector(n_dot_x_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           get_distance_to_hull
 *
 * Determines the distance of a point to hull
 *
 * This function computes the distance of a point to a hull boundary. If the
 * point is outside the hull, the distance is positive, and if it is inside the
 * hull, the distance is negative.
 *
 * The first argument "hp" is the address of the convex hull
 *
 * The second argument "test_vp" contains the address of a the query location.
 *
 * The argument distance_ptr is a pointer to the distance to be computed.
 *
 * Warning:
 *    This routine is computationally expensive, and currently not all that
 *    exact either (likely due to bugs)!
 *
 * Returns:
 *    NO_ERROR on success, and ERROR on failure with an error message being set.
 *
 * Related:
 *    Hull, get_interior_distance_to_hull
 *
 * Index: geometry, convex hulls
 *
 * -----------------------------------------------------------------------------
*/
int get_distance_to_hull
(
    const Hull*   hp,
    const Vector* test_vp,
    double*       distance_ptr
)
{


    if (hp->dimension != test_vp->length)
    {
        set_bug("Test point and hull dimensions disagree in get_distance_to_hull.");
        return FALSE;
    }

    if (hp->dimension == 2)
    {
        return get_distance_to_2D_hull(hp, test_vp, distance_ptr);
    }
    else if (hp->dimension == 3)
    {
        return get_distance_to_3D_hull(hp, test_vp, distance_ptr);
    }
    else
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_distance_to_2D_hull
(
    const Hull*   hp,
    const Vector* test_vp,
    double*       distance_ptr
)
{
    Vector* constraint_vp;
    Matrix* normal_mp;
    Matrix* vertex_mp;
    Matrix** edges;
    Matrix* edge_point_mp;
    Vector* v1_vp            = NULL;
    Vector* v2_vp            = NULL;
    Vector* n_dot_x_vp       = NULL;
    Vector* seg_candidate_vp = NULL;
    Vector* temp_vp          = NULL;
    Vector* vertex_vp = NULL;
    Vector* normal_vp = NULL;
    int     i;
    double    d;
    double    min_inside_d = -DBL_MAX;   /* For debugging. Should be changed! */
    double    min_outside_d = -DBL_MAX;  /* For debugging. Should be changed! */
    int     inside_hull      = TRUE;
    int     inside_segment;
    int     num_vertices;
    int     num_edges;


    if (hp->vertex_mp == NULL)
    {
        set_bug("Hulls sent to get_distance_to_2D_hull must have HULL_VERTICES.");
        return ERROR;
    }

    if (hp->normal_mp == NULL)
    {
        set_bug("Hulls sent to get_distance_to_2D_hull must have HULL_NORMALS.");
        return ERROR;
    }

    if (hp->b_value_vp == NULL)
    {
        set_bug("Hulls sent to get_distance_to_2D_hull must have HULL_B_VALUES.");
        return ERROR;
    }

    if (hp->facets == NULL)
    {
        set_bug("Hulls sent to get_distance_to_2D_hull must have HULL_FACETS.");
        return ERROR;
    }

    constraint_vp = hp->b_value_vp;
    normal_mp = hp->normal_mp;
    vertex_mp = hp->vertex_mp;
    edges = hp->facets->elements;

    num_vertices = vertex_mp->num_rows;
    num_edges = normal_mp->num_rows;   /* Same, just more descriptive. */

    if (multiply_matrix_and_vector(&n_dot_x_vp, normal_mp, test_vp)==ERROR)
    {
        kjb_print_error();
        set_bug("Unable to multiply matrix and vector in get_distance_to_2D_hull.");
        return ERROR;
    }

    for (i=0; i<num_edges; i++)
    {
        d = constraint_vp->elements[i] - n_dot_x_vp->elements[i];

        if (d < 0.0)
        {
            inside_hull = FALSE;

            /*
            // Drop perpendicular from origin to facet half-space. If the place
            // where the perpendicular contacts the face half-space is in facet,
            // then nudge distance downwards.
            //    x.n = a
            //    d = a - x.n
            //    x.n is perpendicular distance from origin to facet half-space.
            //    -d is the amount that we exceed this by
            //    so  x - n.(-d) is min distance to facet half-space
            //    Note: n.(x - n.(x.n - a)) = a
            */
            d = -d;
            edge_point_mp = edges[ i ];
            ERE(get_matrix_row(&normal_vp, normal_mp, i));
            ERE(multiply_vector_by_scalar(&temp_vp, normal_vp, d));
            ERE(subtract_vectors(&seg_candidate_vp, test_vp, temp_vp));
            ERE(get_matrix_row(&v1_vp, edge_point_mp, 0));
            ERE(get_matrix_row(&v2_vp, edge_point_mp, 1));

            ERE(inside_segment = is_point_in_segment(v1_vp, v2_vp,
                                                     seg_candidate_vp));
            if (inside_segment)
            {
                if (    (min_outside_d < -DBL_MAX/2)   /* First time. */
                     || (d < min_outside_d)
                   )
                {
                    min_outside_d = d;
                }
            }
        }
        else if (inside_hull)
        {
            if ((min_inside_d < -DBL_MAX/2) || (d < min_inside_d))
            {
                min_inside_d = d;
            }
        }
    }

    if ( !inside_hull)
    {
        /* Check distances to vertices. */

        for (i=0; i<num_vertices; i++)
        {
            ERE(get_matrix_row(&vertex_vp, vertex_mp, i));

            d = vector_distance(vertex_vp, test_vp);

            if (    (min_outside_d < -DBL_MAX/2.0)   /* First time. */
                 || (d < min_outside_d)
               )
            {
                min_outside_d = d;
            }
        }
        *distance_ptr = min_outside_d;
    }
    else
    {
        *distance_ptr = -min_inside_d;
    }

    free_vector(n_dot_x_vp);
    free_vector(temp_vp);
    free_vector(v1_vp);
    free_vector(v2_vp);
    free_vector(seg_candidate_vp);
    free_vector(vertex_vp);
    free_vector(normal_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int get_distance_to_3D_hull
(
    const Hull*   hp,
    const Vector* test_vp,
    double*       distance_ptr
)
{
    Vector* constraint_vp;
    Matrix* normal_mp;
    Matrix* vertex_mp;
    Matrix** facets;
    Matrix* facet_point_mp;
    Vector* v1_vp            = NULL;
    Vector* v2_vp            = NULL;
    Vector* s1_vp            = NULL;
    Vector* s2_vp            = NULL;
    Vector* n_dot_x_vp       = NULL;
    Vector* seg_candidate_vp = NULL;
    Vector* temp_vp          = NULL;
    Vector* vertex_vp = NULL;
    Vector* normal_vp = NULL;
    int     i, j;
    double    d;
    double    min_inside_d = -DBL_MAX;   /* For debugging. Should be changed! */
    double    min_outside_d = -DBL_MAX;  /* For debugging. Should be changed! */
    int     inside_hull      = TRUE;
    int     inside_polygon;
    int     num_vertices;
    int     num_facets;
    Matrix* triangle_mp = NULL;
    Vector* corner_vp = NULL;


    if (hp->vertex_mp == NULL)
    {
        set_bug("Hulls sent to get_distance_to_3D_hull must have HULL_VERTICES.");
        return ERROR;
    }

    if (hp->normal_mp == NULL)
    {
        set_bug("Hulls sent to get_distance_to_3D_hull must have HULL_NORMALS.");
        return ERROR;
    }

    if (hp->b_value_vp == NULL)
    {
        set_bug("Hulls sent to get_distance_to_3D_hull must have HULL_B_VALUES.");
        return ERROR;
    }

    if (hp->facets == NULL)
    {
        set_bug("Hulls sent to get_distance_to_3D_hull must have HULL_FACETS.");
        return ERROR;
    }

    constraint_vp = hp->b_value_vp;
    normal_mp = hp->normal_mp;
    vertex_mp = hp->vertex_mp;
    facets = hp->facets->elements;

    num_vertices = vertex_mp->num_rows;
    num_facets = normal_mp->num_rows;

    if (multiply_matrix_and_vector(&n_dot_x_vp, normal_mp, test_vp)==ERROR)
    {
        kjb_print_error();
        set_bug("Unable to multiply matrix and vector in get_distance_to_3D_hull.");
        return ERROR;
    }

    for (i=0; i<num_facets; i++)
    {
        /*
        // We assume we are inside the hull, until proven otherwise by getting a
        // d less than 0. As long as we are inside, the minimal d will be the
        // distance.
        */
        d = constraint_vp->elements[i] - n_dot_x_vp->elements[i];

        if (d < 0.0)
        {
            inside_hull = FALSE;

            /*
            // Drop perpendicular from origin to facet half-space. If the place
            // where the perpendicular contacts the face half-space is in facet,
            // then nudge distance downwards.
            //    x.n = a
            //    d = a - x.n
            //    x.n is perpendicular distance from origin to facet half-space.
            //    -d is the amount that we exceed this by
            //    so  x - n.(-d) is min distance to facet half-space
            //    Note: n.(x - n.(x.n - a)) = a
            */
            d = -d;
            facet_point_mp = facets[ i ];
            ERE(get_matrix_row(&normal_vp, normal_mp, i));
            ERE(multiply_vector_by_scalar(&temp_vp, normal_vp, d));
            ERE(subtract_vectors(&seg_candidate_vp, test_vp, temp_vp));

            ERE(get_target_matrix(&triangle_mp, 3, 3));
            ERE(get_matrix_row(&corner_vp, facet_point_mp, 0));
            ERE(put_matrix_row(triangle_mp, corner_vp, 0));

            /*
            // Check whether or not the candidate is inside each triangle of the
            // facet. We can't send the facet itself to is_point_in_polygon
            // because the points are not cyclic.
            */
            for (j=0; j<facet_point_mp->num_rows - 2; j++)
            {
                ERE(get_matrix_row(&corner_vp, facet_point_mp, j + 1));
                ERE(put_matrix_row(triangle_mp, corner_vp, 1));
                ERE(get_matrix_row(&corner_vp, facet_point_mp, j + 2));
                ERE(put_matrix_row(triangle_mp, corner_vp, 2));

                ERE(inside_polygon = is_point_in_polygon(triangle_mp,
                                                         seg_candidate_vp));
                if (inside_polygon)
                {
                    if (    (min_outside_d < -DBL_MAX/2.0)   /* First time. */
                         || (d < min_outside_d)
                       )
                    {
                        min_outside_d = d;
                    }
                    break;
                }
            }
        }
        else if (inside_hull)
        {
            /* Initialize on first cycle, since for this result to stand, we
            // will have to get here for every cycle.
            */
            if ((i == 0) || (d < min_inside_d)) min_inside_d = d;
        }
    }

    if ( !inside_hull)
    {
        /* Check distances to vertices. */

        for (i=0; i<num_vertices; i++)
        {
            ERE(get_matrix_row(&vertex_vp, vertex_mp, i));

            d = vector_distance(vertex_vp, test_vp);

            if (    (min_outside_d < -DBL_MAX/2)   /* First time. */
                 || (d < min_outside_d)
               )
            {
                min_outside_d = d;
            }
        }

        /* Check distances to edges. */

        for (i=0; i<num_facets; i++)
        {
            int num_edges, inside_segment;
            double mag_s2;

            facet_point_mp = facets[ i ];
            num_edges = facet_point_mp->num_rows;

            for (j=0; j<num_edges; j++)
            {
                double dot_product;

                ERE(get_matrix_row(&v1_vp, facet_point_mp, j));
                                                         /* Wrap arround */
                ERE(get_matrix_row(&v2_vp, facet_point_mp, (j+1) % num_edges));

                ERE(subtract_vectors(&s1_vp, test_vp, v1_vp));
                ERE(subtract_vectors(&s2_vp, v2_vp, v1_vp));
                ERE(get_dot_product(s1_vp, s2_vp, &dot_product));
                mag_s2 = vector_magnitude(s2_vp);
                dot_product /= (mag_s2 * mag_s2);

                ERE(multiply_vector_by_scalar(&temp_vp, s2_vp,
                                              dot_product));
                ERE(add_vectors(&seg_candidate_vp, v1_vp, temp_vp));

#ifdef TEST
                /* More robust? */
                {
                    double t, d11, d12, d22, d33, d13, d23, diff;
                    Vector* tv1_vp = NULL;
                    Vector* tv2_vp = NULL;
                    Vector* seg_candidate2_vp = NULL;

                    ERE(get_dot_product(v1_vp, v1_vp, &d11));
                    ERE(get_dot_product(v1_vp, v2_vp, &d12));
                    ERE(get_dot_product(v2_vp, v2_vp, &d22));
                    ERE(get_dot_product(test_vp, test_vp, &d33));
                    ERE(get_dot_product(v1_vp, test_vp, &d13));
                    ERE(get_dot_product(v2_vp, test_vp, &d23));

                    t = (d22 +d13 - d12 - d23) / (d11 + d22 - 2.0*d12);

                    ERE(multiply_vector_by_scalar(&tv1_vp, v1_vp, t));
                    ERE(multiply_vector_by_scalar(&tv2_vp, v2_vp,
                                                  1.0 - t));

                    ERE(add_vectors(&seg_candidate2_vp, tv1_vp, tv2_vp));

                    diff = vector_distance(seg_candidate2_vp, seg_candidate_vp);

                    if (diff > 1e-5)
                    {
                        dbe(diff);
                        db_rv(seg_candidate_vp);
                        db_rv(seg_candidate2_vp);
                    }

                    ERE(copy_vector(&seg_candidate_vp, seg_candidate2_vp));

                    free_vector(seg_candidate2_vp);
                    free_vector(tv1_vp);
                    free_vector(tv2_vp);
                }
#endif

                ERE(inside_segment = is_point_in_segment(v1_vp, v2_vp,
                                                         seg_candidate_vp));
                if (inside_segment)
                {
                    d = vector_distance(test_vp, seg_candidate_vp);

                    if (d < min_outside_d)
                    {
                        min_outside_d = d;
                    }
                }
            }
        }

        *distance_ptr = min_outside_d;
    }
    else
    {
        *distance_ptr = -min_inside_d;
    }

    free_vector(n_dot_x_vp);
    free_vector(temp_vp);
    free_vector(v1_vp);
    free_vector(v2_vp);
    free_vector(s1_vp);
    free_vector(s2_vp);
    free_vector(seg_candidate_vp);
    free_vector(vertex_vp);
    free_vector(normal_vp);
    free_matrix(triangle_mp);
    free_vector(corner_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             expand_hull
 *
 * Increases a hull using a relative error model
 *
 * This routine increases a hull assuming a relative error model. Under "exact"
 * expansion, each point is replace by an error box, using the relative error,
 * and the resulting convex hull is round. Exact expansion can lead to precision
 * problems, so non-exact expansion was developed whereby the error box (eight
 * points in 3D) is trimmed to one, specifically the point which is furthest
 * away from the hull. Thus the expanded hull does not (typically) have any more
 * facets than the original. (The problem with exact expansion is that it
 * creates many small facets.) Unfortunately, non-exact expansion is
 * computationally expensive. The choice between exact and non-exact expansion
 * is governed by the user settable options "expand-hulls-exactly".
 *
 * Returns:
 *    A pointer to the expanded hull. If there are problems, an error message is
 *    set, and NULL is returned.
 *
 * Index: geometry, convex hulls
 *
 * -----------------------------------------------------------------------------
*/

Hull* expand_hull
(
    const Hull* hp,
    double      relative_expansion,
    unsigned long       options
)
{
    Matrix* vertex_mp;
    Matrix* error_box_mp;
    Matrix* new_vertex_mp   = NULL;
    Matrix* error_vertex_mp = NULL;
    Hull*   new_hp;
    int     i, j;
    double  f1, f2;
    int     result = NO_ERROR;


    vertex_mp = hp->vertex_mp;

    f1 = 1.0 - relative_expansion;

    if (f1 < 0.0)
    {
        f1 = 0.0;
        TEST_PSO(("Hull expansion clipped at zero.\n"));
    }

    f2 = 1.0 + relative_expansion;

    NRN(error_box_mp = create_matrix(vertex_mp->num_rows,
                                     2*(vertex_mp->num_cols)));

    for (i=0; i<vertex_mp->num_rows; i++)
    {
        for (j=0; j<vertex_mp->num_cols; j++)
        {
            (error_box_mp->elements)[i][ 2*j ] = f1*(vertex_mp->elements)[i][j];
            (error_box_mp->elements)[i][2*j+1] = f2*(vertex_mp->elements)[i][j];
        }
    }

    if (fs_expand_hulls_exactly)
    {
        new_vertex_mp = expand_error_box(error_box_mp);

        if (new_vertex_mp == NULL) result = ERROR;
    }
    else
    {
        error_vertex_mp = expand_error_box(error_box_mp);

        if (error_vertex_mp == NULL) result = ERROR;

        if (result != ERROR)
        {
            int expansion_factor = error_vertex_mp->num_rows /
                                                           vertex_mp->num_rows;

            result = get_approximate_error_hull_data(&new_vertex_mp,
                                                     error_vertex_mp,
                                                     expansion_factor, hp);
        }
    }

    free_matrix(error_box_mp);
    free_matrix(error_vertex_mp);

    if (result != ERROR)
    {
        new_hp = find_convex_hull(new_vertex_mp, options);
    }
    else
    {
        new_hp = NULL;
    }

    free_matrix(new_vertex_mp);

    return new_hp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                        get_approximate_error_hull_data
 *
 * Gets approximation of convex hull error data
 *
 * This routine is a support routine for expand_hull, which is exported because
 * it is also used in the gamut program. It is likely too specialized to be
 * considered a double library module.
 *
 * It takes a matrix pointed to "mp" which is assumed to be divided into
 * consective row blocks, consisting of "block_size" rows each. The blocks are
 * assumed to be various error points (such as the corners of an error box)
 * which, in order to reduce convex hull precision problems, need to be
 * distilled into one point. This routine does this by choosing the point which
 * is furthest from the hull "hp" (expensive!). The target matrix **error_mpp
 * will end up having (mp->num_rows / block_size) rows, and mp->num_cols,
 * columns.
 *
 * Warning:
 *     It is assumed that at least one of the points is either on the hull
 *     boundary, or outside the hull boundary. In other-words, all points in an
 *     error block cannot be inside the hull. We include this check, because
 *     error invariably expands hulls, and proceeding naively with the point
 *     which is closest to the boundary, even if it is well inside the hull,
 *     would contract the hull in that region.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * -----------------------------------------------------------------------------
*/

int get_approximate_error_hull_data
(
    Matrix**      error_mpp,
    const Matrix* mp,
    int           block_size,
    const Hull*   hp
)
{
    int     result   = NO_ERROR;
    Matrix* error_mp;
    Vector* row_vp   = NULL;
    Vector* max_distance_row_vp = NULL;
    int     i, j;


    if (mp->num_rows % block_size != 0)
    {
        SET_ARGUMENT_BUG();
    }

    ERE(get_target_matrix(error_mpp, mp->num_rows / block_size,
                          mp->num_cols));

    error_mp = *error_mpp;

    for (i=0; i<error_mp->num_rows; i++)
    {
        IMPORT volatile Bool io_atn_flag;
        IMPORT volatile Bool halt_all_output;
        double                max_distance    = -DBL_MAX;
        double                distance;

        if (result == ERROR) break;

        for (j=0; j<block_size; j++)
        {
            if (result == ERROR) break;

            result = get_matrix_row(&row_vp, mp, block_size * i + j);

            if (result != ERROR)
            {
                result = get_distance_to_hull(hp, row_vp, &distance);
            }

            if (result != ERROR)
            {
                if (distance > max_distance)
                {
                    result = copy_vector(&max_distance_row_vp, row_vp);
                    max_distance = distance;
                }
            }
        }

        if (result != ERROR)
        {
            if (max_distance < -DBL_EPSILON)
            {
                set_error("Problem finding error point outside (or on) hull.");
                result = ERROR;
            }
            else
            {
                result = put_matrix_row(error_mp, max_distance_row_vp, i);
            }
        }

        if (io_atn_flag)
        {
            set_error("Processing interrupted.");
            halt_all_output = FALSE;
            result = ERROR;
        }
    }

    free_vector(row_vp);
    free_vector(max_distance_row_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           trim_2D_hull
 *
 * Removes redundant points from a 2D hull.
 *
 * This routine creates a new hull, similar to the argument hull, but with
 * redundant points removed. A point is considered redundant if removing the
 * point does not change the hull too much. Slightly more specifically, it is
 * considered redundant if it is almost colinear with two other vertices. The
 * gory details are as follows.
 * |    Let D be the distance between two vertices P1, P2.
 * |    Consider some other vertex, V.
 * |    Let the perpendicular distance from V to the segment P1--P2 be d.
 * |    Then, the point is redundant if d/D < tolerance.
 *
 * When a redundant point is found, then it is removed, and then the remaining
 * points are considered as a group (without the removed ones). It would be
 * faster to condider each point to be removed in the context of the complete
 * set of vertices, but this could lead to all points being deleted. However,
 * no effort is currently made to remove the "most" redundant point first. This
 * may be a reasonable improvement to the routine.
 *
 * Returns:
 *    A pointer to a new hull on success and NULL on failure.
 *
 * Index: geometry, convex hulls
 *
 * -----------------------------------------------------------------------------
*/

Hull* trim_2D_hull(const Hull* hp, double tolerance)
{
    Matrix* dist_sqr_mp  = NULL;
    Matrix* trim_vertex_mp = NULL;
    Matrix* vertex_mp = hp->vertex_mp;
    int num_rows = vertex_mp->num_rows;
    int num_cols = vertex_mp->num_cols;
    Hull* trim_hp;
    int i, j, k;
    int count = 0;
    double tolerance_sqr = tolerance * tolerance;


    if (num_cols != 2)
    {
        SET_ARGUMENT_BUG();
        return NULL;
    }

    ERN(get_target_matrix(&dist_sqr_mp, num_rows, num_rows));
    ERN(copy_matrix(&trim_vertex_mp, vertex_mp));

    for (i = 0; i < num_rows; i++)
    {
        /* Not actually used, but may be relavent as things change. */
        dist_sqr_mp->elements[ i ][ i ] = 0.0;

        for (j = i + 1; j < num_rows; j++)
        {
            double x1 = vertex_mp->elements[ i ][ 0 ];
            double x2 = vertex_mp->elements[ j ][ 0 ];
            double y1 = vertex_mp->elements[ i ][ 1 ];
            double y2 = vertex_mp->elements[ j ][ 1 ];
            double dx = x1 - x2;
            double dy = y1 - y2;
            double dd = dx * dx + dy * dy;


            dist_sqr_mp->elements[ i ][ j ] = dd;
            dist_sqr_mp->elements[ j ][ i ] = dd;
        }
    }

    /*
    // For each vertex, see if it is between any other pair of vertices. If it
    // is, then we shuffle the matrix rows so that that vertex is ignored. We
    // shuffle after eliminating each vertex to gaurentee that some vertices
    // remain. Otherwise, a big enough circle could completely dissapear.
    */

    while (count < num_rows)
    {
        int keep = TRUE;
        double vx, vy;

        vx = trim_vertex_mp->elements[ count ][ 0 ];
        vy = trim_vertex_mp->elements[ count ][ 1 ];

        for (j = 0; j < num_rows; j++)
        {
            double p1x, p1y;

            if (j == count) continue;

            p1x = trim_vertex_mp->elements[ j ][ 0 ];
            p1y = trim_vertex_mp->elements[ j ][ 1 ];

            for (k = j + 1 ; k < num_rows; k++)
            {
                double d1, d2, d12;
                double p2x, p2y;

                if (k == count) continue;

                p2x = trim_vertex_mp->elements[ k ][ 0 ];
                p2y = trim_vertex_mp->elements[ k ][ 1 ];

                d12 = dist_sqr_mp->elements[ j ][ k ];
                d1 = dist_sqr_mp->elements[ count ][ j ];
                d2 = dist_sqr_mp->elements[ count ][ k ];

                if ((d1 < d12) && (d2 < d12))
                {
                    double dx12 = p2x - p1x;
                    double dy12 = p2y - p1y;
                    double dx = vx - p1x;
                    double dy = vy - p1y;
                    double dot = dx*dx12 + dy*dy12;
                    double dot_sqr = dot * dot;
                    double ratio_sqr = d1 / d12 - dot_sqr / (d12 * d12);

                    ASSERT(ratio_sqr >= 0.0);

                    if (ratio_sqr < tolerance_sqr)
                    {
                        keep = FALSE;
                        break;
                    }
                }
            }

            if ( ! keep ) break;
        }

        if (keep)
        {
            count++;
        }
        else
        {
            int s;

            /* Shuffle the rows.  */
            for (s = count + 1; s < num_rows; s++)
            {
                int t;

                trim_vertex_mp->elements[ s - 1][ 0 ] =
                                           trim_vertex_mp->elements[ s ][ 0 ];
                trim_vertex_mp->elements[ s - 1][ 1 ] =
                                           trim_vertex_mp->elements[ s ][ 1 ];

                for (t = 0; t < num_rows; t++)
                {
                    dist_sqr_mp->elements[ s - 1 ][ t ] =
                                              dist_sqr_mp->elements[ s ][ t ];
                }
            }

            /* Shuffle the distance matrix columns. */
            for (s = count + 1; s < num_rows; s++)
            {
                int t;

                for (t = 0; t < num_rows; t++)
                {
                    dist_sqr_mp->elements[ t ][ s - 1 ] =
                                              dist_sqr_mp->elements[ t ][ s ];
                }
            }

            num_rows--;

#ifdef TEST
            /*
            // Check shuffling of distance matrix.
            */
            for (i = 0; i < num_rows; i++)
            {
                ASSERT(IS_EQUAL_DBL(dist_sqr_mp->elements[ i ][ i ],
                                     0.0));

                for (j = i + 1; j < num_rows; j++)
                {
                    double x1 = trim_vertex_mp->elements[ i ][ 0 ];
                    double x2 = trim_vertex_mp->elements[ j ][ 0 ];
                    double y1 = trim_vertex_mp->elements[ i ][ 1 ];
                    double y2 = trim_vertex_mp->elements[ j ][ 1 ];
                    double dx = x1 - x2;
                    double dy = y1 - y2;
                    double dd = dx * dx + dy * dy;


                    ASSERT(IS_EQUAL_DBL(dist_sqr_mp->elements[ i ][ j ], dd));
                    ASSERT(IS_EQUAL_DBL(dist_sqr_mp->elements[ j ][ i ], dd));
                }
            }
#endif

        }
    }

    ASSERT(num_rows == count);

    trim_vertex_mp->num_rows = count;

    trim_hp = find_convex_hull(trim_vertex_mp, DEFAULT_HULL_OPTIONS);

    free_matrix(trim_vertex_mp);
    free_matrix(dist_sqr_mp);

    return trim_hp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

