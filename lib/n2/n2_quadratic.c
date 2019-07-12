
/* $Id: n2_quadratic.c 4727 2009-11-16 20:53:54Z kobus $ */

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

#include "n2/n2_gen.h"        /*  Only safe if first #include in a ".c" file  */
#include "wrap_slatec/wrap_slatec.h"
#include "n2/n2_quadratic.h"

/* For declared function pointers. Generally harmless. */
#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static Method_option fs_constrained_least_squares_methods[ ] =
{
      /* Current first choice. */
      { "dlsei",  "dlsei",  (int(*)())do_dlsei_quadratic }
    , {"dbocls", "dbocls",  (int(*)())do_dbocls_quadratic }
};

static const int fs_num_constrained_least_squares_methods =
                                sizeof(fs_constrained_least_squares_methods) /
                                          sizeof(fs_constrained_least_squares_methods[ 0 ]);
static int         fs_constrained_least_squares_method                  = 0;
static const char* fs_constrained_least_squares_method_option_short_str =
                                                       "constrained-least-squares";
static const char* fs_constrained_least_squares_method_option_long_str =
                                                      "constrained-least-squares-method";


/* -------------------------------------------------------------------------- */

int set_constrained_least_squares_options(const char* option, const char* value)
{
    char lc_option[ 100 ];
    int  result           = NOT_FOUND;


    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
          || match_pattern(lc_option,
                           fs_constrained_least_squares_method_option_short_str)
          || match_pattern(lc_option,
                           fs_constrained_least_squares_method_option_long_str)
       )
    {
        if (value == NULL) return NO_ERROR;

        ERE(parse_method_option(fs_constrained_least_squares_methods,
                                fs_num_constrained_least_squares_methods,
                                fs_constrained_least_squares_method_option_long_str,
                                "constrained least squares method", value,
                                &fs_constrained_least_squares_method));
        result = NO_ERROR;
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                       constrained_least_squares
 *
 * Solves Ax=b in the least squares sense with linear constraints.
 *
 * This routine solves Ax=b in the least squares sense subject to linear
 * constraints.
 *
 * Put differently, this routine calculates
 * |
 * |  MIN { || Ax - b ||  }
 * |   x                2
 * |
 * |  Subject to:
 * |
 * |     Lx <= m
 * |     Ex  = e
 * |      x <= u
 * |      x >= l
 *
 *
 * Note that it is generally easy to convert a basic sum of squared error
 * objective function into Ax=0, least squares sense.
 *
 * The result is put into the vector pointed to by *result_vpp, which is created
 * if it is NULL, resized if it is the wrong sized, and reused otherwise. The
 * number of rows in the matrix pointed to by the argument "A_mp" (i.e, A,
 * above) must be the same as the length of the vector pointed to by the
 * argument "b_vp" (i.e. b, above). In addition, the number of columns of of
 * *A_mp (i.e, number of unknowns) cannot exceed the number of rows of *A_mp
 * (number of equations).
 *
 * With respect to the above formulation, the parameter le_constraint_mp
 * corresponds to L, le_constraint_vp to m, eq_constraint_mp to E,
 * eq_constraint_vp to e, lb_row_arg_vp corresponds to l, and ub_row_arg_vp
 * corresponds to u. Any of these constraint arguments can be set to NULL if
 * there are no constraints of the corresponding type.
 *
 * Returns:
 *     NO_ERROR on success, and ERROR on failure, with an appropriate error
 *     message being set.
 *
 * Index: least squares, quadratic programming, optimization
 *
 * -----------------------------------------------------------------------------
*/

int constrained_least_squares
(
    Vector**      result_vpp,
    const Matrix* A_mp,
    const Vector* b_vp,
    const Matrix* le_constraint_mp,
    const Vector* le_constraint_vp,
    const Matrix* eq_constraint_mp,
    const Vector* eq_constraint_vp,
    const Vector* lb_vp,
    const Vector* ub_vp
)
{
    int result = NO_ERROR;

    if (    (fs_constrained_least_squares_method < 0)
         || (fs_constrained_least_squares_method >= fs_num_constrained_least_squares_methods)
       )
    {
        SET_CANT_HAPPEN_BUG();
        result = ERROR;
    }
    else
    {
        int (*constrained_least_squares_fn) (Vector**, const Matrix*, const Vector*, const Matrix*, const Vector*, const Matrix*, const Vector*, const Vector*, const Vector*)
            /* Kobus: 05/01/22: Be very pedantic here, to keep C++ happy.  */
            = (int (*) (Vector**, const Matrix*, const Vector*, const Matrix*, const Vector*, const Matrix*, const Vector*, const Vector*, const Vector*))
                fs_constrained_least_squares_methods[ fs_constrained_least_squares_method ].fn;


        result = constrained_least_squares_fn
        (
             result_vpp, A_mp, b_vp,
             le_constraint_mp, le_constraint_vp,
             eq_constraint_mp, eq_constraint_vp,
             lb_vp, ub_vp
        );
    }

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif





