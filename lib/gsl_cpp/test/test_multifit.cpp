/* $Id: test_multifit.cpp 20313 2016-02-02 06:14:36Z predoehl $ */
/* {{{=========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Kyle Simek
 * =========================================================================== }}}*/

// vim: tabstop=4 shiftwidth=4 foldmethod=marker

#include <iostream>
using namespace std;
#ifndef KJB_HAVE_GSL
int main(int argc, char** argv)
{
    cout << "gsl not installed." << endl;
    return 1;
}
#else

#include <gsl_cpp/gsl_multifit.h>

using namespace kjb;

int eval_f(const gsl_vector* x, void * params, gsl_vector* f)
{
    f->data[0] = x->data[0] - 0.0;
    f->data[1] = x->data[0] - 1.0;
    return GSL_SUCCESS;
}

int eval_df(const gsl_vector* x, void * params, gsl_matrix* df)
{
    df->data[0] = 1.0;
    df->data[1] = 1.0;
    return GSL_SUCCESS;
}

int eval_fdf(const gsl_vector* x, void * params, gsl_vector* f, gsl_matrix* df)
{
    eval_f(x, params, f);
    eval_df(x, params, df);
    return GSL_SUCCESS;
}

int main(int argc, char** argv)
{
#if HOW_IT_WAS_JANUARY_2016 
    Gsl_multifit_fdf solver( 2, 1);
    gsl_vector* guess = gsl_vector_alloc(1);
    guess->data[0]  = 5.0;

    gsl_multifit_function_fdf f;

    f.f = eval_f;
    f.df = eval_df;
    f.fdf = eval_fdf;
    f.n = 2;
    f.p = 1;
    f.params = NULL;

    solver.initialize(&f, guess);

    size_t iterations = 0;
    size_t MAX_ITERATIONS = 500;
    while(solver.test_gradient() && iterations++ < MAX_ITERATIONS)
    {
        solver.iterate();
    }

    if(iterations >= MAX_ITERATIONS)
    {
        cerr << "FITTING FAILED!" << endl;
        return EXIT_FAILURE;
    }

    cout << "expected result: " << 0.5 << endl;
    cout << "result: " << solver.position()->data[0] << endl;
    cout << "iterations: " << iterations << endl;
#endif /* HOW_IT_WAS_JANUARY_2016 */
    return EXIT_SUCCESS;
}

#endif
