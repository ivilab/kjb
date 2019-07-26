
/* $Id: m_spline.c 20654 2016-05-05 23:13:43Z kobus $ */

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

#include "m/m_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "m/m_spline.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef TRACK_MEMORY_ALLOCATION
    static void free_uniform_cubic_spline_storage(void);
    static void free_cubic_spline_storage(void);
    static void free_get_spline_parms_storage(void);
#endif

/* -------------------------------------------------------------------------- */

static int   fs_max_in_n              = NOT_SET;
static int   fs_max_out_n             = NOT_SET;
static int   fs_cubic_spline_max_size = NOT_SET;
static int   fs_gsp_max_size          = NOT_SET;
static double* fs_uniform_in_x          = NULL;
static double* fs_uniform_out_x         = NULL;
static double* fs_cubic_spline_b        = NULL;
static double* fs_cubic_spline_c        = NULL;
static double* fs_cubic_spline_d        = NULL;
static double* fs_gsp_h                 = NULL;
static double* fs_gsp_l                 = NULL;
static double* fs_gsp_u                 = NULL;
static double* fs_gsp_z                 = NULL;
static double* fs_gsp_alpha             = NULL;

/* -------------------------------------------------------------------------- */

Vector* cs_interpolate_vector
(
    Vector* in_vp,
    double  in_offset,
    double  in_step,
    Vector* out_vp,
    int     out_count,
    double  out_offset,
    double  out_step
)
{
    int    in_count;
    double*  out_vp_pos;
    double*  in_vp_pos;
    double in_max;
    double out_max;


    if (out_count < 1)
    {
        set_bug("Non-positive count passed to cs_interpolate_vector.");
        return NULL;
    }

    in_count = in_vp->length;

    in_max = in_offset + (in_count-1) * in_step;
    out_max = out_offset + (out_count-1) * out_step;

    if (IS_LESSER_DBL(out_offset, in_offset))
    {
        set_bug("Out_offset less then in_offset in cs_interpolate_vector.");
        return NULL;
    }
    else if (IS_GREATER_DBL(out_max, in_max))
    {
        set_bug("Not enough input data in cs_interpolate_vector.");
        return NULL;
    }

    if (out_vp == NULL)
    {
        NRN(out_vp = create_vector(out_count));
    }
    else
    {
        if (out_vp->length != out_count)
        {
            SET_ARGUMENT_BUG();
            return NULL;
        }
    }

    in_vp_pos = in_vp->elements;
    out_vp_pos = out_vp->elements;

    ERN(uniform_x_cubic_spline(in_count, in_offset, in_step,
                               in_vp_pos, out_count, out_offset, out_step,
                               out_vp_pos));

    return out_vp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

Vector* interpolate_vector
(
    Vector* in_vp,
    double  in_offset,
    double  in_step,
    Vector* out_vp,
    int     out_count,
    double  out_offset,
    double  out_step
)
{
    int    in_count;
    double*  out_vp_pos;
    double*  in_vp_pos;
    int    i;
    double in_max;
    double out_max;
    double   interpolate_val;
    double   right_fraction;
    double   left_fraction;
    int    left_val;


    if (out_count < 1)
    {
        set_bug("Non-positive count passed to interpolate_vector.");
        return NULL;
    }

    in_count = in_vp->length;

    in_max = in_offset + (in_count * (in_step - 1));
    out_max = out_offset + (out_count * (out_step - 1));

    if (IS_LESSER_DBL(out_offset, in_offset))
    {
        set_bug("Out_offset less then in_offset in interpolate_vector.");
        return NULL;
    }
    else if (IS_GREATER_DBL(out_max, in_max))
    {
        set_bug("Not enough input data in interpolate_vector.");
        return NULL;
    }

    if (out_vp == NULL)
    {
        NRN(out_vp = create_vector(out_count));
    }
    else
    {
        if (out_vp->length != out_count)
        {
            SET_ARGUMENT_BUG();
            return NULL;
        }
    }

    in_vp_pos = in_vp->elements;
    out_vp_pos = out_vp->elements;

    i = 0;
    interpolate_val = out_offset - in_offset;

    if (interpolate_val < 0.0)
    {
        interpolate_val = 0.0;
    }

    while (i < out_count)
    {
        left_val = (int) (interpolate_val/in_step);

        right_fraction = interpolate_val/in_step - left_val;
        left_fraction = 1.0 - right_fraction;

        if (fabs(right_fraction) > DBL_EPSILON)
        {
            *out_vp_pos = left_fraction * in_vp_pos[left_val] +
                                     right_fraction * in_vp_pos[left_val + 1 ];
        }
        else
        {
             *out_vp_pos = in_vp_pos[left_val];
        }

        i++;
        out_vp_pos++;
        interpolate_val += out_step;
    }

    return out_vp;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int uniform_x_cubic_spline
(
    int     in_n,
    double  in_start,
    double  in_step,
    double* in_data,
    int     out_n,
    double  out_start,
    double  out_step,
    double* out_data
)
{
    int     i, j;
    double* in_x_pos;
    double* out_x_pos;


    if ((in_n <= 0) || (out_n <= 0))
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (in_n > fs_max_in_n)
    {
        if (fs_uniform_in_x != NULL)
        {
            kjb_free(fs_uniform_in_x);
        }

        NRE(fs_uniform_in_x = DBL_MALLOC(in_n));

#ifdef TRACK_MEMORY_ALLOCATION
        if (fs_max_out_n == NOT_SET)
        {
            add_cleanup_function(free_uniform_cubic_spline_storage);
        }
#endif

        fs_max_in_n = in_n;
    }

    if (out_n > fs_max_out_n)
    {
        if (fs_uniform_out_x != NULL)
        {
            kjb_free(fs_uniform_out_x);
        }

        NRE(fs_uniform_out_x = DBL_MALLOC(out_n));
        fs_max_out_n = out_n;
    }

    in_x_pos = fs_uniform_in_x;

    for (i=0; i<in_n; i++)
    {
        *in_x_pos = in_start + ((double) i) * in_step;
        in_x_pos++;
    }

    out_x_pos = fs_uniform_out_x;

    for (j=0; j<out_n; j++)
    {
        *out_x_pos = out_start + ((double) j) * out_step;
        out_x_pos++;
    }

    ERE(cubic_spline(in_n, fs_uniform_in_x, in_data, out_n, fs_uniform_out_x,
                     out_data));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void free_uniform_cubic_spline_storage(void)
{
    kjb_free(fs_uniform_in_x);
    kjb_free(fs_uniform_out_x);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int cubic_spline
(
    int     in_n,
    double* in_x,
    double* in_data,
    int     out_n,
    double* out_x,
    double* out_data
)
{
    double cur_out_x;
    double out_delta;
    int    i, j;


    if (in_n < 1)
    {
        SET_ARGUMENT_BUG();
        return ERROR;
    }

    if (in_n > fs_cubic_spline_max_size)
    {
        if (fs_cubic_spline_b != NULL) kjb_free(fs_cubic_spline_b);
        if (fs_cubic_spline_c != NULL) kjb_free(fs_cubic_spline_c);
        if (fs_cubic_spline_d != NULL) kjb_free(fs_cubic_spline_d);

        NRE(fs_cubic_spline_b = DBL_MALLOC(in_n));
        NRE(fs_cubic_spline_c = DBL_MALLOC(in_n));
        NRE(fs_cubic_spline_d = DBL_MALLOC(in_n));

#ifdef TRACK_MEMORY_ALLOCATION
        if (fs_cubic_spline_max_size == NOT_SET)
        {
            add_cleanup_function(free_cubic_spline_storage);
        }
#endif

        fs_cubic_spline_max_size = in_n;
    }

    ERE(get_spline_parms(in_n,in_x, in_data,
                         fs_cubic_spline_b, fs_cubic_spline_c, fs_cubic_spline_d));

    if (IS_LESSER_DBL(out_x[ 0 ], in_x[ 0 ]))
    {
        dbe(out_x[ 0 ]);
        dbe(in_x[ 0 ]);
        set_bug("Spline output X out of range.");
        return ERROR;
    }

    if (IS_GREATER_DBL(out_x[ out_n - 1 ], in_x[ in_n - 1 ]))
    {
        dbe(out_x[ out_n - 1 ]);
        dbe(in_x[ in_n - 1 ]);
        set_bug("Spline output X out of range.");
        return ERROR;
    }

    i = 0;

    for (j=0; j<out_n; j++)
    {
     cur_out_x = out_x[j];

     while ((i < (in_n - 2) ) && (cur_out_x > in_x[ i + 1 ])) i++;

        if ((i < 0) || (i >= in_n))
        {
            set_bug("Spline output X out of range.");
            return ERROR;
        }

        out_delta = cur_out_x - in_x[i];

        *out_data = eval_cubic(out_delta, in_data[i],
                               fs_cubic_spline_b[i], fs_cubic_spline_c[i],
                               fs_cubic_spline_d[i]);

        out_data++;
    }

   return NO_ERROR;
  }

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void free_cubic_spline_storage(void)
{
    kjb_free(fs_cubic_spline_b);
    kjb_free(fs_cubic_spline_c);
    kjb_free(fs_cubic_spline_d);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

 /********************* get_spline_parms ***********************/
 /*                                                            */
 /* Algorithm from page 122 of Numerical Analysis by Burden    */
 /* and Faires. "Steps", and variable names are chosen to      */
 /* reflect those in the above reference. The spline is a free */
 /* spline with second derrivatives at the endpoints set to 0. */
 /*                                                            */
 /**************************************************************/

int get_spline_parms
(
    int     n,
    double* x,
    double* a,
    double* b,
    double* c,
    double* d
)
{
    double* h;
    double* l;
    double* u;
    double* z;
    double* alpha;
    double  temp;
    int     i, j;


    if (n <= 1)
    {
        SET_ARGUMENT_BUG();
    }

    if (n > fs_gsp_max_size)
    {
        if (fs_gsp_h != NULL) kjb_free(fs_gsp_h);
        if (fs_gsp_l != NULL) kjb_free(fs_gsp_l);
        if (fs_gsp_u != NULL) kjb_free(fs_gsp_u);
        if (fs_gsp_z != NULL) kjb_free(fs_gsp_z);
        if (fs_gsp_alpha != NULL) kjb_free(fs_gsp_alpha);

        NRE(fs_gsp_h = DBL_MALLOC(n));
        NRE(fs_gsp_l = DBL_MALLOC(n));
        NRE(fs_gsp_u = DBL_MALLOC(n));
        NRE(fs_gsp_z = DBL_MALLOC(n));
        NRE(fs_gsp_alpha = DBL_MALLOC(n));

#ifdef TRACK_MEMORY_ALLOCATION
        if (fs_gsp_max_size == NOT_SET)
        {
            add_cleanup_function(free_get_spline_parms_storage);
        }
#endif

        fs_gsp_max_size = n;
    }

    h = fs_gsp_h;
    l = fs_gsp_l;
    u = fs_gsp_u;
    z = fs_gsp_z;
    alpha = fs_gsp_alpha;

    n--;

    /* Step 1 */

    for (i=0; i<n; ++i)
    {
        h[i] = x[i+1] - x[i];
    }

    /* Step 2 */

    for (i=1; i<n; ++i)
    {
        temp = a[i+1]*h[i-1] - a[i]*(x[i+1]-x[i-1]);
        temp += a[i-1]*h[i];
        alpha[i] = 3 * temp / ( h[i-1] * h[i] );
    }

    /* Step 3 */

    l [ 0 ] = (double ) 1;
    u [ 0 ] = (double ) 0;
    z [ 0 ] = (double ) 0;

    /* Step 4 */

    for (i=1; i<n; i++)
    {
        l[i] = 2*(x[i+1] - x[i-1]) - h[i-1]*u[i-1];
        u[i] = h[i] / l[i];
        z[i] = (alpha[i] - h[i-1]*z[i-1]) / l[i];
    }

    /* Step 5 */

    l[n] =  1.0;
    z[n] =  0.0;
    c[n] =  0.0;

    /* Step 6 */

    for (j=n-1; j>=0; j--)
    {
        c[j] = z[j] - u[j]*c[j+1];
        b[j] = (a[j+1] - a[j])/h[j] - h[j]*(c[j+1] + 2*c[j])/3;
        d[j] = (c[j+1] - c[j]) / (3*h[j]);
    }


    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef TRACK_MEMORY_ALLOCATION

static void free_get_spline_parms_storage(void)
{
     kjb_free(fs_gsp_h);
     kjb_free(fs_gsp_l);
     kjb_free(fs_gsp_u);
     kjb_free(fs_gsp_z);
     kjb_free(fs_gsp_alpha);
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

double eval_cubic(double x, double a, double b, double c, double d)
{

     return ((d*x + c)*x + b)*x + a;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif

