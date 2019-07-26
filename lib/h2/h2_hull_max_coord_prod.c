
/* $Id: h2_hull_max_coord_prod.c 21545 2017-07-23 21:57:31Z kobus $ */

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

#include "h2/h2_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "h2/h2_hull_max_coord_prod.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int fs_hull_max_coord_product_resolution      = 100;

/* -------------------------------------------------------------------------- */

static int find_2D_hull_max_coord_product
(
    const Hull* hp,
    Vector**    max_point_vpp,
    double*     max_product_ptr
);

static int find_3D_hull_max_coord_product
(
    const Hull* hp,
    Vector**    max_point_vpp,
    double*     max_product_ptr
);

static int estimate_3D_hull_max_coord_product
(
    const Hull* hp,
    int         resolution,
    Vector**    max_point_vpp,
    double*     max_product_ptr
);

/* -------------------------------------------------------------------------- */

int set_hull_max_coord_prod_options
(
    const char* option,
    const char* value
)
{
    char lc_option[ 100 ];
    int  result = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option, "hull-mcp-resolution")
          || match_pattern(lc_option, "hull-max-coord-product-resolution")
       )
    {
        int temp_int_value;

        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("Hull average resolution is %d.\n",
                    fs_hull_max_coord_product_resolution));
        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("hull-average-resolution = %d\n",
                    fs_hull_max_coord_product_resolution));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_hull_max_coord_product_resolution = temp_int_value;
        }
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int find_hull_max_coord_product
(
    const Hull* hp,
    Vector**    point_vpp,
    double*     product_ptr
)
{
    int dim;


    if (hp == NULL)
    {
        set_bug("Null hull pointer passed to find_hull_max_coord_product.");
        return ERROR;
    }

    dim = hp->dimension;

    if ((point_vpp != NULL) && (*point_vpp != NULL))
    {
        if ((*point_vpp)->length != dim)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if (dim == 2)
    {
        return find_2D_hull_max_coord_product(hp, point_vpp, product_ptr);
    }
    else if (dim == 3)
    {
        return find_3D_hull_max_coord_product(hp, point_vpp, product_ptr);
    }
    else
    {
        set_bug("Finding max coord product of %d-D hulls not supported.", dim);
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_2D_hull_max_coord_product
(
    const Hull* hp,
    Vector**    max_point_vpp,
    double*     max_product_ptr
)
{
    Matrix *facet_mp;
    int i;
    double prod, max_prod;
    double m, x1, x2, y1, y2;
    double x, y, max_x, max_y;


    x1 = ((hp->facets->elements)[0]->elements)[0][0];
    y1 = ((hp->facets->elements)[0]->elements)[0][1];

    max_prod = x1*y1;
    max_x = x1;
    max_y = y1;

    for (i=0; i<hp->num_facets; i++)
    {
        facet_mp = (hp->facets->elements)[i];

        x1 = (facet_mp->elements)[0][0];
        x2 = (facet_mp->elements)[1][0];
        y1 = (facet_mp->elements)[0][1];
        y2 = (facet_mp->elements)[1][1];


        /*
            If segment is vertical or horizontal, then max is at an endpoint.
        */
        if ((! IS_EQUAL_DBL(x1, x2)) &&
            (! IS_EQUAL_DBL(y1, y2)))
        {
            m = (y2 - y1)/ (x2 - x1);
            x = (m*x1 - y1) / (2.0 * m);

            if (((x1 <= x2) && (x1 <= x) && (x <= x2)) ||
                ((x1 >= x2) && (x1 >= x) && (x >= x2)))
            {
                y = y1 + m*(x - x1);

                prod = x*y;

                if (prod > max_prod)
                {
                    max_x = x;
                    max_y = y;
                    max_prod = prod;
                }
            }
        }

         prod = x1*y1;

         if (prod > max_prod)
         {
             max_x = x1;
             max_y = y1;
             max_prod = prod;
         }

         prod = x2*y2;

         if (prod > max_prod)
         {
             max_x = x2;
             max_y = y2;
             max_prod = prod;
         }
     }

    if (max_point_vpp != NULL)
    {
        ERE(get_target_vector(max_point_vpp, 2));

        ((*max_point_vpp)->elements)[ 0 ] = max_x;
        ((*max_point_vpp)->elements)[ 1 ] = max_y;
    }

    if (max_product_ptr != NULL)
    {
        *max_product_ptr = max_prod;
    }

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int find_3D_hull_max_coord_product
(
    const Hull* hp,
    Vector**    max_point_vpp,
    double*     max_product_ptr
)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Matrix** facet_mp_list;
    Matrix*  facet_mp;
    Matrix*  normal_mp;
    Vector*  test_vp;
    int      i, j, k;
    int      inside_hull;
    double     prod;
    double     max_prod;
    double     K, nx, ny, nz, cx, cy, cz, x, y, z;
    double     max_x;
    double     max_y;
    double     max_z;
    double     A, B, C, D, x1, x2, y1, y2, z1, z2;
    double     roots[ 2 ];
    double     coef[ 3 ];
    int      num_roots;
    int      result        = NO_ERROR;


    x = ((hp->facets->elements)[0]->elements)[0][0];
    y = ((hp->facets->elements)[0]->elements)[0][1];
    z = ((hp->facets->elements)[0]->elements)[0][2];

    max_prod = x*y*z;
    max_x = x;
    max_y = y;
    max_z = z;

    facet_mp_list = hp->facets->elements;

    normal_mp = hp->normal_mp;

    NRE(test_vp = create_vector( 3 ));

    for (i=0; i<hp->num_facets; i++)
    {
        if (io_atn_flag)
        {
            set_error("Processing interrupted.");
            halt_all_output = FALSE;
            result = ERROR;
            break;
        }

        facet_mp = (hp->facets->elements)[i];

        nx = (normal_mp->elements)[i][0];
        ny = (normal_mp->elements)[i][1];
        nz = (normal_mp->elements)[i][2];

        cx = ((facet_mp_list[i])->elements)[0][0];
        cy = ((facet_mp_list[i])->elements)[0][1];
        cz = ((facet_mp_list[i])->elements)[0][2];

#ifdef HOW_IT_WAS
        if ((! IS_ZERO_DBL(nx)) && (! IS_ZERO_DBL(ny)) &&(! IS_ZERO_DBL(nz)))
#else
        if (    ((nx > HULL_EPSILON) || (nx < -HULL_EPSILON))
             && ((ny > HULL_EPSILON) || (ny < -HULL_EPSILON))
             && ((nz > HULL_EPSILON) || (nz < -HULL_EPSILON))
           )
#endif
        {
            K = nx*cx + ny*cy + nz*cz;

            x = K/( 3.0 * nx );
            y = K/( 3.0 * ny );
            z = K/( 3.0 * nz );

            (test_vp->elements)[ 0 ] = x;
            (test_vp->elements)[ 1 ] = y;
            (test_vp->elements)[ 2 ] = z;

            ERE(inside_hull = is_point_inside_hull(hp, test_vp));

            if (inside_hull)
            {
                prod = x*y*z;

                if (prod > max_prod)
                {
                    max_x = x;
                    max_y = y;
                    max_z = z;
                    max_prod = prod;
                }
            }
        }

        for (j=0; j<facet_mp->num_rows; j++)
        {
            if (io_atn_flag)
            {
                set_error("Processing interrupted.");
                halt_all_output = FALSE;
                result = ERROR;
                break;
            }

            /* Test segment middle */

            x1 = (facet_mp->elements)[j][0];
            y1 = (facet_mp->elements)[j][1];
            z1 = (facet_mp->elements)[j][2];

            if (j < (facet_mp->num_rows - 1))
            {
                x2 = (facet_mp->elements)[j+1][0];
                y2 = (facet_mp->elements)[j+1][1];
                z2 = (facet_mp->elements)[j+1][2];
            }
            else
            {
                x2 = (facet_mp->elements)[ 0 ][0];
                y2 = (facet_mp->elements)[ 0 ][1];
                z2 = (facet_mp->elements)[ 0 ][2];
            }

            /* Test one endpoint. Since we do all segments, the other endpoint
            // is either done, or will be done later.
            */

            x = x1;
            y = y1;
            z = z1;

            prod = x*y*z;

            if (prod > max_prod)
            {
                max_x = x;
                max_y = y;
                max_z = z;
                max_prod = prod;
            }

            /*
            // If segment is vertical or horizontal, then max is at an endpoint.
             */
            if ((! IS_EQUAL_DBL(x1, x2)) &&
                (! IS_EQUAL_DBL(y1, y2)) &&
                (! IS_EQUAL_DBL(z1, z2)))
            {
                A = (y2-y1)/(x2-x1);
                B = A*x2 - y2;
                C = (z2-z1)/(x2-x1);
                D = C*x2 - z2;

                coef[ 0 ] = B*D;
                coef[ 1 ] = -2.0 * (A*D + B*C);
                coef[ 2 ] = 3.0 * A * C;

                ERE(num_roots = real_roots_of_real_polynomial(2, coef, roots));

                for (k=0; k<num_roots; k++)
                {
                    x = roots[ k ];

                    if (((x1 <= x2) && (x1 <= x) && (x <= x2)) ||
                        ((x1 >= x2) && (x1 >= x) && (x >= x2)))
                    {
                        y = A*x - B;
                        z = C*x - D;

                        prod = x*y*z;

                        if (prod > max_prod)
                        {
                            max_x = x;
                            max_y = y;
                            max_z = z;
                            max_prod = prod;
                        }
                    }
                }
            }

        }

        if (result == ERROR) break;
    }

    free_vector(test_vp);

    if (result != ERROR)
    {
        if (max_point_vpp != NULL)
        {
            ERE(get_target_vector(max_point_vpp, 3));

            ((*max_point_vpp)->elements)[ 0 ] = max_x;
            ((*max_point_vpp)->elements)[ 1 ] = max_y;
            ((*max_point_vpp)->elements)[ 2 ] = max_z;
        }

        if (max_product_ptr != NULL)
        {
            *max_product_ptr = max_prod;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int estimate_hull_max_coord_product
(
    const Hull* hp,
    Vector**    point_vpp,
    double*     product_ptr
)
{
    int dim;


    if (hp == NULL)
    {
        set_bug("Null hull pointer passed to find_hull_max_coord_product.");
        return ERROR;
    }

    dim = hp->dimension;

    if ((point_vpp != NULL) && (*point_vpp != NULL))
    {
        if ((*point_vpp)->length != dim)
        {
            SET_ARGUMENT_BUG();
            return ERROR;
        }
    }

    if (dim == 2)
    {
        /*
        // For now, in the 2d case, just use the slow, exact, routine.
        */
        return find_2D_hull_max_coord_product(hp, point_vpp, product_ptr);
    }
    else if (dim == 3)
    {
        return estimate_3D_hull_max_coord_product(hp,
                                        fs_hull_max_coord_product_resolution,
                                                  point_vpp,
                                                  product_ptr);
    }
    else
    {
        set_bug("Intersection of %d-D hulls not supported.", dim);
        return ERROR;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 * STATIC                 estimate_3D_hull_max_coord_product
 * -----------------------------------------------------------------------------
*/
static int estimate_3D_hull_max_coord_product
(
    const Hull* hp,
    int         resolution,
    Vector**    max_point_vpp,
    double*     max_product_ptr
)
{
    IMPORT volatile Bool io_atn_flag;
    IMPORT volatile Bool halt_all_output;
    Matrix* normal_mp;
    Vector* constraint_vp;
    double**  normal_row_ptr;
    double*   normal_elem_ptr;
    double*   constraint_elem_ptr;
    int     i, j, k, m;
    int     num_normals;
    double    min_x;
    double    max_x;
    double    min_y;
    double    max_y;
    double    min_z;
    double    max_z;
    double    x, y;
    double    x_step;
    double    y_step;
    double    z;
    double    hull_min_z;
    double    hull_max_z;
    double    temp;
    Vector* x_cache_vp;
    double*   x_cache_ptr;
    int     result     = NO_ERROR;
    double    max_prod_x = DBL_NOT_SET;
    double    max_prod_y = DBL_NOT_SET;
    double    max_prod_z = DBL_NOT_SET;
    double    max_prod    = 0.0;


    min_x = hp->min_x;
    max_x = hp->max_x;

    min_y = hp->min_y;
    max_y = hp->max_y;

    min_z = hp->min_z;
    max_z = hp->max_z;

    x_step = (max_x - min_x) / resolution;
    y_step = (max_y - min_y) / resolution;

    constraint_vp = hp->b_value_vp;
    normal_mp = hp->normal_mp;
    num_normals = normal_mp->num_rows;

    x = min_x;

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
                    if (IS_NEGATIVE_DBL(temp))
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

#ifdef HOW_IT_WAS
                        if (IS_GREATER_DBL(hull_min_z, hull_max_z))
#else
                        if (IS_NOT_LESSER_DBL(hull_min_z, hull_max_z))
#endif
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

#ifdef HOW_IT_WAS
                        if (IS_GREATER_DBL(hull_min_z, hull_max_z))
#else
                        if (IS_NOT_LESSER_DBL(hull_min_z, hull_max_z))
#endif
                        {
                            break;  /* No solution */
                        }
                    }
                }

                constraint_elem_ptr++;
                normal_row_ptr++;
                x_cache_ptr++;
            }

            /*
            // if (hull_min_z < hull_max_z)
            */
            if (IS_NOT_GREATER_DBL(hull_min_z, hull_max_z))
            {
                double prod = x * y * hull_max_z;

                if (prod > max_prod)
                {
                    max_prod = prod;
                    max_prod_x = x;
                    max_prod_y = y;
                    max_prod_z = hull_max_z;
                }
            }
             y += y_step;
         }
         x += x_step;
    }

    free_vector(x_cache_vp);

    if (result != ERROR)
    {
        if (    (IS_EQUAL_DBL(max_prod_x, DBL_NOT_SET))
             || (IS_EQUAL_DBL(max_prod_y, DBL_NOT_SET))
             || (IS_EQUAL_DBL(max_prod_z, DBL_NOT_SET))
           )
        {
            int index;

            warn_pso("Unable to find a point in hull for max prod estimate.\n");
            warn_pso("Resoution is %d.\n", resolution);
            warn_pso("Using max prod estimate of vertices instead.\n");

            ERE(index = get_max_matrix_row_product(hp->vertex_mp,
                                                   &max_prod));

            if (max_point_vpp != NULL)
            {
                ERE(get_matrix_row(max_point_vpp, hp->vertex_mp, index));
            }

            if (max_product_ptr != NULL)
            {
                *max_product_ptr = max_prod;
            }

            return NO_ERROR;
        }
    }

    if (result != ERROR)
    {
        if (max_point_vpp != NULL)
        {
            ERE(get_target_vector(max_point_vpp, 3));

            ((*max_point_vpp)->elements)[ 0 ] = max_prod_x;
            ((*max_point_vpp)->elements)[ 1 ] = max_prod_y;
            ((*max_point_vpp)->elements)[ 2 ] = max_prod_z;
        }

        if (max_product_ptr != NULL)
        {
            *max_product_ptr = max_prod;
        }
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

