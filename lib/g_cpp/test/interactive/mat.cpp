/* $Id$ */

#include <g_cpp/g_quaternion.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <iostream>

using namespace std;
using namespace kjb;

int main(int argc, char** argv)
{
    Vector a = 2*M_PI*create_random_vector(3);
    Quaternion q1(a[0], a[1], a[2], Quaternion::XYZS);

    Vector x(1.0, 0.0, 0.0);
    Vector y(0.0, 1.0, 0.0);
    Vector z(0.0, 0.0, 1.0);

    Vector xp = q1.rotate(x);
    Vector yp = q1.rotate(y);
    Vector zp = q1.rotate(z);

    xp.resize(4, 0.0);
    yp.resize(4, 0.0);
    zp.resize(4, 0.0);

    Matrix R(4, 4, 0.0);
    R.set_col(0, xp);
    R.set_col(1, yp);
    R.set_col(2, zp);
    R(3, 3) = 1.0;

    Quaternion q2(R);

    Vector im1 = q1.get_imag();
    Vector im2 = q2.get_imag();
    cout << im1[0] << ", " << im1[1] << ", " << im1[2] << ", " << q1.get_real() << endl;
    cout << im2[0] << ", " << im2[1] << ", " << im2[2] << ", " << q2.get_real() << endl;

    return EXIT_SUCCESS;
}

