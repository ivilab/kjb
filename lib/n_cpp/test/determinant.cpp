/* =========================================================================== *
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
   |  Author:  Ernesto Brau
 * =========================================================================== */

/* $Id$ */

#include <m_cpp/m_matrix.h>
#include <n_cpp/n_svd.h>
#include <n_cpp/n_eig.h>
#include <n_cpp/n_cholesky.h>
#include <l_cpp/l_test.h>
#include <iostream>

using namespace std;
using namespace kjb;

const bool VERBOSE = false;

/** @brief  Main -- all the magic happens here. */
int main(int argc, char** argv)
{
    const size_t n = 250;
    const double eps = 1e-8;

    Matrix T = create_random_matrix(n, n);
    Matrix A = T * matrix_transpose(T);

    // the dumb way
    kjb_c::init_cpu_time();
    double ld1 = log(A.abs_of_determinant());
    long t1 = kjb_c::get_cpu_time();

    if(VERBOSE)
    {
        cout << "Dumb way\n";
        cout << "|A| = " << ld1 << endl;
        cout << "time: " << t1 / 1000.0 << "s" << endl << endl;
    }

    // the cholesky way
    kjb_c::init_cpu_time();
    double ld2 = log_det(A);
    long t2 = kjb_c::get_cpu_time();

    TEST_TRUE(fabs(ld2 - ld1) <= eps);

    if(VERBOSE)
    {
        cout << "Cholesky way\n";
        cout << "|A| = " << ld2 << endl;
        cout << "time: " << t2 / 1000.0 << "s" << endl << endl;
    }

    // the SVD way
    kjb_c::init_cpu_time();
    Svd svd(A);
    double ld3 = 0.0;
    for(Matrix::Size_type i = 0; i < svd.d().get_length(); i++)
    {
        ld3 += log(svd.d()[i]);
    }
    long t3 = kjb_c::get_cpu_time();

    TEST_TRUE(fabs(ld3 - ld1) <= eps);

    if(VERBOSE)
    {
        cout << "SVD way\n";
        cout << "|A| = " << ld3 << endl;
        cout << "time: " << t3 / 1000.0 << "s" << endl << endl;
    }

#ifdef KJB_HAVE_BOOST
    // the eig way
    kjb_c::init_cpu_time();
    boost::tuple<Matrix, Vector> lv = eig(A, true);
    double ld4 = 0.0;
    for(Matrix::Size_type i = 0; i < lv.get<1>().get_length(); i++)
    {
        ld4 += log(lv.get<1>()[i]);
    }
    long t4 = kjb_c::get_cpu_time();

    TEST_TRUE(fabs(ld4 - ld1) <= eps);

    if(VERBOSE)
    {
        cout << "Eivengalues way\n";
        cout << "|A| = " << ld4 << endl;
        cout << "time: " << t4 / 1000.0 << "s" << endl << endl;
    }
#endif

    RETURN_VICTORIOUSLY();
}

