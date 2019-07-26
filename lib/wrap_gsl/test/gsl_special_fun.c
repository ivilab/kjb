/* $Id: gsl_special_fun.c 21491 2017-07-20 13:19:02Z kobus $ */

#include "l/l_sys_std.h"
#include "l/l_sys_io.h"
#include "l/l_sys_debug.h"
#include "l/l_init.h"
#include "l/l_error.h"
#include "l/l_verbose.h"
#include "l/l_global.h"

#include "wrap_gsl/wrap_gsl_sf.h"

/*
 * approx equality -- but it is troublesome near zero.  Better to
 * give a slighly nonzero reference value, small and much closer
 * than the tolerance you would accept as close enough to zero.
 */
static int pretty_close(double y1, double y2, double err)
{
    double dif = fabs(y1-y2), absum = fabs(y1)+fabs(y2);
    err = fabs(err);

    if ( dif <= err*absum )
    {
        verbose_pso(2, "Input test, expected values: %e, %e\n"
                "Error tolerance: %e \tDifference: %e "
                "less than product %e:  success.\n",
                y1, y2, err, dif, err*absum);
        
        return NO_ERROR;
    }
    add_error("Bad approximation:  "
        "received %e, expected %e, tolerance %e", y1, y2, err);
    return ERROR;
}


static int test_erf(void)
{
    double y;

    /* erf(0) = 0 */
    ERE(kjb_erf(&y, 0));
    ERE(pretty_close(y, 1e-10, 2)); /* looks weird, is ok:  see note */

    /* erf(.5) = 0.5205 approx  */
    ERE(kjb_erf(&y, 0.5));
    ERE(pretty_close(y, 0.5205, 1e-4));
    /* odd function */
    ERE(kjb_erf(&y, -0.5));
    ERE(pretty_close(y, -0.5205, 1e-4));

    /* erf(1) = 0.8427 approx  */
    ERE(kjb_erf(&y, 1.0));
    ERE(pretty_close(y, 0.8427, 1e-4));
    /* odd function */
    ERE(kjb_erf(&y, -1.0));
    ERE(pretty_close(y, -0.8427, 1e-4));

    return NO_ERROR;
}

static int test_bessel_i0(void)
{
    double y;

    /* I0(0) = 1 */
    ERE(kjb_bessel_I0(&y, 0));
    ERE(pretty_close(y, 1, 1e-3));

    /* I0(1) = 1.266 approx */
    ERE(kjb_bessel_I0(&y, 1));
    ERE(pretty_close(y, 1.266, 1e-3));

    /* I0(2) = 2.280 approx */
    ERE(kjb_bessel_I0(&y, 2));
    ERE(pretty_close(y, 2.28, 1e-3));

    /* I0(3) = 4.881 approx */
    ERE(kjb_bessel_I0(&y, 3));
    ERE(pretty_close(y, 4.881, 1e-3));

    return NO_ERROR;
}


/* Using tabulated values from Spiegel, Murray L., _Mathematical Handbook_,
   McGraw-Hill (New York: 1968) below.
 */
static int test_scaled_bessel_i0()
{
    double y;

    /* Do you have an exp() function?  Does it work? */
    ERE(pretty_close(1, exp(0), 1e-3));
    ERE(pretty_close(.36788, exp(-1), 1e-5));

    /* I0(0) * exp(-0) = 1 */
    ERE(kjb_scaled_bessel_I0(&y, 0));
    ERE(pretty_close(y, 1*exp(-0), 1e-3));

    /* I0(3) * exp(-3) = 4.881 * exp(-3) approx */
    ERE(kjb_scaled_bessel_I0(&y, 3));
    ERE(pretty_close(y, 4.881*exp(-3), 1e-3));

    /* I0(9) * exp(-9) = 1091 * exp(-9) approx = 0.1346 approx */
    ERE(kjb_scaled_bessel_I0(&y, 9));
    ERE(pretty_close(y, 1091*exp(-9), 3e-3));
    return NO_ERROR;
}

static int test_scaled_bessel_i1()
{
    double y;

    /* Do you have an exp() function?  Does it work? */
    ERE(pretty_close(1, exp(0), 1e-3));
    ERE(pretty_close(.36788, exp(-1), 1e-5));

    /* I1(1) * exp(-1) = 0.5652 * exp(-1) approx */
    ERE(kjb_scaled_bessel_I1(&y, 1));
    ERE(pretty_close(y, 0.5652*exp(-1), 1e-3));

    /* I1(4) * exp(-4) = 9.759 * exp(-4) approx */
    ERE(kjb_scaled_bessel_I1(&y, 4));
    ERE(pretty_close(y, 9.759*exp(-4), 1e-3));

    /* I1(9) * exp(-9) = 1031 * exp(-9) approx = 0.1346 approx */
    ERE(kjb_scaled_bessel_I1(&y, 9));
    ERE(pretty_close(y, 1031*exp(-9), 3e-3));
    return NO_ERROR;
}

int main(void)
{
    EPETE(kjb_init());

    EPETB(test_erf());
    EPETB(test_bessel_i0());
    EPETB(test_scaled_bessel_i0());
    EPETB(test_scaled_bessel_i1());

    kjb_cleanup();

    return EXIT_SUCCESS;
}
