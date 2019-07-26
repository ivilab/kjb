/* $Id$ */

#include <g_cpp/g_quaternion.h>
#include <m_cpp/m_matrix.h>
#include <m_cpp/m_vector.h>
#include <l/l_sys_rand.h>
#include <iostream>

using namespace std;
using namespace kjb;

int main(int argc, char** argv)
{
    kjb_c::kjb_seed_rand_with_tod();

    Vector x = create_random_vector(3) - Vector(0.5, 0.5, 0.5);
    x.normalize();

    Vector a = cross(x, Vector(1.0, 0.0, 0.0));
    a.normalize();
    Quaternion q(a, M_PI);

    Vector y = q.rotate(x);

    cout << x << endl;
    cout << y << endl;

    return EXIT_SUCCESS;
}

