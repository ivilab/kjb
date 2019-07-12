
/* $Id: n_roots.c 4727 2009-11-16 20:53:54Z kobus $ */

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

#include "n/n_gen.h"      /*  Only safe if first #include in a ".c" file  */

#include "n/n_roots.h"

#ifdef KJB_HAVE_NUMERICAL_RECIPES
#include "nr/nr_roots.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

int real_roots_of_real_polynomial
(
    int     degree,
    double* polynomial,
    double* roots
)
{


    if (degree < 1)
    {
        set_bug("Invalid degree in real_roots.");
        return ERROR;
    }

    if (IS_ZERO_DBL(polynomial[ degree ]))
    {
        /* Does not look like degree 1 to me. */
        set_bug("Leading coefficient in polynomial is zero in root finding routine.");
        return ERROR;
    }

    /*
    // We have removed the Numerical Recipes code for finding roots in order
    // that the code can be released to the world without copyright worries.
    // Fortunately, for at least the current version of the "gamut" program we
    // want to release, this routine was only used to solve quadratic
    // equations, so I have put in the code to handle this special case.
    */
    if (degree == 1)
    {
        roots[ 0 ] = -polynomial[ 1 ] / polynomial[ 0 ];
        return 1;
    }
    else if (degree == 2)
    {
        double a = polynomial[ 2 ];
        double b = polynomial[ 1 ];
        double c = polynomial[ 0 ];
        double d = b*b - 4.0*a*c;
        double root_d;
#ifdef KJB_HAVE_NUMERICAL_RECIPES
#ifdef TEST
        double num_roots;

        num_roots = nr_real_roots_of_real_polynomial(degree, polynomial, roots);
#endif
#endif

        if (d < 0.0)
        {
#ifdef KJB_HAVE_NUMERICAL_RECIPES
#ifdef TEST
            ASSERT(num_roots == 0);
#endif
#endif
            return 0;   /* Only doing real roots. */
        }

        root_d = sqrt(d);

#ifdef KJB_HAVE_NUMERICAL_RECIPES
#ifdef TEST
        ASSERT(num_roots == 2);
        if (ABS_OF(roots[ 0 ] - (-b + root_d) / (2.0 * a)) > ABS_OF(roots[ 0 ] - (-b - root_d) / (2.0 * a)))
        {
            ASSERT(ABS_OF(roots[ 0 ] - (-b - root_d) / (2.0 * a)) < 1e6*DBL_EPSILON);
            ASSERT(ABS_OF(roots[ 1 ] - (-b + root_d) / (2.0 * a)) < 1e6*DBL_EPSILON);
        }
        else
        {
            ASSERT(ABS_OF(roots[ 0 ] - (-b + root_d) / (2.0 * a)) < 1e6*DBL_EPSILON);
            ASSERT(ABS_OF(roots[ 1 ] - (-b - root_d) / (2.0 * a)) < 1e6*DBL_EPSILON);
        }
#endif
#endif
        roots[ 0 ] = (-b + root_d) / (2.0 * a);
        roots[ 1 ] = (-b - root_d) / (2.0 * a);

        return 2;
    }
    else
    {
#ifdef KJB_HAVE_NUMERICAL_RECIPES
        return nr_real_roots_of_real_polynomial(degree, polynomial, roots);
#else
        set_error("No routine for real roots of real polynomial is availble.");
        return ERROR;
#endif
    }

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

