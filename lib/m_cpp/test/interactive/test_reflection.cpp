/**
 * @file
 * @brief test prog for Fftw_convolution_2d
 * @author Andrew Predoehl
 *
 * There is a fully-automatic version of this in the test directory.
 */
/*
 * $Id: test_reflection.cpp 16063 2013-11-21 00:52:52Z predoehl $
 */

#include <m_cpp/m_convolve.h>
#include <iostream>

namespace
{

int test(const kjb::Matrix& x, const kjb::Matrix& y)
{
    kjb::Fftw_convolution_2d::Sizes s(
            x.get_num_rows(), x.get_num_cols(),
            y.get_num_rows(), y.get_num_cols());

    kjb::Matrix z;

    KJB(ERE(kjb::debug::test_reflect_into_input_buf(x, &z, s)));
    std::cout << "Input:\n" << x << "\nOutput:\n" << z << '\n';
    return kjb_c::NO_ERROR;
}

}

int main(int argc, char** argv)
{
    const double d[] = {-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7};
    kjb::Matrix x(4, 4, d); 

    KJB(EPETE(test(x, kjb::Matrix(1, 1))));
    // output: same as input

    KJB(EPETE(test(x, kjb::Matrix(3, 3))));
    /* output:
        -8.0 -7.0 -6.0 -5.0 -5.0 -8.0
        -4.0 -3.0 -2.0 -1.0 -1.0 -4.0
         0.0  1.0  2.0  3.0  3.0  0.0
         4.0  5.0  6.0  7.0  7.0  4.0
         4.0  5.0  6.0  7.0  7.0  4.0
        -8.0 -7.0 -6.0 -5.0 -5.0 -8.0
     */

    // ridiculously large:  quadruple size of x (exactly)
    KJB(EPETE(test(x, kjb::Matrix(5, 5))));
    /* output:
        -8.0 -7.0 -6.0 -5.0 -5.0 -6.0 -7.0 -8.0
        -4.0 -3.0 -2.0 -1.0 -1.0 -2.0 -3.0 -4.0
         0.0  1.0  2.0  3.0  3.0  2.0  1.0  0.0
         4.0  5.0  6.0  7.0  7.0  6.0  5.0  4.0
         4.0  5.0  6.0  7.0  7.0  6.0  5.0  4.0
         0.0  1.0  2.0  3.0  3.0  2.0  1.0  0.0
        -4.0 -3.0 -2.0 -1.0 -1.0 -2.0 -3.0 -4.0
        -8.0 -7.0 -6.0 -5.0 -5.0 -6.0 -7.0 -8.0
     */

    KJB(EPETE(test(x, kjb::Matrix(1, 4))));
    /* output:
        -8.0 -7.0 -6.0 -5.0 -5.0 -6.0 -8.0
        -4.0 -3.0 -2.0 -1.0 -1.0 -2.0 -4.0
         0.0  1.0  2.0  3.0  3.0  2.0  0.0
         4.0  5.0  6.0  7.0  7.0  6.0  4.0
     */

    KJB(EPETE(test(x, kjb::Matrix(4, 1))));
    /* output:
        -8.0 -7.0 -6.0 -5.0
        -4.0 -3.0 -2.0 -1.0
         0.0  1.0  2.0  3.0
         4.0  5.0  6.0  7.0
         4.0  5.0  6.0  7.0
         0.0  1.0  2.0  3.0
        -8.0 -7.0 -6.0 -5.0
     */

    KJB(EPETE(test(x, kjb::Matrix(1, 2))));
    /* output:
        -8.0 -7.0 -6.0 -5.0 -5.0
        -4.0 -3.0 -2.0 -1.0 -1.0
         0.0  1.0  2.0  3.0  3.0
         4.0  5.0  6.0  7.0  7.0
     */

    KJB(EPETE(test(x, kjb::Matrix(2, 1))));
    /* output:
        -8.0 -7.0 -6.0 -5.0
        -4.0 -3.0 -2.0 -1.0
         0.0  1.0  2.0  3.0
         4.0  5.0  6.0  7.0
         4.0  5.0  6.0  7.0
     */

    return EXIT_SUCCESS;
}
