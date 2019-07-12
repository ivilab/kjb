
/* $Id: x_complex.c 4727 2009-11-16 20:53:54Z kobus $ */

/* =========================================================================== *
|
|  Copyright (c) 2003-2008 by Kobus Barnard (author).
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


#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "x/x_complex.h"


#ifdef __cplusplus
extern "C" {
#endif
  

/* -------------------------------------------------------------------------- */

KJB_complex make_complex(double r, double i)
{
    KJB_complex z;


    UNTESTED_CODE(); 

    z.r = r;
    z.i = i;

    return z;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

KJB_complex conjugate_of_complex(KJB_complex z)
{
    KJB_complex z_bar;

    UNTESTED_CODE(); 

    z_bar.r = z.r;
    z_bar.i = -z.i;

    return z_bar;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double magnitude_of_complex(KJB_complex z)
{
    UNTESTED_CODE(); 

    return magnitude_of_complex_2(z.r, z.i); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double magnitude_of_complex_2(double r, double i)
{
    double abs_r = ABS_OF(r);
    double abs_y = ABS_OF(i);

    /*
     * Alternative complex interface; real and imaginary are tracked by the
     * calling program. 
     *
    */

    /*
     * Order of operations adopted from numerical recipies (5.4.4). As far as I
     * can tell, using their recomendation is not the same as distributing their
     * code (which we do not want to do, as we want to keep our options open
     * regarding distributing source. 
    */

    UNTESTED_CODE(); 

    if (abs_r == 0.0)
    {
        return abs_y; 
    }
    else if (abs_y == 0.0)
    {
        return abs_r;
    }
    else if (abs_r > abs_y)
    {
        double temp = abs_y / abs_r;

        return  abs_r * sqrt(1.0 + temp*temp);
    }
    else
    {
        double temp = abs_r / abs_y;

        return abs_y * sqrt(1.0 + temp*temp);
    }

}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double angle_of_complex(KJB_complex z)
{
    UNTESTED_CODE(); 

    return angle_of_complex_2(z.r, z.i);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double angle_of_complex_2(double re, double im)
{
    /*
     * Alternative complex interface; real and imaginary are tracked by the
     * calling program. 
     *
    */

    UNTESTED_CODE(); 

#ifdef HAVE_ATAN_BUT_NOT_ATAN2
    if (re == 0.0) 
    {
        return ((im == 0.0) ? 0.0 : ((im > 0.0) ? M_PI / 2.0 : -M_PI / 2.0)); 
    }
    else if (re > 0.0)
    {
        return atan(im / re);
    }
    else if (im > 0.0) 
    {
        return atan(im / re) + M_PI;
    }
    else 
    {
        return atan(im / re) - M_PI;
    }
#endif 
    return atan2(im, re); 
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

KJB_complex add_complex(KJB_complex a, KJB_complex b)
{
    KJB_complex c;


    UNTESTED_CODE(); 

    c.r = a.r + b.r;
    c.i = a.i + b.i;

    return c;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

KJB_complex subtract_complex(KJB_complex a, KJB_complex b)
{
    KJB_complex c;


    UNTESTED_CODE(); 

    c.r = a.r - b.r;
    c.i = a.i - b.i;

    return c;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

KJB_complex multiply_complex(KJB_complex a, KJB_complex b)
{
    KJB_complex c;


    UNTESTED_CODE(); 

    c.r = a.r*b.r - a.i*b.i;
    c.i = a.i*b.r + a.r*b.i;

    return c;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

KJB_complex divide_complex(KJB_complex a, KJB_complex b)
{
    KJB_complex c;
    double      denominator;
    double      temp;

    /*
     * Order of operations adopted from numerical recipies (5.4.4). As far as I
     * can tell, using their recomendation is not the same as distributing their
     * code (which we do not want to do, as we want to keep our options open
     * regarding distributing source. 
    */

    UNTESTED_CODE(); 

    if (ABS_OF(b.r) >= ABS_OF(b.i))
    {
        temp = b.i / b.r;
        denominator = b.r + temp * b.i;
        c.r = (a.r + temp*a.i) / denominator;
        c.i = (a.i - temp*a.r) / denominator;
    }
    else
    {
        temp = b.r / b.i;
        denominator = b.i + temp * b.r;
        c.r = (temp * a.r + a.i) / denominator;
        c.i = (temp * a.i - a.r) / denominator;
    }

    return c;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

KJB_complex multiply_complex_by_real(KJB_complex a, double s)
{
    KJB_complex z;


    UNTESTED_CODE(); 

    z.r = s * a.r;
    z.i = s * a.i;

    return z;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

