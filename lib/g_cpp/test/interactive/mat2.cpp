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
    Quaternion q(a[0], a[1], a[2], Quaternion::XYZS);

    Matrix Rx = Matrix::create_3d_rotation_matrix(a[0], 1, 0, 0);
    Matrix Ry = Matrix::create_3d_rotation_matrix(a[1], 0, 1, 0);
    Matrix Rz = Matrix::create_3d_rotation_matrix(a[2], 0, 0, 1);

    Matrix R = Rz*Ry*Rx;
    Matrix S = q.get_rotation_matrix();
    S.resize(3, 3);

    cout << R << endl;
    cout << endl;
    cout << S << endl;
    cout << endl;
    cout << (R - S) << endl;
    cout << endl;

    Vector x = create_random_vector(3);
    Vector y1 = R * x;
    Vector y2 = q.rotate(x);
    Vector x1 = matrix_transpose(R) * y1;
    Vector x2 = q.conj().rotate(y2);

    cout << x << endl;
    cout << y1 << endl;
    cout << y2 << endl;
    cout << x1 << endl;
    cout << x2 << endl;

    return EXIT_SUCCESS;
}

