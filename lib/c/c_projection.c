
/* $Id: c_projection.c 6352 2010-07-11 20:13:21Z kobus $ */

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

#include "c/c_gen.h"        /* Only safe as first include in a ".c" file. */
#include "c/c_projection.h"


#define DEFAULT_MAX_PROJECTION_COORDINATE   10000.0

#define TRUNCATE_RATIO(x, y, max) \
            ((ABS_OF(x) >= max * (y)) ? (max) : (x) / (y))

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static double combine_error_measures
(
    double base_value,
    double absolute_error,
    double relative_error
);

/* -------------------------------------------------------------------------- */

/* =============================================================================
 *                            get_projection_matrix
 *
 * Projects RGB data into a 2-D space
 *
 * This routine projects RGB data into a 2-D space, normally a chromaticity
 * space specified by the argument "projection_method".
 *
 * "projection method" should be one of
 * DIVIDE_BY_RED, DIVIDE_BY_GREEN, DIVIDE_BY_BLUE, DIVIDE_BY_SUM,
 * ONTO_RG_PLANE, ONTO_GB_PLANE, or ONTO_RB_PLANE.
 *
 * If "projection_method" is one of DIVIDE_BY_RED, DIVIDE_BY_GREEN, or
 * DIVIDE_BY_BLUE, then the argument "max_projection_coordinate" should be used
 * to limit the size of the result in the case of denominators being too close
 * to zero. Ideally, the effect of attempting to exceed
 * "max_projection_coordinate" should be under the control of the argument
 * "invalid_chromaticity_action", but currently this argument is ignored.
 * However, in order to be compatable with future code, the argument
 * "invalid_chromaticity_action" should be set to TRUNCATE_INVALID_CHROMATICITY
 * for these three projection methods.
 *
 * If "projection_method" is DIVIDE_BY_SUM, then "max_projection_coordinate"
 * is used to specify the reciprocal of the smallest value of the sum which
 * is considered valid. For example, if "max_projection_coordinate" is 1000,
 * then any pixel with R+G+B < 0.001 is considered invalid, and an action
 * dictated by invalid_chromaticity_action is taken. Possible actions are
 * SKIP_INVALID_CHROMATICITY, ZERO_INVALID_CHROMATICITY, and
 * NEGATE_INVALID_CHROMATICITY.
 *
 * If "projection_method" is one of ONTO_RG_PLANE, ONTO_GB_PLANE, or
 * ONTO_RB_PLANE, the "max_projection_coordinate" argument is essentially
 * ignored, but should have the value DBL_NOT_SET.
 *
 * If "max_projection_coordinate" is negative, then a default value is used.
 *
 * If one (or both) of "error_box_mpp" or "error_points_mpp" are not NULL,
 * and "projection method" is one of DIVIDE_BY_RED, DIVIDE_BY_GREEN,
 * DIVIDE_BY_BLUE, or DIVIDE_BY_SUM, then a result representing the range of
 * points based on "absolute_pixel_error" and "relative_pixel_error" is
 * computed. If "error_box_mpp" and "error_points_mpp" are both NULL, then
 * the error parameters "absolute_pixel_error" and "relative_pixel_error" are
 * ignored, but by convention they should have the value DBL_NOT_SET.
 *
 * A more specific routine get_divide_by_sum_projection_matrix() is available if
 * it is known in advance that the projection method will be DIVIDE_BY_SUM.
 *
 * Note:
 *    The semantics of the routines in this module are ugly, and hopefully they
 *    will be changed for the better. However, it is recomended that these
 *    routines are used for colour projection regardless, to be consistent.
 *    Comments are welcome. (In other words, help use fix (and test!) this
 *    stuff, rather than reinventing the wheel).
 *
 * Note:
 *    If the projection method is DIVIDE_BY_SUM, then the number of rows in the
 *    projected data can be less than that of the input. Similarly, the number
 *    of rows in the error results are not easily determined from the input
 *    size.
 *
 * Returns:
 *    NO_ERROR on success; ERROR on failure.
 *
 * Documentors:
 *    Kobus Barnard, Lindsay Martin
 *
 * Index: chromaticity, colour, projection
 *
 * -----------------------------------------------------------------------------
*/

int get_projection_matrix
(
    Matrix**                    output_mpp,                 /* Output chromaticity matrix.   */
    const Matrix*               input_mp,                   /* Input RGB matrix. */
    Projection_method           projection_method,          /* Type of chromaticity.         */
    Invalid_chromaticity_action invalid_chromaticity_action,
    double                      max_projection_coordinate,  /*Max value in projected coords. */
    double                      absolute_pixel_error,
    double                      relative_pixel_error,
    Matrix**                    error_box_mpp,
    Matrix**                    error_points_mpp
)
{
    Matrix* output_mp;
    int     num_rows;
    int     i;
    int     first_index;
    int     second_index;
    int     third_index;
    double    temp_first;
    double    temp_second;
    double    temp_third;
    double    min_first;
    double    max_first;
    double    min_second;
    double    max_second;
    double    min_third;
    double    max_third;
    Matrix* error_box_mp = NULL;
    double    pixel_error;


    if (input_mp->num_cols != NUM_SENSORS  /* 3 */ )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (max_projection_coordinate <= 0.0)
    {
        max_projection_coordinate = DEFAULT_MAX_PROJECTION_COORDINATE;
    }

    if (projection_method == ONTO_RG_PLANE)
    {
        if (    (error_box_mpp != NULL)   || (error_points_mpp != NULL)
             || (absolute_pixel_error > 0.0) || (relative_pixel_error > 0.0)
           )
        {
            set_bug("Error propogation is not available with ONTO_RG_PLANE.");
            return ERROR;
        }
        else
        {
            return coord_plane_project_matrix(output_mpp, input_mp,
                                              RED_INDEX, GREEN_INDEX);
        }
    }
    else if (projection_method == ONTO_GB_PLANE)
    {
        if (    (error_box_mpp != NULL)   || (error_points_mpp != NULL)
             || (absolute_pixel_error > 0.0) || (relative_pixel_error > 0.0)
           )
        {
            set_bug("Error propogation is not available with ONTO_GB_PLANE.");
            return ERROR;
        }
        else
        {
            return coord_plane_project_matrix(output_mpp, input_mp,
                                              GREEN_INDEX, BLUE_INDEX);
        }
    }
    else if (projection_method == ONTO_RB_PLANE)
    {
        if (    (error_box_mpp != NULL)   || (error_points_mpp != NULL)
             || (absolute_pixel_error > 0.0) || (relative_pixel_error > 0.0)
           )
        {
            set_bug("Error propogation is not available with ONTO_RB_PLANE.");
            return ERROR;
        }
        else
        {
            return coord_plane_project_matrix(output_mpp, input_mp,
                                              RED_INDEX, BLUE_INDEX);
        }
    }
    else if (projection_method == DIVIDE_BY_SUM)
    {
        if (error_box_mpp != NULL)
        {
            set_bug("Request for error box is not valid with DIVIDE_BY_SUM.");
            return ERROR;
        }
        else
        {
            return get_divide_by_sum_projection_matrix(output_mpp, input_mp,
                                                   invalid_chromaticity_action,
                                                   max_projection_coordinate,
                                                   absolute_pixel_error,
                                                   relative_pixel_error,
                                                   error_points_mpp);
        }
    }
    else if (projection_method == ONTO_UNIT_SPHERE)
    {
        if (error_box_mpp != NULL)
        {
            set_bug("Request for error box is not valid with ONTO_UNIT_SPHERE.");
            return ERROR;
        }
        else
        {
            return project_matrix_onto_unit_sphere(output_mpp, input_mp);
        }
    }
    else if (projection_method == DIVIDE_BY_RED)
    {
        first_index  = GREEN_INDEX;
        second_index = BLUE_INDEX;
        third_index  = RED_INDEX;
    }
    else if (projection_method == DIVIDE_BY_GREEN)
    {
        first_index  = RED_INDEX;
        second_index = BLUE_INDEX;
        third_index  = GREEN_INDEX;
    }
    else if (projection_method == DIVIDE_BY_BLUE)
    {
        first_index  = RED_INDEX;
        second_index = GREEN_INDEX;
        third_index  = BLUE_INDEX;
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    /* These have not been implemented yet in the case of dividing by one of the
    // componants.
    */
    if (    (invalid_chromaticity_action == SKIP_INVALID_CHROMATICITY)
         || (invalid_chromaticity_action == ZERO_INVALID_CHROMATICITY)
         || (invalid_chromaticity_action == NEGATE_INVALID_CHROMATICITY)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_rows = input_mp->num_rows;

    ERE(get_target_matrix(output_mpp, num_rows, 2));
    output_mp = *output_mpp;

    if ((error_box_mpp != NULL) || (error_points_mpp != NULL))
    {
        NRE(error_box_mp = create_matrix(num_rows, 4));
    }

    for (i=0; i<num_rows; i++)
    {
        temp_third = (input_mp->elements)[i][ third_index ];

        min_third = max_third = temp_third;

        if (error_box_mp != NULL)
        {
            pixel_error = combine_error_measures(min_third,
                                                 absolute_pixel_error,
                                                 relative_pixel_error);

            min_third -= pixel_error;

            pixel_error = combine_error_measures(max_third,
                                                 absolute_pixel_error,
                                                 relative_pixel_error);

            max_third += pixel_error;
        }

        temp_first = (input_mp->elements)[i][ first_index ];
        min_first = max_first = temp_first;

        temp_second = (input_mp->elements)[i][ second_index ];
        min_second = max_second = temp_second;

        if (error_box_mp != NULL)
        {
            pixel_error = combine_error_measures(min_first,
                                                 absolute_pixel_error,
                                                 relative_pixel_error);

            min_first -= pixel_error;

            if (min_first < 0.0) min_first = 0.0;


            pixel_error = combine_error_measures(min_second,
                                                 absolute_pixel_error,
                                                 relative_pixel_error);

            min_second -= pixel_error;

            if (min_second < 0.0) min_second = 0.0;


            pixel_error = combine_error_measures(max_first,
                                                 absolute_pixel_error,
                                                 relative_pixel_error);

            max_first += pixel_error;

            pixel_error = combine_error_measures(max_second,
                                                 absolute_pixel_error,
                                                 relative_pixel_error);

            max_second += pixel_error;
        }

        if (   (invalid_chromaticity_action == ALL_CHROMATICITIES_MUST_BE_VALID)
             &&(temp_third <= 0.0)
           )
        {
            free_matrix(error_box_mp);
            set_error("Invalid chromaticity found.");
            return ERROR;
        }

        (output_mp->elements)[i][0] = TRUNCATE_RATIO(temp_first, temp_third,
                                                     max_projection_coordinate);

        (output_mp->elements)[i][1] = TRUNCATE_RATIO(temp_second, temp_third,
                                                     max_projection_coordinate);

        if (error_box_mp != NULL)
        {
            (error_box_mp->elements)[i][0] = TRUNCATE_RATIO(min_first,
                                                            max_third,
                                                    max_projection_coordinate);

            (error_box_mp->elements)[i][1] = TRUNCATE_RATIO(max_first,
                                                            min_third,
                                                    max_projection_coordinate);

            (error_box_mp->elements)[i][2] = TRUNCATE_RATIO(min_second,
                                                            max_third,
                                                    max_projection_coordinate);

            (error_box_mp->elements)[i][3] = TRUNCATE_RATIO(max_second,
                                                            min_third,
                                                    max_projection_coordinate);
        }
    }

    if (error_box_mp != NULL)
    {
        if (error_points_mpp != NULL)
        {
            Matrix *error_points_mp;

            error_points_mp = expand_error_box(error_box_mp);

            if (error_points_mp == NULL)
            {
                free_matrix(error_box_mp);
                return ERROR;
            }
            else
            {
                (*error_points_mpp) = error_points_mp;
            }
        }

        if (error_box_mpp != NULL)
        {
            (*error_box_mpp) = error_box_mp;
        }
        else
        {
            free_matrix(error_box_mp);
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     get_divide_by_sum_projection_matrix
 *
 * Projects RGB data into rg space
 *
 * This routine projects RGB data into the rg chromaticity space.
 *
 * The input matrix should have either 3 or 4 columns. The number of columns of
 * the output matrix is one less than that of the input matrix. If the input
 * matrix has 4 columns, the first column is assumed to be a value which should
 * be propogated into the chromaticity output, and it is copied to the first
 * column of the output matrix.
 *
 * The argument "max_projection_coordinate" is used to specify the reciprocal of
 * the smallest value of the sum which is considered valid. For example, if
 * "max_projection_coordinate" is 1000, then any triplet with R+G+B < 0.001 is
 * considered invalid, and an action dictated by invalid_chromaticity_action is
 * taken. Possible actions are SKIP_INVALID_CHROMATICITY,
 * ZERO_INVALID_CHROMATICITY, and NEGATE_INVALID_CHROMATICITY.
 *
 * The name and behaviour of "max_projection_coordinate" is currently chosen to
 * be consistent with the arguments to the more general function
 * get_projection_matrix().
 *
 * If "max_projection_coordinate" is negative, then a default value is used.
 *
 * If "error_points_mpp" is not NULL, then a result representing the range of
 * points based on the "absolute_pixel_error" and "relative_pixel_error" is
 * computed.  If "error_points_mpp" is NULL, then the error parameters "
 * absolute_pixel_error" and "relative_pixel_error" are ignored, but by
 * convention they should have the value DBL_NOT_SET.
 *
 * A more general routine get_projection_matrix() is available.
 *
 * Note:
 *    The semantics of the routines in this module are ugly, and hopefully they
 *    will be changed for the better. However, it is recomended that these
 *    routines are used for colour projection regardless, to be consistent.
 *
 * Note:
 *    The number of rows in the projected data can be less than that of the
 *    input. Similarly, the number of rows in the error results are not easily
 *    determined from the input size.
 *
 * Returns:
 *    NO_ERROR on success; ERROR on failure.
 *
 * Related:
 *    get_projection_matrix
 *
 * Documentors:
 *    Kobus Barnard, Lindsay Martin
 *
 * Index: chromaticity, colour, projection
 *
 * -----------------------------------------------------------------------------
*/

int get_divide_by_sum_projection_matrix
(
    Matrix**                    output_mpp,
    const Matrix*               input_mp,
    Invalid_chromaticity_action invalid_chromaticity_action,
    double                      max_projection_coordinate,
    double                      absolute_pixel_error,
    double                      relative_pixel_error,
    Matrix**                    error_points_mpp
)
{
    Matrix*  output_mp;
    int      num_rows;
    int      count = 0, i;
    double     r;
    double     g;
    double     b;
    double     sum;
    double     min_sum;
    int        extra_col = 0;


    if (input_mp->num_rows < 1)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (input_mp->num_cols == 1 + NUM_SENSORS  /* 3 */ )
    {
        extra_col = 1;
    }
    else if (input_mp->num_cols != NUM_SENSORS  /* 3 */ )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (max_projection_coordinate <= 0.0)
    {
        max_projection_coordinate = DEFAULT_MAX_PROJECTION_COORDINATE;
    }

    min_sum = 1.0 / max_projection_coordinate;

    num_rows = input_mp->num_rows;

    ERE(get_target_matrix(output_mpp, num_rows, 2 + extra_col));
    output_mp = *output_mpp;

    for (i=0; i<num_rows; i++)
    {
        if (extra_col)
        {
            output_mp->elements[count][ 0 ] = input_mp->elements[i][ 0 ];
        }

        r = (input_mp->elements)[i][ extra_col + RED_INDEX ];
        g = (input_mp->elements)[i][ extra_col + GREEN_INDEX ];
        b = (input_mp->elements)[i][ extra_col + BLUE_INDEX ];

        if (r < 0.0) r = 0.0;
        if (g < 0.0) g = 0.0;
        if (b < 0.0) b = 0.0;

        sum = r + g + b;

        if (sum >= min_sum)
        {
            (output_mp->elements)[count][ extra_col + RED_INDEX ] = r / sum;
            (output_mp->elements)[count][ extra_col + GREEN_INDEX ] = g / sum;
            count++;
        }
        else if (invalid_chromaticity_action == ZERO_INVALID_CHROMATICITY)
        {
            (output_mp->elements)[count][ extra_col + RED_INDEX ]   = 0.0;
            (output_mp->elements)[count][ extra_col + GREEN_INDEX ] = 0.0;
            count++;
        }
        else if (invalid_chromaticity_action == NEGATE_INVALID_CHROMATICITY)
        {
            (output_mp->elements)[count][ extra_col + RED_INDEX ]   = DBL_NOT_SET;
            (output_mp->elements)[count][ extra_col + GREEN_INDEX ] = DBL_NOT_SET;
            count++;
        }
        else if (invalid_chromaticity_action ==ALL_CHROMATICITIES_MUST_BE_VALID)
        {
            set_error("Invalid RGB for chromaticity found.\n");
            return ERROR;
        }
        else if (invalid_chromaticity_action != SKIP_INVALID_CHROMATICITY)
        {
            SET_CANT_HAPPEN_BUG();
            return ERROR;
        }

    }

    if (count == 0)
    {
        set_error("No good RGB data found when converting to divide by sum ");
        cat_error("chromaticity.");
        return ERROR;
    }

    output_mp->num_rows = count;

    if (error_points_mpp != NULL)
    {
        Matrix* error_box_mp = NULL;
        Matrix* input_error_points_mp = NULL;
        Matrix* error_points_mp = NULL;
        double  pixel_error;
        int     j;


        ERE(get_target_matrix(&error_box_mp, num_rows, 6));

        for (i=0; i<num_rows; i++)
        {
            for (j=0; j<3; j++)
            {
                pixel_error = combine_error_measures((input_mp->elements)[i][j],
                                                     absolute_pixel_error,
                                                     relative_pixel_error);

                (error_box_mp->elements)[i][2*j] =
                                      (input_mp->elements)[i][ j ] - pixel_error;

                (error_box_mp->elements)[i][2*j+1] =
                                      (input_mp->elements)[i][ j ] + pixel_error;
            }
        }

        input_error_points_mp = expand_error_box(error_box_mp);

        if (input_error_points_mp == NULL)
        {
            free_matrix(error_box_mp);
            return ERROR;
        }

        num_rows = input_error_points_mp->num_rows;

        if (get_target_matrix(&error_points_mp, num_rows, 2) == ERROR)
        {
            free_matrix(input_error_points_mp);
            free_matrix(error_box_mp);
            return ERROR;
        }

        count = 0;

        for (i=0; i<num_rows; i++)
        {
            r = (input_error_points_mp->elements)[i][ RED_INDEX ];
            g = (input_error_points_mp->elements)[i][ GREEN_INDEX ];
            b = (input_error_points_mp->elements)[i][ BLUE_INDEX ];

            if (r < 0.0) r = 0.0;
            if (g < 0.0) g = 0.0;
            if (b < 0.0) b = 0.0;

            sum = r + g + b;

            if (sum > min_sum)
            {
                (error_points_mp->elements)[ count ][ RED_INDEX ] = r / sum;
                (error_points_mp->elements)[ count ][ GREEN_INDEX ] = g / sum;
                count++;
            }
        }

        free_matrix(error_box_mp);
        free_matrix(input_error_points_mp);

        ASSERT(count > 0);

        error_points_mp->num_rows = count;
        (*error_points_mpp) = error_points_mp;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            project_matrix
 *
 * Projects RGB data into a 2-D space
 *
 * This routine projects RGB data into a 2-D space, normally a chromaticity
 * space specified by the argument "projection_method".
 *
 * "projection method" should be one of
 * DIVIDE_BY_RED, DIVIDE_BY_GREEN, DIVIDE_BY_BLUE, DIVIDE_BY_SUM,
 * ONTO_RG_PLANE, ONTO_GB_PLANE, or ONTO_RB_PLANE.
 *
 * This projection routine is a simpler interface to similar functionality as
 * provided by get_projection_matrix(). If more options are required, then that
 * routine should be used instead. Note, however, that the result of the two
 * routines can differ when the data is small or negative.
 *
 * Warning:
 *    I am still working on this library. Be prepared for change!
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure. The usuall reason for failure is
 *    that a projection denominator is less than DBL_EPSILON.
 *
 * Documentors:
 *    Kobus Barnard, Lindsay Martin
 *
 * Index: chromaticity, colour, projection
 *
 * -----------------------------------------------------------------------------
*/

int project_matrix
(
    Matrix**          output_mpp, /* Output chromaticity matrix.   */
    const Matrix*     input_mp,   /* Input RGB matrix. */
    Projection_method projection_method   /* Type of chromaticity.  */
)
{
    Matrix* output_mp;
    int     num_rows;
    int     i;
    int     first_index;
    int     second_index;
    int     third_index;
    double    temp_first;
    double    temp_second;
    double    temp_third;


    if (input_mp->num_cols != NUM_SENSORS  /* 3 */ )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (projection_method == ONTO_RG_PLANE)
    {
        return coord_plane_project_matrix(output_mpp, input_mp,
                                          RED_INDEX, GREEN_INDEX);
    }
    else if (projection_method == ONTO_GB_PLANE)
    {
        return coord_plane_project_matrix(output_mpp, input_mp,
                                          GREEN_INDEX, BLUE_INDEX);
    }
    else if (projection_method == ONTO_RB_PLANE)
    {
        return coord_plane_project_matrix(output_mpp, input_mp,
                                          RED_INDEX, BLUE_INDEX);
    }
    else if (projection_method == DIVIDE_BY_SUM)
    {
        return divide_by_sum_project_matrix(output_mpp, input_mp);
    }
    else if (projection_method == ONTO_UNIT_SPHERE)
    {
        return project_matrix_onto_unit_sphere(output_mpp, input_mp);
    }
    else if (projection_method == DIVIDE_BY_RED)
    {
        first_index  = GREEN_INDEX;
        second_index = BLUE_INDEX;
        third_index  = RED_INDEX;
    }
    else if (projection_method == DIVIDE_BY_GREEN)
    {
        first_index  = RED_INDEX;
        second_index = BLUE_INDEX;
        third_index  = GREEN_INDEX;
    }
    else if (projection_method == DIVIDE_BY_BLUE)
    {
        first_index  = RED_INDEX;
        second_index = GREEN_INDEX;
        third_index  = BLUE_INDEX;
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    num_rows = input_mp->num_rows;

    ERE(get_target_matrix(output_mpp, num_rows, 2));
    output_mp = *output_mpp;

    for (i=0; i<num_rows; i++)
    {
        temp_first = (input_mp->elements)[i][ first_index ];
        temp_second = (input_mp->elements)[i][ second_index ];
        temp_third = (input_mp->elements)[i][ third_index ];

#define MIN_DIVISION_COORDINATE_VALUE  100.0 * DBL_EPSILON

        if (temp_third < MIN_DIVISION_COORDINATE_VALUE)
        {
            set_error("Division coordinate is too small for projection.");
            add_error("Coordinate %d of row %d is is %.3e.", third_index + 1,
                      i + 1, temp_third);
            add_error("It must be at least %.3e.",
                      MIN_DIVISION_COORDINATE_VALUE);
            return ERROR;
        }

        (output_mp->elements)[ i ][ 0 ] = temp_first / temp_third;
        (output_mp->elements)[ i ][ 1 ] = temp_second / temp_third;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     divide_by_sum_project_matrix
 *
 * Projects RGB data into rg space
 *
 * This routine projects RGB data into the rg chromaticity space.
 *
 * This projection routine is a simpler interface to similar functionality as
 * provided by get_divide_by_sum_projection_matrix(). If more options are
 * required, then that routine should be used instead. Note, however, that the
 * result of the two routines can differ when the data is small or negative.
 * There is even a more general routine "get_projection_matrix".
 *
 * Warning:
 *    I am still working on this library. Be prepared for change!
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure. The usuall reason for failure is
 *    that a projection denominator is less than DBL_EPSILON.
 *
 * Related:
 *    get_projection_matrix, get_divide_by_sum_projection_matrix
 *
 * Documentors:
 *    Kobus Barnard, Lindsay Martin
 *
 * Index: chromaticity, colour, projection
 *
 * -----------------------------------------------------------------------------
*/

int divide_by_sum_project_matrix
(
    Matrix**      output_mpp,
    const Matrix* input_mp
)
{
    Matrix* output_mp;
    int     num_rows, i;
    double    r, g, b, sum;


    if (    (input_mp->num_cols != NUM_SENSORS  /* 3 */ )
         || (input_mp->num_rows < 1)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_rows = input_mp->num_rows;
    ERE(get_target_matrix(output_mpp, num_rows, 2));
    output_mp = *output_mpp;

    for (i=0; i<num_rows; i++)
    {
        r = (input_mp->elements)[i][ RED_INDEX ];
        g = (input_mp->elements)[i][ GREEN_INDEX ];
        b = (input_mp->elements)[i][ BLUE_INDEX ];

        sum = r + g + b;

        if (sum < DBL_EPSILON)
        {
            set_error("Division coordinate is too small for projection.");
            add_error("It must be at least positive epsilon.");
            return ERROR;
        }

        (output_mp->elements)[ i ][ RED_INDEX ] = r / sum;
        (output_mp->elements)[ i ][ GREEN_INDEX ] = g / sum;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                     project_matrix_onto_unit_sphere
 *
 * Projects RGB data onto unit sphere
 *
 * This routine projects RGB data onto unit sphere (phi, theta).
 *
 * Warning:
 *    I am still working on this library. Be prepared for change!
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure. The usuall reason for failure is
 *    that a projection denominator is less than DBL_EPSILON.
 *
 * Related:
 *    get_projection_matrix
 *
 * Documentors:
 *    Kobus Barnard
 *
 * Index: chromaticity, colour, projection
 *
 * -----------------------------------------------------------------------------
*/

int project_matrix_onto_unit_sphere
(
    Matrix**      output_mpp,
    const Matrix* input_mp
)
{
    Matrix* output_mp;
    int     num_rows, i;
    double  r, g, b, r2_plus_g2, R, R_rg;


    if (    (input_mp->num_cols != NUM_SENSORS  /* 3 */ )
         || (input_mp->num_rows < 1)
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    num_rows = input_mp->num_rows;
    ERE(get_target_matrix(output_mpp, num_rows, 2));
    output_mp = *output_mpp;

    for (i=0; i<num_rows; i++)
    {
        r = (input_mp->elements)[i][ RED_INDEX ];
        g = (input_mp->elements)[i][ GREEN_INDEX ];
        b = (input_mp->elements)[i][ BLUE_INDEX ];

        r2_plus_g2 = r*r + g*g;
        R = sqrt(r2_plus_g2 + b*b);
        R_rg = sqrt(r2_plus_g2);

        (output_mp->elements)[ i ][ RED_INDEX ] = acos(b / R);
        (output_mp->elements)[ i ][ GREEN_INDEX ] = acos(r / R_rg);
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int coord_plane_project_matrix
(
    Matrix**      output_mpp,
    const Matrix* input_mp,
    int           coord_1,
    int           coord_2
)
{
    int i;


    ERE(get_target_matrix(output_mpp, input_mp->num_rows, 2));

    for (i=0; i<input_mp->num_rows; i++)
    {
        (*output_mpp)->elements[ i ][ 0 ] = input_mp->elements[ i ][coord_1];
        (*output_mpp)->elements[ i ][ 1 ] = input_mp->elements[ i ][coord_2];
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            get_projection_vector
 *
 * Projects RGB vector into a 2-D space
 *
 * This routine projects RGB data into a 2-D space, normally a chromaticity
 * space. The projection method used is specified by the argument
 * "projection_method". This argument should be one of DIVIDE_BY_RED,
 * DIVIDE_BY_GREEN, DIVIDE_BY_BLUE, DIVIDE_BY_SUM, ONTO_RG_PLANE, ONTO_GB_PLANE,
 * and ONTO_RB_PLANE.
 *
 * If "projection_method" is one of DIVIDE_BY_RED, DIVIDE_BY_GREEN,
 * DIVIDE_BY_BLUE, then the argument "max_projection_coordinate" should be used
 * to limit the size of the result in the case of denominators being too close
 * to zero. If "projection_method" is DIVIDE_BY_SUM, then this argument is
 * over-loaded to perform a similar function, but now it is the reciprocal of
 * the smallest value of the sum which is considered valid. For example, if
 * "max_projection_coordinate" is 1000, and R+G+B < 0.001, then that pixel is
 * considered invalid, and ERROR is returned.
 *
 * If "projection_method" not one of the above, then this argument is
 * essentially ignored, but it should be DBL_NOT_SET.
 *
 * If "max_projection_coordinate" is negative, then a default value is used.
 *
 * Note:
 *    The semantics of the routines in this module are ugly, and hopefully they
 *    will be changed for the better. However, it is recomended that these
 *    routines are used for colour projection regardless, to be consistent.
 *    Comments are welcome. (In other words, help use fix (and test!) this
 *    stuff, rather than reinventing the wheel).
 *
 * Index: chromaticity, colour, projection
 *
 * -----------------------------------------------------------------------------
*/

int get_projection_vector
(
    Vector**          output_vpp,
    const Vector*     input_vp,
    Projection_method projection_method,
    double            max_projection_coordinate
)
{
    int     first_index;
    int     second_index;
    double    temp_first;
    double    temp_second;
    double    temp_third;
    Vector* output_vp    = NULL;
    int     result       = NO_ERROR;


    if (input_vp->length != 3)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (max_projection_coordinate <= 0.0)
    {
        max_projection_coordinate = DEFAULT_MAX_PROJECTION_COORDINATE;
    }

    ERE(get_target_vector(&output_vp, 2));

    if (projection_method == ONTO_RG_PLANE)
    {
        first_index  = RED_INDEX;
        second_index = GREEN_INDEX;
        temp_third   = 1.0;
    }
    else if (projection_method == ONTO_GB_PLANE)
    {
        first_index  = GREEN_INDEX;
        second_index = BLUE_INDEX;
        temp_third   = 1.0;
    }
    else if (projection_method == ONTO_RB_PLANE)
    {
        first_index  = RED_INDEX;
        second_index = BLUE_INDEX;
        temp_third   = 1.0;
    }
    else if (projection_method == DIVIDE_BY_SUM)
    {
        first_index = RED_INDEX;
        second_index = GREEN_INDEX;

        temp_third  = (input_vp->elements)[ RED_INDEX ]
                                      + (input_vp->elements)[ GREEN_INDEX ]
                                      + (input_vp->elements)[ BLUE_INDEX ];
    }
    else if (projection_method == DIVIDE_BY_RED)
    {
        first_index = GREEN_INDEX;
        second_index = BLUE_INDEX;
        temp_third = (input_vp->elements)[ RED_INDEX ];
    }
    else if (projection_method == DIVIDE_BY_GREEN)
    {
        first_index = RED_INDEX;
        second_index = BLUE_INDEX;
        temp_third = (input_vp->elements)[ GREEN_INDEX ];
    }
    else if (projection_method == DIVIDE_BY_BLUE)
    {
        first_index = RED_INDEX;
        second_index = GREEN_INDEX;
        temp_third = (input_vp->elements)[ BLUE_INDEX ];
    }
    else
    {
#ifdef __GNUC__
        /* Reduce noise from the error checkers. */
        first_index  = NOT_SET;
        second_index = NOT_SET;
        temp_third   = DBL_NOT_SET;
#endif
        SET_CANT_HAPPEN_BUG();
        result = ERROR;
    }

    if (result != ERROR)
    {
        temp_first  = (input_vp->elements)[ first_index  ];
        temp_second = (input_vp->elements)[ second_index ];

        if (projection_method == DIVIDE_BY_SUM)
        {
            double min_sum = 1.0 / max_projection_coordinate;

            if (temp_third < min_sum)
            {
                set_error("Unable to get rg chrom of pixel which is too dark.");
                result =  ERROR;
            }
            else
            {
                (output_vp->elements)[ 0 ] = temp_first / temp_third;
                (output_vp->elements)[ 1 ] = temp_second / temp_third;
            }
        }
        else
        {
            (output_vp->elements)[0] = TRUNCATE_RATIO(temp_first, temp_third,
                                                     max_projection_coordinate);

            (output_vp->elements)[1] = TRUNCATE_RATIO(temp_second, temp_third,
                                                     max_projection_coordinate);
        }
    }

    if (result != ERROR)
    {
        result = copy_vector(output_vpp, output_vp);
    }

    free_vector(output_vp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                            project_vector
 *
 * Projects RGB vector into a 2-D space
 *
 * This routine projects RGB data into a 2-D space, normally a chromaticity
 * space. The projection method used is specified by the argument
 * "projection_method". This argument should be one of DIVIDE_BY_RED,
 * DIVIDE_BY_GREEN, DIVIDE_BY_BLUE, DIVIDE_BY_SUM, ONTO_RG_PLANE, ONTO_GB_PLANE,
 * and ONTO_RB_PLANE.
 *
 * Note:
 *    This routine offers simpler semantics to similar (but not identical)
 *    operation as get_projection_vector(). The semantics differ if the
 *    componants are non-positive.
 *
 * Warning:
 *    I am still working on this library. Be prepared for change!
 *
 * Returns:
 *    NO_ERROR on success and ERROR on failure. The usuall reason for failure is
 *    that the denominator is less than DBL_EPSILON.
 *
 * Index: chromaticity, colour, projection
 *
 * -----------------------------------------------------------------------------
*/

int project_vector
(
    Vector**          output_vpp,
    const Vector*     input_vp,
    Projection_method projection_method
)
{
    Vector* output_vp;
    int     first_index;
    int     second_index;
    double    temp_first;
    double    temp_second;
    double    temp_third;


    if (input_vp->length != 3)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_vector(output_vpp, 2));
    output_vp = *output_vpp;

    if (projection_method == ONTO_RG_PLANE)
    {
        output_vp->elements[ 0 ] = input_vp->elements[ RED_INDEX ];
        output_vp->elements[ 1 ] = input_vp->elements[ GREEN_INDEX ];
        return NO_ERROR;
    }
    else if (projection_method == ONTO_GB_PLANE)
    {
        output_vp->elements[ 0 ] = input_vp->elements[ GREEN_INDEX ];
        output_vp->elements[ 1 ] = input_vp->elements[ BLUE_INDEX ];
        return NO_ERROR;
    }
    else if (projection_method == ONTO_RB_PLANE)
    {
        output_vp->elements[ 0 ] = input_vp->elements[ RED_INDEX ];
        output_vp->elements[ 1 ] = input_vp->elements[ BLUE_INDEX ];
        return NO_ERROR;
    }
    else if (projection_method == DIVIDE_BY_SUM)
    {
        first_index = RED_INDEX;
        second_index = GREEN_INDEX;

        temp_third  = (input_vp->elements)[ RED_INDEX ]
                                      + (input_vp->elements)[ GREEN_INDEX ]
                                      + (input_vp->elements)[ BLUE_INDEX ];
    }
    else if (projection_method == ONTO_UNIT_SPHERE)
    {
        double  r, g, b, r2_plus_g2, R, R_rg;

        r = (input_vp->elements)[ RED_INDEX ];
        g = (input_vp->elements)[ GREEN_INDEX ];
        b = (input_vp->elements)[ BLUE_INDEX ];

        r2_plus_g2 = r*r + g*g;
        R = sqrt(r2_plus_g2 + b*b);
        R_rg = sqrt(r2_plus_g2);

        (output_vp->elements)[ RED_INDEX ] = acos(b / R);
        (output_vp->elements)[ GREEN_INDEX ] = acos(r / R_rg);

        return NO_ERROR;
    }
    else if (projection_method == DIVIDE_BY_RED)
    {
        first_index = GREEN_INDEX;
        second_index = BLUE_INDEX;
        temp_third = (input_vp->elements)[ RED_INDEX ];
    }
    else if (projection_method == DIVIDE_BY_GREEN)
    {
        first_index = RED_INDEX;
        second_index = BLUE_INDEX;
        temp_third = (input_vp->elements)[ GREEN_INDEX ];
    }
    else if (projection_method == DIVIDE_BY_BLUE)
    {
        first_index = RED_INDEX;
        second_index = GREEN_INDEX;
        temp_third = (input_vp->elements)[ BLUE_INDEX ];
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    temp_first  = (input_vp->elements)[ first_index  ];
    temp_second = (input_vp->elements)[ second_index ];

    if (temp_third < DBL_EPSILON)
    {
        set_error("Division coordinate is too small for projection.");
        add_error("It must be at least positive epsilon.");
        return ERROR;
    }

    (output_vp->elements)[0] = temp_first / temp_third;
    (output_vp->elements)[1] = temp_second / temp_third;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             back_project_matrix
 *
 * Converts chromaticity to RGB
 *
 * This routine converts the chromaticities in the matrix "*input_mp" to RGB
 * using projection method designated by "projection_method". Not all projection
 * methods are valid, and sending this routine and invalid one is treated as a
 * bug. The value ones currently are DIVIDE_BY_RED, DIVIDE_BY_GREEN,
 * DIVIDE_BY_BLUE, and DIVIDE_BY_SUM.
 *
 * Index: chromaticity, colour, projection
 *
 * -----------------------------------------------------------------------------
*/

int back_project_matrix
(
    Matrix**          output_mpp,
    const Matrix*     input_mp,
    Projection_method projection_method
)
{
    Matrix* output_mp;
    int     first_index;
    int     second_index;
    int     third_index;
    int     i;


    if (    (input_mp->num_cols != 2)
         ||
            (    (projection_method != DIVIDE_BY_SUM)
              && (projection_method != DIVIDE_BY_RED)
              && (projection_method != DIVIDE_BY_GREEN)
              && (projection_method != DIVIDE_BY_BLUE)
            )
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_matrix(output_mpp, input_mp->num_rows, 3));
    output_mp = *output_mpp;

    if (projection_method == DIVIDE_BY_SUM)
    {
        first_index = RED_INDEX;
        second_index = GREEN_INDEX;
        third_index  = BLUE_INDEX;
    }
    else if (projection_method == DIVIDE_BY_RED)
    {
        first_index = GREEN_INDEX;
        second_index = BLUE_INDEX;
        third_index  = RED_INDEX;
    }
    else if (projection_method == DIVIDE_BY_GREEN)
    {
        first_index = RED_INDEX;
        second_index = BLUE_INDEX;
        third_index  = GREEN_INDEX;
    }
    else if (projection_method == DIVIDE_BY_BLUE)
    {
        first_index = RED_INDEX;
        second_index = GREEN_INDEX;
        third_index  = BLUE_INDEX;
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    for (i=0; i<input_mp->num_rows; i++)
    {
        (output_mp->elements)[i][ first_index ] = (input_mp->elements)[i][0];
        (output_mp->elements)[i][ second_index ] = (input_mp->elements)[i][1];

        if (projection_method == DIVIDE_BY_SUM)
        {
            (output_mp->elements)[i][ third_index ] = 1.0
                                                - (input_mp->elements)[i][ 0 ]
                                                - (input_mp->elements)[i][ 1 ];
        }
        else
        {
            (output_mp->elements)[i][ third_index ] = 1.0;
        }
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                          back_project_vector
 *
 * Converts chromaticity to RGB
 *
 * This routine converts the chromaticities in the vector "*input_mp" to RGB
 * using projection method designated by "projection_method". Not all projection
 * methods are valid, and sending this routine and invalid one is treated as a
 * bug. The value ones currently are DIVIDE_BY_RED, DIVIDE_BY_GREEN,
 * DIVIDE_BY_BLUE, and DIVIDE_BY_SUM.
 *
 * Index: chromaticity, colour, projection
 *
 * -----------------------------------------------------------------------------
*/

int back_project_vector
(
    Vector**          output_vpp,
    const Vector*     input_vp,
    Projection_method projection_method
)
{
    Vector* output_vp;
    int     first_index;
    int     second_index;
    int     third_index;


    if (    (input_vp->length != 2)
         ||
            (    (projection_method != DIVIDE_BY_SUM)
              && (projection_method != DIVIDE_BY_RED)
              && (projection_method != DIVIDE_BY_GREEN)
              && (projection_method != DIVIDE_BY_BLUE)
            )
       )
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    ERE(get_target_vector(output_vpp, 3));
    output_vp = *output_vpp;

    if (projection_method == DIVIDE_BY_SUM)
    {
        first_index = RED_INDEX;
        second_index = GREEN_INDEX;
        third_index  = BLUE_INDEX;
    }
    else if (projection_method == DIVIDE_BY_RED)
    {
        first_index = GREEN_INDEX;
        second_index = BLUE_INDEX;
        third_index  = RED_INDEX;
    }
    else if (projection_method == DIVIDE_BY_GREEN)
    {
        first_index = RED_INDEX;
        second_index = BLUE_INDEX;
        third_index  = GREEN_INDEX;
    }
    else if (projection_method == DIVIDE_BY_BLUE)
    {
        first_index = RED_INDEX;
        second_index = GREEN_INDEX;
        third_index  = BLUE_INDEX;
    }
    else
    {
        SET_CANT_HAPPEN_BUG();
        return ERROR;
    }

    (output_vp->elements)[ first_index ] = (input_vp->elements)[ 0 ];
    (output_vp->elements)[ second_index ] = (input_vp->elements)[ 1 ];

    if (projection_method == DIVIDE_BY_SUM)
    {
        (output_vp->elements)[ third_index ] =
               1.0 - (input_vp->elements)[ 0 ] - (input_vp->elements)[ 1 ];
    }
    else
    {
        (output_vp->elements)[ third_index ] = 1.0;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static double combine_error_measures
(
    double base_value,
    double absolute_error,
    double relative_error
)
{
    double error;


    if (absolute_error < 0.0)
    {
        if (relative_error > 0.0)
        {
            error = base_value * relative_error;
        }
        else
        {
            error = 0.0;
        }
    }
    else
    {
        if (relative_error > 0.0)
        {
            error = MAX_OF(absolute_error, base_value * relative_error);
        }
        else
        {
            error = absolute_error;
        }
    }

    return error;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

