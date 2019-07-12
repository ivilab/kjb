/* =============================================================================
 * This file contains functions related to creating and manipulating plucker
 * lines.
 *
 * Function Summary:
 *
 *
 *  int plucker_line_from_points
 *      (const Vector *point_one, const Vector *point_two, Vector **plucker_line);
 *  Creates a 6-vector plucker line from the elements in two 4-vectors
 *  representing two points in three-dimensional space.
 *
 *  int plucker_line_from_planes
 *      (const Vector *plane_one, const Vector *plane_two, Vector **plucker_line);
 *  Creates a 6-vector plucker line from the elements in two 4-vectors
 *  representing two planes in three-dimensional space.
 *
 *  int invert_plucker_line
 *      (const Vector *line, Vector **plucker_line);
 *  Creates a 6-vector plucker line that is the dual representation of an
 *  existing plucker line. (Inverts the order of the elements).
 *
 *  int ow_invert_plucker_line
 *      (Vector *line);
 *  Directly changes the given plucker line, inverting the order of the elements,
 *  changing it to the dual representation of itself.
 *
 *  int plucker_line_intersect
 *      (const Vector *line_one, const Vector *line_two);
 *  A boolean check to see if two plucker lines intersect. Returns 0 if false and
 *  1 if true. If either Vector is not a plucker line, will also return 0.
 *
 *  int is_plucker_line
 *      (const Vector *plucker_line);
 *  A boolean check to see if the given Vector is a plucker line. Returns 0 if
 *  false and 1 if true.
 *
 *  int line_projection_matrix_from_planes
 *      (const Vector *plane_one, const Vector *plane_two, const Vector *plane_three, Matrix **line_projection_matrix);
 *  Creates a 3x6 Matrix that is the line projection matrix derived from three
 *  4-vectors representing three planes.
 *
 * Author: Andrew Emmott (aemmott)
 *
 * -------------------------------------------------------------------------- */

/*Includes*/
#include <g/g_plucker.h>

#ifdef __cplusplus
extern "C" {
#endif
/* =============================================================================
 *                                 plucker_line_from_points
 *
 * Creates a plucker line from two three-dimensional points.
 *
 *  Creates a 6-vector plucker line from the elements in two 4-vectors
 *  representing two points in three-dimensional space.
 *
 * Returns:
 *    ERROR if either point is of innappropriate size or if allocation fails.
 *    NO_ERROR otherwise.
 *
 * See also:
 *    plucker_line_from_planes
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: plucker line
 * -------------------------------------------------------------------------- */
int plucker_line_from_points
(
    Vector **plucker_line, /*The target Vector pointer to put the new plucker line into.*/
    const Vector *point_one, /*The first point, as a homogenized 4-Vector.*/
    const Vector *point_two /*The second point, as a homogenized 4-Vector.*/
)
{
    /*Error Checking*/
    /*If either point is not a 4-vector, ERROR is returned.*/
    if (point_one->length != 4)
    {
        G_PLUCKER_SIZE_ERROR;
        return ERROR;
    }
    if (point_two->length != 4)
    {
        G_PLUCKER_SIZE_ERROR;
        return ERROR;
    }

    ERE(get_target_vector(plucker_line, 6));
    /*End Error Checking*/

    /*Matrix multiplication is avoided simply by calculating the six elements that are relevant to a plucker line.*/
    (*plucker_line)->elements[0] = (point_one->elements[0]*point_two->elements[1])-(point_one->elements[1]*point_two->elements[0]);
    (*plucker_line)->elements[1] = (point_one->elements[0]*point_two->elements[2])-(point_one->elements[2]*point_two->elements[0]);
    (*plucker_line)->elements[2] = (point_one->elements[0]*point_two->elements[3])-(point_one->elements[3]*point_two->elements[0]);
    (*plucker_line)->elements[3] = (point_one->elements[1]*point_two->elements[2])-(point_one->elements[2]*point_two->elements[1]);
    (*plucker_line)->elements[4] = (point_one->elements[3]*point_two->elements[1])-(point_one->elements[1]*point_two->elements[3]);
    (*plucker_line)->elements[5] = (point_one->elements[2]*point_two->elements[3])-(point_one->elements[3]*point_two->elements[2]);

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                                 plucker_line_from_planes
 *
 * Creates a plucker line from two planes in three-dimensional space.
 *
 *  Creates a 6-vector plucker line from the elements in two 4-vectors
 *  representing two planes in three-dimensional space.
 *
 * Returns:
 *    ERROR if either plane is of innappropriate size or if allocation fails.
 *    NO_ERROR otherwise.
 *
 * See also:
 *    plucker_line_from_points
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: plucker line
 * -------------------------------------------------------------------------- */
int plucker_line_from_planes
(
    Vector **plucker_line, /*The target Vector pointer to put the new plucker line into.*/
    const Vector *plane_one, /*The first plane, as a 4-Vector.*/
    const Vector *plane_two /*The second plane, as a 4-Vector.*/
)
{
    Vector *_vp;

    /*Error Checking*/
    /*If either plane is not a 4-vector, ERROR is returned.*/
    if (plane_one->length != 4)
    {
        G_PLUCKER_SIZE_ERROR;
        return ERROR;
    }
    if (plane_two->length != 4)
    {
        G_PLUCKER_SIZE_ERROR;
        return ERROR;
    }
    /*If allocation fails, ERROR is returned.*/
    if(get_target_vector(plucker_line, 6)==ERROR)
    {
        G_PLUCKER_ALLOC_ERROR;
        return ERROR;
    }
    /*End Error Checking*/

    _vp = *plucker_line;

    /*The calculations ot derive a plucker line from planes is identical to the one for points, but the elements are inverted.*/
    /*Code is duplicated here to avoid the processing time of inverting the elements.*/
    _vp->elements[5] = (plane_one->elements[2]*plane_two->elements[3])-(plane_one->elements[3]*plane_two->elements[2]);
    _vp->elements[4] = (plane_one->elements[3]*plane_two->elements[1])-(plane_one->elements[1]*plane_two->elements[3]);
    _vp->elements[3] = (plane_one->elements[1]*plane_two->elements[2])-(plane_one->elements[2]*plane_two->elements[1]);
    _vp->elements[2] = (plane_one->elements[0]*plane_two->elements[3])-(plane_one->elements[3]*plane_two->elements[0]);
    _vp->elements[1] = (plane_one->elements[0]*plane_two->elements[2])-(plane_one->elements[2]*plane_two->elements[0]);
    _vp->elements[0] = (plane_one->elements[0]*plane_two->elements[1])-(plane_one->elements[1]*plane_two->elements[0]);

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                                 invert_plucker_line
 *
 * Creates the dual representation of a plucker line by inverting the elements.
 *
 *  Creates a 6-vector plucker line that is the dual representation of an
 *  existing plucker line. (Inverts the order of the elements).
 *
 * Returns:
 *    ERROR if the Vector is not a plucker line or if allocation fails.
 *    NO_ERROR otherwise.
 *
 * See also:
 *    ow_invert_plucker_line
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: plucker line
 * -------------------------------------------------------------------------- */
int invert_plucker_line
(
    Vector **plucker_line, /*The target Vector pointer.*/
    const Vector *line /*The line to invert.*/
)
{
    Vector *_vp;
    int i;

    /*Error Checking*/
    /*If the line is not a valid plucker line, ERROR is returned.*/
    if(is_plucker_line(line) == 0)
    {
        return ERROR;
    }
    /*If allocation fails, ERROR is returned.*/
    if(get_target_vector(plucker_line, 6)==ERROR)
    {
        G_PLUCKER_ALLOC_ERROR;
        return ERROR;
    }
    /*End Error Checking*/

    _vp = *plucker_line;

    /*The order of the elements is simply swapped.*/
    for (i = 0; i < 6; i++)
    {
        _vp->elements[i] = line->elements[5-i];
    }

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                                 ow_invert_plucker_line
 *
 * Inverts the elements in a plucker line.
 *
 *  Directly changes the given plucker line, inverting the order of the elements,
 *  changing it to the dual representation of itself.
 *
 * Returns:
 *    ERROR if the Vector is not a plucker line.
 *    NO_ERROR otherwise.
 *
 * See also:
 *    invert_plucker_line
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: plucker line
 * -------------------------------------------------------------------------- */
int ow_invert_plucker_line
(
    Vector *line /*The line to invert.*/
)
{
    double swap_value; /*Classic temp value for swapping.*/
    int i;

    /*Error Checking*/
    /*If it is not a valid plucker line, ERROR is returned.*/
    if(is_plucker_line(line) == 0)
    {
        return ERROR;
    }
    /*End Error Checking*/

    /*Only three swaps are needed to invert all six elements.*/
    for (i = 0; i < 3; i++)
    {
        swap_value = line->elements[i];
        line->elements[i] = line->elements[5-i];
        line->elements[5-i] = swap_value;
    }

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                                 plucker_line_intersect
 *
 * Checks two plucker lines to see if they intersect.
 *
 *  A boolean check to see if two plucker lines intersect. Returns 0 if false and
 *  1 if true. If either Vector is not a plucker line, will also return 0.
 *
 * Returns:
 *    1 if the plucker lines intersect.
 *    0 if they do not, or on errors.
 *
 * See also:
 *    is_plucker_line
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: plucker line
 * -------------------------------------------------------------------------- */
int plucker_line_intersect
(
    const Vector *line_one, /*The first line to check*/
    const Vector *line_two /*The second line to check*/
)
{
    /*Error Checking*/
    /*If either line is not a valid plucker line, they do not intersect.*/
    if (is_plucker_line(line_one) == 0)
    {
        return 0;
    }
    if (is_plucker_line(line_two) == 0)
    {
        return 0;
    }
    /*End Error Checking*/
    
    /*The equation inside the if statement verifies that the lines intersect.*/
    if
    (
        (line_one->elements[0]*line_two->elements[5]) +
        (line_one->elements[1]*line_two->elements[4]) +
        (line_one->elements[2]*line_two->elements[3]) +
        (line_one->elements[3]*line_two->elements[2]) +
        (line_one->elements[4]*line_two->elements[1]) +
        (line_one->elements[5]*line_two->elements[0]) == 0
    )
    {
        return 1;
    }

    return 0;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                                 is_plucker_line
 *
 * Checks a Vector to see if it is a valid plucker line representation.
 *
 *  A boolean check to see if the given Vector is a plucker line. Returns 0 if
 *  false and 1 if true.
 *
 * Returns:
 *    1 if the Vector is a plucker line.
 *    0 if not, or on errors.
 *
 * See also:
 *    plucker_line_intersect
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: plucker line
 * -------------------------------------------------------------------------- */
int is_plucker_line
(
    const Vector *plucker_line /*The Vector to check against.*/
)
{
    Vector *normed = 0;
    /*If the line does not have six elements, it is not a plucker line.*/
    if (plucker_line->length != 6)
    {
        return 0;
    }
    normalize_vector(&normed,plucker_line,NORMALIZE_BY_MAGNITUDE);
    /*The equation inside the if statement verifies that the plucker line meets
    the proper constraints.*/
    if
    (
        fabs( (normed->elements[0]*normed->elements[5]) +
        (normed->elements[1]*normed->elements[4]) +
        (normed->elements[2]*normed->elements[3])) < DBL_EPSILON
    )
    {
        free_vector(normed);
        return 1;
    }
    free_vector(normed);
    return 0;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                                 line_projection_matrix_from_planes
 *
 * Creates a line projection matrix from three planes.
 *
 *  Creates a 3x6 Matrix that is the line projection matrix derived from three
 *  4-vectors representing three planes.
 *
 * Returns:
 *    ERROR if any Vector is of inappropriate size, or if allocation fails.
 *    NO_ERROR otherwise.
 *
 * See also:
 *    forward_line_projection
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: plucker line, line projection matrix
 * -------------------------------------------------------------------------- */
int line_projection_matrix_from_planes
(
    Matrix **line_projection_matrix, /*The Matrix pointer to store the new projection matrix*/
    const Vector *plane_one, /*The first plane as a 4-Vector*/
    const Vector *plane_two, /*The second plane as a 4-Vector*/
    const Vector *plane_three /*The second plane as a 4-Vector*/
)
{
    Matrix *_mp;

    /*Error Checking*/
    /*If either plane is not a 4-vector, ERROR is returned.*/
    if (plane_one->length != 4)
    {
        G_PLUCKER_SIZE_ERROR;
        return ERROR;
    }
    if (plane_two->length != 4)
    {
        G_PLUCKER_SIZE_ERROR;
        return ERROR;
    }
    if (plane_three->length != 4)
    {
        G_PLUCKER_SIZE_ERROR;
        return ERROR;
    }
    /*If allocation fails, ERROR is returned.*/
    if(get_target_matrix(line_projection_matrix, 3, 6)==ERROR)
    {
        G_PLUCKER_MAT_ALLOC_ERROR;
        return ERROR;
    }
    /*End Error Checking*/

    _mp = *line_projection_matrix;
    /*The matrix is actually three plucker lines representing the intersections of the planes.*/
    /*Calculated here to avoid function overhead.*/
    _mp->elements[0][5] = (plane_two->elements[2]*plane_three->elements[3])-(plane_two->elements[3]*plane_three->elements[2]);
    _mp->elements[0][4] = (plane_two->elements[3]*plane_three->elements[1])-(plane_two->elements[1]*plane_three->elements[3]);
    _mp->elements[0][3] = (plane_two->elements[1]*plane_three->elements[2])-(plane_two->elements[2]*plane_three->elements[1]);
    _mp->elements[0][2] = (plane_two->elements[0]*plane_three->elements[3])-(plane_two->elements[3]*plane_three->elements[0]);
    _mp->elements[0][1] = (plane_two->elements[0]*plane_three->elements[2])-(plane_two->elements[2]*plane_three->elements[0]);
    _mp->elements[0][0] = (plane_two->elements[0]*plane_three->elements[1])-(plane_two->elements[1]*plane_three->elements[0]);
    
    _mp->elements[1][5] = (plane_three->elements[2]*plane_one->elements[3])-(plane_three->elements[3]*plane_one->elements[2]);
    _mp->elements[1][4] = (plane_three->elements[3]*plane_one->elements[1])-(plane_three->elements[1]*plane_one->elements[3]);
    _mp->elements[1][3] = (plane_three->elements[1]*plane_one->elements[2])-(plane_three->elements[2]*plane_one->elements[1]);
    _mp->elements[1][2] = (plane_three->elements[0]*plane_one->elements[3])-(plane_three->elements[3]*plane_one->elements[0]);
    _mp->elements[1][1] = (plane_three->elements[0]*plane_one->elements[2])-(plane_three->elements[2]*plane_one->elements[0]);
    _mp->elements[1][0] = (plane_three->elements[0]*plane_one->elements[1])-(plane_three->elements[1]*plane_one->elements[0]);

    _mp->elements[2][5] = (plane_one->elements[2]*plane_two->elements[3])-(plane_one->elements[3]*plane_two->elements[2]);
    _mp->elements[2][4] = (plane_one->elements[3]*plane_two->elements[1])-(plane_one->elements[1]*plane_two->elements[3]);
    _mp->elements[2][3] = (plane_one->elements[1]*plane_two->elements[2])-(plane_one->elements[2]*plane_two->elements[1]);
    _mp->elements[2][2] = (plane_one->elements[0]*plane_two->elements[3])-(plane_one->elements[3]*plane_two->elements[0]);
    _mp->elements[2][1] = (plane_one->elements[0]*plane_two->elements[2])-(plane_one->elements[2]*plane_two->elements[0]);
    _mp->elements[2][0] = (plane_one->elements[0]*plane_two->elements[1])-(plane_one->elements[1]*plane_two->elements[0]);

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                 line_projection_matrix_from_point_project_matrix
 *
 * TODO Short Description
 *
 * TODO Description
 *
 * Returns:
 *    TODO Returns
 *
 * See also:
 *    TODO See also
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: TODO Index
 * -------------------------------------------------------------------------- */
int line_projection_matrix_from_point_project_matrix
(
    Matrix **line_projection_matrix, /*The Matrix pointer to store the new projection matrix*/
    const Matrix *point_projection_matrix /*The point projection matrix*/
)
{
    Vector *plane1 = NULL;
    Vector *plane2 = NULL;
    Vector *plane3 = NULL;
    ERE(get_matrix_row(&plane1, point_projection_matrix, 0));
    ERE(get_matrix_row(&plane2, point_projection_matrix, 1));
    ERE(get_matrix_row(&plane3, point_projection_matrix, 2));

    ERE(line_projection_matrix_from_planes(line_projection_matrix, plane1, plane2, plane3));

    free_vector(plane1);
    free_vector(plane2);
    free_vector(plane3);

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                                forward_line_projection 
 *
 * Makes a 3-vector by projecting a line onto a projection matrix. 
 *
 * Given a Line Projection Matrix and a Plucker Line, this function error checks
 * the inputs, then creates the projected 3-vector line by simply multiplying
 * the matrix and the plucker line together.
 *
 * Returns:
 *    ERROR if either input is inappropriate.
 *    NO_ERROR otherwise.
 *
 * See also:
 *    line_projection_matrix_from_planes
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: plucker line, line projection matrix
 * -------------------------------------------------------------------------- */
int forward_line_projection
(
    Vector **target_vector,
    const Matrix *line_projection_matrix,
    const Vector *plucker_line
)
{
    /*Error Checking*/
    /*If the matrix is the wrong size, ERROR is returned.*/
    if (line_projection_matrix->num_rows != 3 || line_projection_matrix->num_cols != 6)
    {
        G_PLUCKER_MAT_SIZE_ERROR;
        return ERROR;
    }
    /*If the line is not a plucker line, ERROR is returned.*/
    if (is_plucker_line(plucker_line) == 0)
    {
        return ERROR;
    }
    /*End Error Checking*/
    
    /*A simple multiply operation gets the desired result.*/
    multiply_matrix_and_vector(target_vector, line_projection_matrix, plucker_line);

    return NO_ERROR;
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif
