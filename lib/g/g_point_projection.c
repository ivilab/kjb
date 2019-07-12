/* =============================================================================
 *
 * g_point_projection includes functions pertaining to creating point projection
 * matrices from plucker line representations/line projection matrices.
 *
 * int points_from_plucker(Vector **point_one, Vector **point_two, const Vector *plucker_line) -
 * Derives two points, (As 4-vectors), from a plucker line representation.
 *
 * int distance_between_lines_as_points(double *distance, int *is_parallel, const Vector *line_one_point_one,
 *         const Vector *line_one_point_two, const Vector *line_two_point_one, const Vector *line_two_point_two) -
 * Calculates the distance between two lines. Used to check for intersection and
 * parallelism.
 *
 * int plane_from_lines_as_points(Vector **plane, const Vector *line_one_point_one,
 *         const Vector *line_one_point_two, const Vector *line_two_point_one, const Vector *line_two_point_two) -
 * Will create a 4-vector representation of a plane from four coplanar points.
 *
 * int plane_from_plucker_lines(Vector **plane, const Vector *plucker_one, const Vector *plucker_two) -
 * Derives a 4-vector plane from two plucker line representations.
 *
 * int point_projection_matrix_from_line_projection_matrix(Matrix **point_projection_matrix, const Matrix *line_projection_matrix) -
 * Creates a 3x4 point projection matrix from a 3x6 line projection matrix.
 *
 * Author: Andrew Emmott (aemmott)
 *
 * -------------------------------------------------------------------------- */

#include <g/g_point_projection.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 *                                 points_from_plucker
 *
 * Derives two points, (As 4-vectors), from a plucker line representation.
 *
 * Given *plucker_line, will find two points in 3-D space, homogenized to w = 1.0.
 * The approach used is to asssume that the line crosses one of the axes, (X,Y,Z),
 * resulting in both points having a value of 1.0 or -1.0 for one of the other 
 * three values.
 *
 * Returns:
 *    ERROR if plucker_line is not a valid plucker line or on allocation failure.
 *    NO_ERROR otherwise.
 *
 * See also:
 *    plane_from_lines_as_points, plane_from_plucker_lines,
 *    distance_between_lines_as_points
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: plucker line
 * -------------------------------------------------------------------------- */
int points_from_plucker
(
    Vector **point_one, /*Pointer to the first point.*/
    Vector **point_two, /*Pointer to the second point.*/
    const Vector *plucker_line /*The plucker line to derive points from.*/
)
{
    Vector *_vp1;
    Vector *_vp2;

    /*Error Checking*/
    /*The line must be a valid plucker representation.*/
    if (is_plucker_line(plucker_line) == 0)
    {
        G_POINT_PROJECTION_PLUCKER_ERROR;
        return ERROR;
    }
    /*Allocation errors.*/
    if (get_target_vector(point_one, 4) == ERROR || get_target_vector(point_two, 4) == ERROR)
    {
        G_POINT_PROJECTION_ALLOC_ERROR;
        return ERROR;
    }
    /*End Error Checking*/

    /*The strategy is to observe that at least one of the axis planes is intersected.
      It can be assumed that one of the coordinates is 1 and -1 respectively, and the others
      can be reverse engineered.*/
    _vp1 = *point_one;
    _vp1->elements[3] = 1.0;
    _vp2 = *point_two;
    _vp2->elements[3] = 1.0;
    /*X-axis plane is intersected.*/
    if (fabs(plucker_line->elements[2]) > DBL_EPSILON)
    {
        _vp1->elements[0] = 1;
        _vp2->elements[0] = -1;
        _vp1->elements[1] = (plucker_line->elements[0]-plucker_line->elements[4])/plucker_line->elements[2];
        _vp2->elements[1] = (plucker_line->elements[0]+plucker_line->elements[4])/plucker_line->elements[2];
        _vp1->elements[2] = (plucker_line->elements[1]+plucker_line->elements[5])/plucker_line->elements[2];
        _vp2->elements[2] = (plucker_line->elements[1]-plucker_line->elements[5])/plucker_line->elements[2];
    }
    /*Y-axis plane is intersected.*/
    else if (fabs(plucker_line->elements[4]) > DBL_EPSILON)
    {
        _vp1->elements[1] = -1;
        _vp2->elements[1] = 1;
        _vp1->elements[0] = (plucker_line->elements[0]+plucker_line->elements[2])/plucker_line->elements[4];
        _vp2->elements[0] = (plucker_line->elements[0]-plucker_line->elements[2])/plucker_line->elements[4];
        _vp1->elements[2] = (plucker_line->elements[5]-plucker_line->elements[3])/plucker_line->elements[4];
        _vp2->elements[2]= (plucker_line->elements[5]+plucker_line->elements[3])/-plucker_line->elements[4];
    }
    /*Z=axis plane is intersected.*/
    else /*Implicit: if (plucker_line->elements[5] != 0)*/
    {
        _vp1->elements[2] = 1;
        _vp2->elements[2] = -1;
        _vp1->elements[0]=(plucker_line->elements[2]-plucker_line->elements[1])/plucker_line->elements[5];
        _vp2->elements[0]=(plucker_line->elements[2]+plucker_line->elements[1])/-plucker_line->elements[5];
        _vp1->elements[1]=(plucker_line->elements[4]+plucker_line->elements[3])/-plucker_line->elements[5];
        _vp2->elements[1]=(plucker_line->elements[4]-plucker_line->elements[3])/plucker_line->elements[5];
    }

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 distance_between_lines_as_points
 *
 * Calculates the distance between two lines. Used to check for intersection and
 * parallelism.
 *
 * Given four points, assumes two lines from them and calculates the smallest
 * distance between them. If the lines intersect, this distance will be zero.
 * Also set a boolean flag if the lines are parallel.
 *
 * Returns:
 *    ERROR if the points are not 4-vectors or if they aren't homogenized.
 *    NO_ERROR otherwise.
 *
 * See also:
 *    plane_from_lines_as_points, plane_from_plucker_lines, points_from_plucker
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: line intersection
 * -------------------------------------------------------------------------- */
int distance_between_lines_as_points
(
    double *distance, /*Will hold the minimum distance between the lines.*/
    int *is_parallel, /*Will be zero if the lines are not parallel.*/
    const Vector *line_one_point_one, /*A point of a given line.*/
    const Vector *line_one_point_two, /*A point of a given line.*/
    const Vector *line_two_point_one, /*A point of a given line.*/
    const Vector *line_two_point_two /*A point of a given line.*/
)
{
    /*Variables for computation and to keep operations in stack memory.*/
    double ux;
    double uy;
    double uz;
    double vx;
    double vy;
    double vz;
    double wx;
    double wy;
    double wz;
    double a;
    double b;
    double c;
    double d;
    double e;
    double denom;
    double Sc;
    double Tc;
    double distx;
    double disty;
    double distz;
    
    /*Error Checking*/
    if (line_one_point_one->length != 4 || line_one_point_two->length != 4 || line_two_point_one->length != 4 || line_two_point_two->length != 4)
    {
        G_POINT_PROJECTION_SIZE_ERROR;
        return ERROR;
    }
    if (line_one_point_one->elements[3] != 1.0 || line_one_point_two->elements[3] != 1.0 ||
        line_two_point_one->elements[3] != 1.0 || line_two_point_two->elements[3] != 1.0)
    {
        G_POINT_PROJECTION_NORM_ERROR;
        return ERROR;
    }
    /*End Error Checking*/

    /*Calculating faux-vectors in stack memory: u, v and w*/
    ux = line_one_point_two->elements[0] - line_one_point_one->elements[0];
    uy = line_one_point_two->elements[1] - line_one_point_one->elements[1];
    uz = line_one_point_two->elements[2] - line_one_point_one->elements[2];
    vx = line_two_point_two->elements[0] - line_two_point_one->elements[0];
    vy = line_two_point_two->elements[1] - line_two_point_one->elements[1];
    vz = line_two_point_two->elements[2] - line_two_point_one->elements[2];
    wx = line_one_point_one->elements[0] - line_two_point_one->elements[0];
    wy = line_one_point_one->elements[1] - line_two_point_one->elements[1];
    wz = line_one_point_one->elements[2] - line_two_point_one->elements[2];

    /*The needed dot products.*/
    a = ux*ux + uy*uy + uz*uz;
    b = ux*vx + uy*vy + uz*vz;
    c = vx*vx + vy*vy + vz*vz;
    d = ux*wx + uy*wy + uz*wz;
    e = vx*wx + vy*wy + vz*wz;

    /*The denominator of the Sc and Tc scalars.*/
    denom = a*c - b*b;

    /*If denominator is zero.*/
    if (fabs(denom) < DBL_EPSILON)
    {
        *is_parallel = 1;
        /*Implied: Sc = 0*/
        Tc = d/b;
        distx = wx + Tc*vx;
        disty = wy + Tc*vy;
        distz = wz + Tc*vz;
    } else { /*Implied: Denominator is non-zero.*/
        *is_parallel = 0;
        Sc = (b*e - c*d)/denom;
        Tc = (a*e - b*d)/denom;
        distx = wx + Sc*ux - Tc*vx;
        disty = wy + Sc*uy - Tc*vy;
        distz = wz + Sc*uz - Tc*vz;
    }

    /*The magnitude of the distance vector.*/
    *distance = sqrt(distx*distx + disty*disty + distz*distz);

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 plane_from_lines_as_points
 *
 * Will create a 4-vector representation of a plane from four coplanar points.
 *
 * Given four points, assumes two lines from them and derives the common plane
 * between them.
 *
 * Returns:
 *    ERROR if the lines are not coplanar or allocation fails.
 *    NO_ERROR otherwise.
 *
 * See also:
 *    plane_from_plucker_lines, points_from_plucker
 *    distance_between_lines_as_points
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: coplanar, line, plane
 * -------------------------------------------------------------------------- */
int plane_from_lines_as_points
(
    Vector **plane, /*Pointer to the new plane.*/
    const Vector *line_one_point_one, /*A point on a given line.*/
    const Vector *line_one_point_two, /*A point on a given line.*/
    const Vector *line_two_point_one, /*A point on a given line.*/
    const Vector *line_two_point_two /*A point on a given line.*/
)
{
    double distance;
    int is_parallel;
    Vector *line1 = NULL;
    Vector *line2 = NULL;
    Vector *_vp;

    /*Calculate the distance. If the lines are parallel we just mix the points up so we get two
     * intersecting lines that would still exist on the target plane.*/
    distance_between_lines_as_points(&distance, &is_parallel, line_one_point_one, line_one_point_two, line_two_point_one, line_two_point_two);
    if (fabs(distance) > DBL_EPSILON)
    {
        if (is_parallel)
        {
            subtract_vectors(&line1, line_two_point_one, line_one_point_one);
            subtract_vectors(&line2, line_two_point_two, line_one_point_two);
        }
        else
        {
            G_POINT_PROJECTION_NOT_COPLANAR;
            return ERROR;
        }
    }
    else
    {
        subtract_vectors(&line1, line_one_point_two, line_one_point_one);
        subtract_vectors(&line2, line_two_point_two, line_two_point_one);
    }

    if (get_target_vector(plane, 4) == ERROR)
    {
        G_POINT_PROJECTION_ALLOC_ERROR;
        return ERROR;
    }
    _vp = *plane;
    /*Cross-Product of the two lines to get the normal.*/
    _vp->elements[0] = line1->elements[1]*line2->elements[2] - line1->elements[2]*line2->elements[1];
    _vp->elements[1] = line1->elements[2]*line2->elements[0] - line1->elements[0]*line2->elements[2];
    _vp->elements[2] = line1->elements[0]*line2->elements[1] - line1->elements[1]*line2->elements[0];
    /*Calculate the w-value by assuming x+y+z+w=0.*/
    _vp->elements[3] = -(_vp->elements[0]*line_one_point_one->elements[0]+_vp->elements[1]*line_one_point_one->elements[1]+_vp->elements[2]*line_one_point_one->elements[2]);


    free_vector(line1);
    free_vector(line2);

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 plane_from_plucker_lines
 *
 * Derives a 4-vector plane from two plucker line representations.
 *
 * Given two plucker lines, will find points on those lines and then use those
 * points to derive the plane that they have in common.
 *
 * Returns:
 *    ERROR if any called function fails.
 *    NO_ERROR otherwise.
 *
 * See also:
 *    plane_from_lines_as_points, plane_from_plucker_lines, points_from_plucker,
 *    distance_between_lines_as_points
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: plane, plucker
 * -------------------------------------------------------------------------- */
int plane_from_plucker_lines
(
    Vector **plane, /*The target plane address.*/
    const Vector *plucker_one, /*One plucker line.*/
    const Vector *plucker_two /*The other.*/
)
{
    Vector *line_one_point_one = NULL;
    Vector *line_one_point_two = NULL;
    Vector *line_two_point_one = NULL;
    Vector *line_two_point_two = NULL;

    /*No explicit error checking but ERE follows*/
    ERE(points_from_plucker(&line_one_point_one, &line_one_point_two, plucker_one));
    ERE(points_from_plucker(&line_two_point_one, &line_two_point_two, plucker_two));
    ERE(plane_from_lines_as_points(plane, line_one_point_one, line_one_point_two, line_two_point_one, line_two_point_two));
    /*End Error checking*/
    free_vector(line_one_point_one);
    free_vector(line_one_point_two);
    free_vector(line_two_point_one);
    free_vector(line_two_point_two);

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 point_projection_matrix_from_line_projection_matrix
 *
 * Creates a 3x4 point projection matrix from a 3x6 line projection matrix.
 *
 * Observing that a line projection matrix is three plucker lines and that a point
 * projection matrix is three planes, this function derives three planes from the 
 * plucker representations.
 *
 * Returns:
 *    ERROR if the line projection matrix is invalid or if allocation fails.
 *    NO_ERROR otherwise.
 *
 * See also:
 *    plane_from_plucker_lines
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: point projection, line projection, plucker
 * -------------------------------------------------------------------------- */
int point_projection_matrix_from_line_projection_matrix
(
    Matrix **point_projection_matrix,
    const Matrix *line_projection_matrix
)
{
    Vector *plucker_one = NULL;
    Vector *plucker_two = NULL;
    Vector *plucker_three = NULL;
    Vector *plane_one = NULL;
    Vector *plane_two = NULL;
    Vector *plane_three = NULL;
    Vector *scalar_one = NULL;
    Vector *scalar_two = NULL;
    Vector *scalar_three = NULL;
    double pl_scalar1 = DBL_NOT_SET;
    double pl_scalar2 = DBL_NOT_SET;
    double pl_scalar3 = DBL_NOT_SET;
    int i;

    /*Error Checking*/
    /*Ensure line projection matrix is the right size.*/
    if (line_projection_matrix->num_rows != 3 || line_projection_matrix->num_cols != 6)
    {
        G_POINT_PROJECTION_MAT_SIZE_ERROR;
        return ERROR;
    }
    /*Allocation Errors*/
    if (get_target_matrix(point_projection_matrix, 3, 4) == ERROR)
    {
        G_POINT_PROJECTION_MAT_ALLOC_ERROR;
        return ERROR;
    }
    /*End explicit error checking, but ERE follows*/
    ERE(get_matrix_row(&plucker_one, line_projection_matrix, 0));
    ERE(get_matrix_row(&plucker_two, line_projection_matrix, 1));
    ERE(get_matrix_row(&plucker_three, line_projection_matrix, 2));

    if (line_projection_matrix->num_rows != 3 || line_projection_matrix->num_cols != 6)
    {
        G_POINT_PROJECTION_MAT_SIZE_ERROR;
        return ERROR;
    }

    ERE( ow_invert_plucker_line(plucker_one) );
    ERE( ow_invert_plucker_line(plucker_two) );
    ERE( ow_invert_plucker_line(plucker_three) );
    
    /*Derive the planes from the plucker lines.*/
    ERE(plane_from_plucker_lines(&plane_one, plucker_two, plucker_three));
    ERE(plane_from_plucker_lines(&plane_two, plucker_one, plucker_three));
    ERE(plane_from_plucker_lines(&plane_three, plucker_one, plucker_two));
    
    /*Need to reverse the process so that ratios can be maintained.*/
    /*NOTE: These plucker lines need to be derived in a way identical
     * to the implementation in g_plucker.c*/
    ERE(plucker_line_from_planes(&scalar_one, plane_two, plane_three));
    ERE(plucker_line_from_planes(&scalar_two, plane_three, plane_one));
    ERE(plucker_line_from_planes(&scalar_three, plane_one, plane_two));

    /*Element five is used because the lines have been inverted.
     * This is done to save inverting the lines again. Make a note of it.*/
    for(i = 0; i < 6; i++)
    {
        if(fabs(scalar_one->elements[i]) > DBL_EPSILON && fabs(plucker_one->elements[5-i]) > DBL_EPSILON)
        {
            pl_scalar1 = scalar_one->elements[i]/plucker_one->elements[5-i];
            break;
        }
    }
    for(i = 0; i < 6; i++)
    {
        if(fabs(scalar_two->elements[i]) > DBL_EPSILON && fabs(plucker_two->elements[5-i]) > DBL_EPSILON)
        {
            pl_scalar2 = scalar_two->elements[i]/plucker_two->elements[5-i];
            break;
        }
    }
    for(i = 0; i < 6; i++)
    {
        if(fabs(scalar_three->elements[i]) > DBL_EPSILON && fabs(plucker_three->elements[5-i]) > DBL_EPSILON)
        {
            pl_scalar3 = scalar_three->elements[i]/plucker_three->elements[5-i];
            break;
        }
    }
    
    /*Rather than use all three scalars, the last two are made proportional to
     * the first scalar. This saves in total division operations.*/
    pl_scalar2 = pl_scalar1/pl_scalar2;
    pl_scalar3 = pl_scalar1/pl_scalar3;

    for (i=0; i<4; i++)
    {
        plane_two->elements[i]/=pl_scalar2;
        plane_three->elements[i]/=pl_scalar3;
    }

    /*Actually put the planes in the target matrix.*/
    ERE(put_matrix_row(*point_projection_matrix, plane_one, 0));
    ERE(put_matrix_row(*point_projection_matrix, plane_two, 1));
    ERE(put_matrix_row(*point_projection_matrix, plane_three, 2));
    /*End Error checking*/

    
    free_vector(plucker_one);
    free_vector(plucker_two);
    free_vector(plucker_three);
    free_vector(plane_one);
    free_vector(plane_two);
    free_vector(plane_three);
    free_vector(scalar_one);
    free_vector(scalar_two);
    free_vector(scalar_three);

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif
