
/* $Id: nr_roots.c 4725 2009-11-16 19:50:08Z kobus $ */


#include "nr/nr_gen.h"      /*  Only safe if first #include in a ".c" file  */

#include "x/x_complex.h"
#include "nr/nr_roots.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int zroots(KJB_complex*, int, KJB_complex*, int);
static int laguer(KJB_complex*, int, KJB_complex*, double, int);

/* -------------------------------------------------------------------------- */

int nr_real_roots_of_real_polynomial
(
    int     degree,
    double* polynomial,
    double* roots
)
{
    KJB_complex* complex_polynomial;
    KJB_complex* complex_roots;
    int      i;
    int      root_count = 0;
    int      save_degree = degree;
    double   sum = 0.0;


    UNTESTED_CODE(); /* Since mucking with complex to get rid of NR. */

    if (degree < 1)
    {
        set_bug("Invalid degree in real_roots.");
        return ERROR;
    }

    for (i=0; i<=degree; i++)
    {
        sum += ABS_OF(polynomial[ i ]);
    }

    if (sum < 1e10 * DBL_MIN)
    {
#ifdef DEF_OUT_FOR_NOW
        dbe(sum);
        for (i=0; i<=save_degree; i++)
        {
            dbe(polynomial[ i ]);
        }
#endif

        set_error("Polynomial is essentially zero in real_roots.");
        return ERROR;
    }

    NRE(complex_polynomial = N_TYPE_MALLOC(KJB_complex, degree + 1));
    NRE(complex_roots = N_TYPE_MALLOC(KJB_complex, degree + 1));

    for (i=0; i<=degree; i++)
    {
        /*
         * Push into standard floating point range.
         */
        complex_polynomial[i].r = polynomial[i] / sum;
        complex_polynomial[i].i = 0.0;
    }

    i = degree;

    while (i >= 0)
    {
        if (ABS_OF(complex_polynomial[i].r) > 100.0 * DBL_EPSILON)
        {
            break;
        }

        i--;
    }

    degree = i;

    if (degree < 1)
    {
#ifdef TEST
        for (i=0; i<=save_degree; i++)
        {
            dbe(polynomial[ i ]);
            dbe(complex_polynomial[i].r);
        }

        dbe(sum);
#endif

        set_error("Polynomial is essentially zero in real_roots.");
        return ERROR;
    }

    if (degree != save_degree)
    {
        verbose_pso(5, "Dropping degree from %d to %d in real_roots.\n",
                    save_degree, degree);
#ifdef TEST
        if (kjb_get_verbose_level() >= 5)
        {
            dbe(polynomial[ save_degree ]);
            dbe(complex_polynomial[ save_degree ].r);
        }
#endif
    }

    if (zroots(complex_polynomial, degree, complex_roots, TRUE) == ERROR)
    {
        kjb_free(complex_polynomial);
        kjb_free(complex_roots);
        return ERROR;
    }

    for (i=0; i<degree; i++)
    {
        if (ABS_OF(complex_roots[i + 1].i) < 1000.0 * DBL_EPSILON )
        {
            roots[root_count] = complex_roots[i + 1].r;
            {
                int j;
                double should_be_zero = 0.0;

                for (j = 0; j <= degree; j++)
                {
                    should_be_zero += polynomial[ j ]*pow(roots[root_count], (double)j);
                }

                if (ABS_OF(should_be_zero) > 1.0e-4)
                {
#ifdef TEST
                    dbe(roots[root_count]);
                    dbe(complex_roots[i + 1].i);
                    dbe(should_be_zero);

                    for (j = 0; j <= degree; j++)
                    {
                        dbe(polynomial[ j ]);
                    }
#else
                    static int first_time = TRUE;

                    if (first_time)
                    {
                        warn_pso("Unstable polynomial in nr_real_roots_of_real_polynomial.\n");
                        warn_pso("Additional instances will not be reported. \n");

                        first_time = FALSE;
                    }
#endif
                }
            }
            root_count++;
        }
    }

    kjb_free(complex_polynomial);
    kjb_free(complex_roots);

    return root_count;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*  --------------- End Kobus's Code ----------------- */

/*
// From Numerical Recipes in C (modified)
*/


static KJB_complex sqrt_complex(KJB_complex z)
{
    KJB_complex c;
    double   x, y, w, r;


    if ((z.r == 0.0) && (z.i == 0.0))
    {
        c.r = 0.0;
        c.i = 0.0;

        return c;
    }
    else
    {
        x = fabs(z.r);
        y = fabs(z.i);

        if (x >= y)
        {
            r = y/x;

            w = sqrt(x)*sqrt(0.5*(1.0 + sqrt(1.0 + r*r)));

        }
        else
        {
            r = x/y;

            w = sqrt(y)*sqrt(0.5*(r + sqrt(1.0 + r*r)));
        }

        if (z.r >= 0.0)
        {
            c.r = w;
            c.i = z.i / (2.0*w);
        }
        else
        {
            c.i = (z.i >= 0.0) ? w : -w;
            c.r = z.i / (2.0*c.i);
        }

        return c;
    }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */





#define EPS 2.0e-6
#define MAXM 1000

static int zroots(KJB_complex* a, int m, KJB_complex* roots, int polish)
{
    int     jj;
    int     j;
    int     i;
    KJB_complex x;
    KJB_complex b;
    KJB_complex c;
    KJB_complex ad[MAXM];

    for (j = 0;j<=m;j++) ad[j]=a[j];

    for (j=m;j>=1;j--)
    {
        x = make_complex(0.0, 0.0);

        if (laguer(ad, j, &x, EPS, 0) == ERROR)
        {
            if (laguer(ad, j, &x, EPS * 10.0, 0) == ERROR)
            {
                ERE(laguer(ad, j, &x, EPS * 100.0, 0));
            }
        }

        if (fabs(x.i) <= (2.0*EPS*fabs(x.r)))
        {
            x.i = 0.0;
        }

        roots[j] = x;
        b = ad[j];

        for (jj=j-1;jj>=0;jj--)
        {
            c = ad[jj];
            ad[jj] = b;
            b = add_complex(multiply_complex(x, b), c);
        }
    }

    if (polish)
    {
        for (j=1;j<=m;j++)
            ERE(laguer(a, m, &roots[j], EPS, 1));
    }

    for (j=2;j<=m;j++)
    {
        x = roots[j];

        for (i=j-1;i>=1;i--)
        {
            if (roots[i].r <= x.r) break;
            roots[i+1] = roots[i];
        }

        roots[i+1] = x;
    }

    return NO_ERROR;
}

#undef EPS
#undef MAXM

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#define EPSS 6.e-8
#define MAXIT 1000

static int laguer
(
    KJB_complex* a,
    int       m,
    KJB_complex* x,
    double    eps,
    int       polish
)
{
    int     j, iter;
    double  err, dxold, cdx, abx;
    KJB_complex sq, h, gp, gm, g2, g, b, d, dx, f, x1;

    dxold = magnitude_of_complex(*x);

    for (iter=1;iter<=MAXIT;iter++)
    {
        b = a[m];
        err = magnitude_of_complex(b);
        d = f=make_complex(0.0, 0.0);
        abx = magnitude_of_complex(*x);

        for (j=m-1;j>=0;j--)
        {
            f = add_complex(multiply_complex(*x, f), d);
            d = add_complex(multiply_complex(*x, d), b);
            b = add_complex(multiply_complex(*x, b), a[j]);
            err = magnitude_of_complex(b)+abx*err;
        }

        err *= EPSS;

        if (magnitude_of_complex(b) <= err)
        {
            return NO_ERROR;
        }

        g = divide_complex(d, b);
        g2 = multiply_complex(g, g);
        h = subtract_complex(g2, multiply_complex_by_real(divide_complex(f, b), 2.0));

        sq = sqrt_complex(multiply_complex_by_real(
                              subtract_complex(multiply_complex_by_real(h, (double)m),
                                               g2),
                              (double)(m-1)));

        gp = add_complex(g, sq);
        gm = subtract_complex(g, sq);

        if (magnitude_of_complex(gp) < magnitude_of_complex(gm))
        {
            gp = gm;
        }

        dx = divide_complex(make_complex((double)m, 0.0), gp);
        x1 = subtract_complex(*x, dx);

        if (x->r == x1.r && x->i == x1.i)
        {
            return NO_ERROR;
        }

        *x = x1;
        cdx = magnitude_of_complex(dx);

        if (iter > 6 && cdx >= dxold)
        {
            return NO_ERROR;
        }

        dxold = cdx;

        if (!polish)
        {
            if (cdx <= eps*magnitude_of_complex(*x))
            {
                return NO_ERROR;
            }
        }
     }

    set_error(
       "Too many iterations in \"Numerical Recipes in C\" routine \"laguer\".");
    return ERROR;
}

#undef EPSS
#undef MAXIT

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

