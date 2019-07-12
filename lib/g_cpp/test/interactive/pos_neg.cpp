/* $Id$ */

#include <g_cpp/g_quaternion.h>
#include <m_cpp/m_vector.h>
#include <iostream>

using namespace std;
using namespace kjb;

int main(int argc, char** argv)
{
    Quaternion q1(M_PI/2, 0.0, 0.0, Quaternion::XYZS);
    Quaternion q2(0.0, M_PI/2, 0.0, Quaternion::XYZS);
    Quaternion q3(0.0, 0.0, M_PI/2, Quaternion::XYZS);

    Vector x(1.0, 0.0, 0.0);
    Vector y(0.0, 1.0, 0.0);
    Vector z(0.0, 0.0, 1.0);

    cout << "x +90y: " << q2.rotate(x) << endl;
    cout << "x +90z: " << q3.rotate(x) << endl;
    cout << "y +90x: " << q1.rotate(y) << endl;
    cout << "y +90z: " << q3.rotate(y) << endl;
    cout << "z +90x: " << q1.rotate(z) << endl;
    cout << "z +90y: " << q2.rotate(z) << endl;

    return EXIT_SUCCESS;
}

