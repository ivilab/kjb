
/* $Id: complex_phase.c 4723 2009-11-16 18:57:09Z kobus $ */


#include "m/m_incl.h" 
#include "x/x_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    double im, re, phase;
    KJB_complex z; 

    check_num_args(argc, 2, 2, NULL);

    ss1snd(argv[ 1 ], &re);
    ss1snd(argv[ 2 ], &im);

    if (re == 0.0) 
    {
        phase = ((im == 0.0) ? 0.0 : ((im > 0.0) ? M_PI / 2.0 : -M_PI / 2.0)); 
    }
    else if (re > 0.0)
    {
        phase = atan(im / re);
    }
    else if (im > 0.0) 
    {
        phase = atan(im / re) + M_PI;
    }
    else 
    {
        phase = atan(im / re) - M_PI;
    }

    dbe(phase); 

    z.r = re;
    z.i = im;

    dbe(angle_of_complex(z)); 

    return EXIT_SUCCESS; 
}

