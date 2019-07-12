
/* $Id: g_geometry.c 22174 2018-07-01 21:49:18Z kobus $ */

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

#include "g/g_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "g/g_geometry.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                      get_polygon_CM_and_area
 *
 * Finds the CM and area of a polygon
 *
 * This routine finds the center of mass and area of a polygon. The polygon is
 * specified by a list of points placed in the rows of the matrix point_mp. The
 * points can be of 2 or 3 dimensions, but this routine assumes that they lie
 * in a 2-D subspace. Either output argument can be set to NULL, in which case,
 * that output is not returned.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: geometry, convex hulls
 *
 * -----------------------------------------------------------------------------
*/

int get_polygon_CM_and_area
(
    const Matrix* points_mp,
    Vector**      cm_vpp,
    double*       area_ptr
)
{
    Matrix* ordered_points_mp = NULL;
    int     result;

    ERE(order_planer_points(&ordered_points_mp, points_mp));

    result = get_ordered_polygon_CM_and_area(ordered_points_mp, cm_vpp,
                                             area_ptr);

    free_matrix(ordered_points_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                    get_ordered_polygon_CM_and_area
 *
 * Finds the CM and area of a polygon
 *
 * This routine finds the center of mass and area of a polygon. The polygon is
 * specified by a list of points in cyclacle order placed in the rows of the
 * matrix point_mp. The points can be of any dimension, but this routine assumes
 * that they lie in a 2-D subspace. Either output argument can be set to NULL,
 * in which case, that output is not returned.
 *
 * Note:
 *     If the points are out of order, then this routine will not work! If the
 *     points are not known to be in order, then it is best to use
 *     get_polygon_CM_and_area().
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an error message being
 *     set.
 *
 * Index: geometry, convex hulls
 *
 * -----------------------------------------------------------------------------
*/

int get_ordered_polygon_CM_and_area
(
    const Matrix* points_mp,
    Vector**      cm_vpp,
    double*       area_ptr
)
{
    int     i;
    int     num_edges;
    double  area                        = 0.0;
    Vector* point_vp                    = NULL;
    Vector* normal_vp                   = NULL;
    Vector* e1_vp                       = NULL;
    Vector* e2_vp                       = NULL;
    Vector* cm_vp                       = NULL;
    Vector* region_cm_vp                = NULL;
    Vector* two_times_edge_mid_point_vp = NULL;
    int     dim;


    dim = points_mp->num_cols;

    num_edges = points_mp->num_rows - 2;
    ERE(get_matrix_row(&point_vp, points_mp, 0));
    ERE(get_zero_vector(&cm_vp, dim));

    for (i=0; i<num_edges; i++)
    {
        double l1;
        double l2;
        double l3;
        double s;
        double area_squared;
        double region_area;

        ERE(get_matrix_row(&e1_vp, points_mp, i + 1));
        ERE(get_matrix_row(&e2_vp, points_mp, i + 2));

        l1 = vector_distance(e1_vp, e2_vp);
        l2 = vector_distance(e1_vp, point_vp);
        l3 = vector_distance(e2_vp, point_vp);
        s = (l1 + l2 + l3) / 2.0;

        area_squared = s * (s - l1) * (s - l2) * (s - l3);

        if (area_squared < 0.0)
        {
            double tol = 1000.0 * DBL_EPSILON;

            if (    ((s < l1) && (s >= l1 * (1.0 - tol)))
                 || ((s < l2) && (s >= l2 * (1.0 - tol)))
                 || ((s < l3) && (s >= l3 * (1.0 - tol)))
               )
            {
                region_area = 0.0;
            }
            else
            {
                dbm("area_squared < 0.0");
                dbe(area_squared);
                db_mat(points_mp);
                db_rv(point_vp);
                db_rv(e1_vp);
                db_rv(e2_vp);
                dbe(l1);
                dbe(l2);
                dbe(l3);

                set_bug("Area squared is negative in get_polygon_CM_and_area.");

                return ERROR;
            }
        }
        else
        {
            region_area = sqrt(area_squared);
        }

        ERE(add_vectors(&two_times_edge_mid_point_vp, e1_vp, e2_vp));
        ERE(add_vectors(&region_cm_vp, two_times_edge_mid_point_vp,
                        point_vp));
        ERE(ow_divide_vector_by_scalar(region_cm_vp, 3.0));
        ERE(ow_multiply_vector_by_scalar(region_cm_vp, region_area));

        ERE(ow_add_vectors(cm_vp, region_cm_vp));
        area += region_area;
    }

    if (cm_vpp != NULL)
    {
        if (area < 100.0 * DBL_EPSILON)
        {
            /*
            // If the area is small, then at least some of the points are quite
            // close together, and likely the average of the corners is a
            // reasonable estimate.
            */
            dbm("Small area noted in get_polygon_CM_and_area.\n");
            dbf(area);
            ERE(average_matrix_rows(cm_vpp, points_mp));
        }
        else
        {
            ERE(ow_divide_vector_by_scalar(cm_vp, area));
            ERE(copy_vector(cm_vpp, cm_vp));
        }
    }

    if (area_ptr != NULL)
    {
        *area_ptr = area;
    }

    free_vector(point_vp);
    free_vector(normal_vp);
    free_vector(region_cm_vp);
    free_vector(cm_vp);
    free_vector(two_times_edge_mid_point_vp);
    free_vector(e1_vp);
    free_vector(e2_vp);

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       is_point_in_segment
 *
 * Determins if point is inside segment
 *
 * This routine determines if a point, assumed to be co-linear with a segment,
 * is between the endpoints. The end-points can be of any dimension. This
 * routine assumes that the points are co-linear, but does not check.
 *
 * Returns:
 *     TRUE if the point is inside the segment, FALSE if not, and ERROR if there
 *     are any problems, with an error message being set.
 *
 * Index: geometry
 *
 * -----------------------------------------------------------------------------
*/

int is_point_in_segment
(
    const Vector* p1_vp,
    const Vector* p2_vp,
    const Vector* test_point_vp
)
{
    Vector* t1_vp         = NULL;
    Vector* t2_vp         = NULL;
    double  dot_product_1;
    double  dot_product_2;
    double  temp_norm;
    double  diff_norm;


    ERE(subtract_vectors(&t1_vp, test_point_vp, p1_vp));

    temp_norm = max_abs_vector_element(p1_vp);
    diff_norm = max_abs_vector_element(t1_vp);

    if (diff_norm < 10.0 * temp_norm * DBL_EPSILON)
    {
        free_vector(t1_vp);
        return TRUE;
    }

    ERE(subtract_vectors(&t2_vp, p2_vp, p1_vp));

    ERE(get_dot_product(t1_vp, t2_vp, &dot_product_1));

    ERE(subtract_vectors(&t1_vp, test_point_vp, p2_vp));

    temp_norm = max_abs_vector_element(p2_vp);
    diff_norm = max_abs_vector_element(t1_vp);

    if (diff_norm < 10.0 * temp_norm * DBL_EPSILON)
    {
        free_vector(t1_vp);
        free_vector(t2_vp);
        return TRUE;
    }

    ERE(subtract_vectors(&t2_vp, p1_vp, p2_vp));

    ERE(get_dot_product(t1_vp, t2_vp, &dot_product_2));

    free_vector(t1_vp);
    free_vector(t2_vp);

    if ((dot_product_1 <= 0.0) || (dot_product_2 <= 0.0))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                           is_point_in_polygon
 *
 * Determines if a point is inside a polygon
 *
 * This routine determines if a point, assumed to be co-planer with a convex
 * polygon, is inside that polygon. The polygon is specified by a list of points
 * in cyclic order placed in the rows of the matrix point_mp. The points can
 * be of any dimension, but this routine assumes that they lie in a 2-D
 * subspace. The boundary of the polygon is considered part of the polygon, so
 * if the point is on the boundary, then the routine returns TRUE.
 *
 * Note:
 *     If the points are out of order, or if the polygon is not convex, or if
 *     the point is not in the plane of the polygon, then this routine will not
 *     work!
 *
 * Warning:
 *     Qhull output: The edges of facets of a 3D hull are not in cyclic order.
 *     One solution is do call this routine for each triangle. Also, the edges
 *     of a 2D hull are not in order, unless find_convex_hull() was called with
 *     the ORDER_2D_HULL_FACETS option.
 *
 * Note:
 *     The algorithm used in this routine is not very good. Should be fixed
 *     sometime. 
 *
 * Returns:
 *     TRUE if the point is inside the polygon, FALSE if not, and ERROR if there
 *     are any problems, with an error message being set.
 *
 * Index: geometry, polygons
 *
 * -----------------------------------------------------------------------------
*/

int is_point_in_polygon
(
    const Matrix* points_mp,
    const Vector* test_point_vp
)
{
    Vector* e1_vp            = NULL;
    Vector* e2_vp            = NULL;
    Vector* v1_vp            = NULL;
    Vector* v2_vp            = NULL;
    Vector* v3_vp            = NULL;
    Vector* to_test_point_vp = NULL;
    double    dot_product_1;
    double    dot_product_2;
    int     num_angles;
    int     i;
    int     result;
    double    temp_norm;
    double    diff_norm;


    num_angles = points_mp->num_rows;

    result = TRUE;

    /*
     * FIXME
     *
     * I don't think this algorithm is so good! Perhaps better to consider the
     * normals for the half spaces for the lines. 
    */

    /*
    // For each vertex, check that the test point is inside the angle suggested
    // by that vertex. We do this by checking that the dot product between one
    // of the angle edges and the line to the test point is greater than that
    // dot product between the two edges forming the angle. (Dot product
    // decreases with increasing angle).
    */
    for (i=0; i<num_angles; i++)
    {
        if (get_matrix_row(&v1_vp, points_mp, i) == ERROR)
        {
            result = ERROR;
            break;
        }
                                              /* Wrap arround */
        if (get_matrix_row(&v2_vp, points_mp, (i + 1) % num_angles) == ERROR)
        {
            result = ERROR;
            break;
        }
                                              /* Wrap arround */
        if (get_matrix_row(&v3_vp, points_mp, (i + 2) % num_angles) == ERROR)
        {
            result = ERROR;
            break;
        }

        if (subtract_vectors(&e1_vp, v1_vp, v2_vp) == ERROR)
        {
            result = ERROR;
            break;
        }

        if (subtract_vectors(&e2_vp, v3_vp, v2_vp) == ERROR)
        {
            result = ERROR;
            break;
        }

        if (subtract_vectors(&to_test_point_vp, test_point_vp, v2_vp) == ERROR)
        {
            result = ERROR;
            break;
        }

        temp_norm = max_abs_vector_element(v2_vp);
        diff_norm = max_abs_vector_element(to_test_point_vp);
        /*
        // Unlikely special case: The point being tested is exactly a vertex.
        */
        if (diff_norm < 10.0 * temp_norm * DBL_EPSILON)
        {
            result = TRUE;
            break;
        }

        if (get_dot_product(e1_vp, e2_vp, &dot_product_1) == ERROR)
        {
            result = ERROR;
            break;
        }

        /* Normalization by |e1_vp| logically applies to both dot products, so
        // we can skip it for efficiency.
        */
        dot_product_1 /= vector_magnitude(e2_vp);

        if (get_dot_product(to_test_point_vp, e1_vp, &dot_product_2) == ERROR)
        {
            result = ERROR;
            break;
        }

        /* Normalization by |e1_vp| logically applies to both dot products, so
        // we can skip it for efficiency.
        */
        dot_product_2 /= vector_magnitude(to_test_point_vp);

        /*
        db_rv(e1_vp);
        db_rv(e2_vp);
        db_rv(to_test_point_vp);
        dbf(dot_product_1);
        dbf(dot_product_2);
        */

        if (dot_product_1 > dot_product_2)
        {
            result = FALSE;
            break;
        }
    }

    free_vector(v1_vp);
    free_vector(v2_vp);
    free_vector(v3_vp);
    free_vector(e1_vp);
    free_vector(e2_vp);
    free_vector(to_test_point_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
#define USE_PLOT_TO_VERIFY_ORDERING
*/

#ifdef USE_PLOT_TO_VERIFY_ORDERING
#include "p/p_plot.h"
#endif

int order_planer_points
(
    Matrix**      ordered_points_mpp,
    const Matrix* points_mp
)
{
    Vector* ave_vp = NULL;
    Vector* theta_vp = NULL;
    Vector* p1_vp = NULL;
    Vector* p2_vp = NULL;
    Vector* s1_vp = NULL;
    Vector* s2_vp = NULL;
    int num_points = points_mp->num_rows;
    int result;
    int i;


    if (points_mp->num_rows < 4)
    {
        return copy_matrix(ordered_points_mpp, points_mp);
    }

    ERE(average_matrix_rows(&ave_vp, points_mp));

    result = get_target_vector(&theta_vp, num_points);

    if (result != ERROR)
    {
        theta_vp->elements[ 0 ] = 0.0;

        result = get_matrix_row(&p1_vp, points_mp, 0);
    }

    if (result != ERROR)
    {
        result = subtract_vectors(&s1_vp, p1_vp, ave_vp);
    }

    for (i=1; i<num_points; i++)
    {
        if (result == ERROR) break;

        result = get_matrix_row(&p2_vp, points_mp, i);

        if (result != ERROR)
        {
            result = subtract_vectors(&s2_vp, p2_vp, ave_vp);
        }

        if (result != ERROR)
        {
            result = get_vector_angle_in_degrees(s1_vp, s2_vp,
                                                 &(theta_vp->elements[ i ]));
        }

        if (result != ERROR)
        {
            if (z_component_of_cross_product(s1_vp, s2_vp) > 0.0)
            {
                theta_vp->elements[ i ] = 360.0 - theta_vp->elements[ i ];
            }
        }
    }

    if (result != ERROR)
    {
        Indexed_vector* indexed_angles_vp = vp_create_indexed_vector(theta_vp);

        if (indexed_angles_vp == NULL) result = ERROR;

        if (result != ERROR)
        {
            result = ascend_sort_indexed_vector(indexed_angles_vp);
        }

        if (result != ERROR)
        {
            result = get_target_matrix(ordered_points_mpp, num_points,
                                       points_mp->num_cols);
        }

        for (i=0; i<num_points; i++)
        {
            if (result == ERROR) break;

            result = get_matrix_row(&p1_vp, points_mp,
                                    indexed_angles_vp->elements[ i ].index);

            if (result != ERROR)
            {
                result = put_matrix_row(*ordered_points_mpp, p1_vp, i);

            }
        }

#ifdef USE_PLOT_TO_VERIFY_ORDERING
        if (result != ERROR)
        {
            int plot_id;
            Matrix* points_copy_mp = NULL;
            Vector* min_vp = NULL;
            Vector* max_vp = NULL;
            Vector* x_vp = NULL;
            Vector* y_vp = NULL;
            char buff[ 100 ];

            UNTESTED_CODE();

            ERE(plot_id = plot_open());

            EPE(get_min_matrix_col_elements(&min_vp, points_mp));
            EPE(get_max_matrix_col_elements(&max_vp, points_mp));

            EPE(plot_set_range(plot_id,
                               0.9 * min_vp->elements[ 0 ] - 1.0,
                               1.1 * max_vp->elements[ 0 ] + 1.0,
                               0.9 * min_vp->elements[ 1 ] - 1.0,
                               1.1 * max_vp->elements[ 1 ] + 1.0));

            EPE(get_matrix_col(&x_vp, *ordered_points_mpp, 0));
            EPE(get_matrix_col(&y_vp, *ordered_points_mpp, 1));
            EPE(plot_curve(plot_id, x_vp, y_vp, NULL));

            EPE(copy_matrix(&points_copy_mp, points_mp));
            points_copy_mp->num_cols = 2;
            EPE(plot_matrix_row_points(plot_id, points_copy_mp, NULL));

            for (i=0; i<num_points; i++)
            {
                EPE(kjb_sprintf(buff, sizeof(buff), "%d", i + 1));
                EPE(plot_add_label(plot_id, buff,
                                 (*ordered_points_mpp)->elements[ i ][ 0 ],
                                 (*ordered_points_mpp)->elements[ i ][ 1 ]));
            }
                ERE(plot_update(plot_id));
                free_matrix(points_copy_mp);
                free_vector(x_vp);
                free_vector(y_vp);
                free_vector(min_vp);
                free_vector(max_vp);
        }
#endif

        free_indexed_vector(indexed_angles_vp);
    }

    free_vector(ave_vp);
    free_vector(theta_vp);
    free_vector(p1_vp);
    free_vector(p2_vp);
    free_vector(s1_vp);
    free_vector(s2_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double z_component_of_cross_product(const Vector* vp1, const Vector* vp2)
{
    double a1, a2;

    a1 = vp1->elements[ 0 ] * vp2->elements[ 1 ];
    a2 = vp1->elements[ 1 ] * vp2->elements[ 0 ];

    return (a1 - a2);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

