
/* $Id: test_quaternion.cpp 9250 2011-04-09 00:12:25Z ksimek $ */

#include <iostream>
#include <cmath>
#include <string.h>
#include <ostream>
#include <sstream>
#include <assert.h>
#include <float.h>
#include "g_cpp/g_quaternion.h"
#include "test.h"
#include "m_cpp/m_vector.h"
#include "sample/sample_misc.h"

#define DB(x) std::cout << #x << ": \n" << x << std::endl;

using namespace::kjb;

int gimbal_lock_counter = 0;
int time_factor = 1;

std::string print_rotation_matrix(const Quaternion & q1);
std::string print_axis_angle(const Quaternion & q1);
std::string print_euler_angles(const Quaternion & q1);
bool compare_rotation_matrix(const Quaternion & q1, const Quaternion & q2);
bool compare_axis_angle(const Quaternion & q1, const Quaternion & q2);
bool compare_euler_angles(const Quaternion & q1, const Quaternion & q2);
const char* get_euler_mode_string(Quaternion::Euler_mode mode);

int main(int argc, char *argv[])
{
    if(argc >= 2)
    {
        time_factor = atoi(argv[1]);
    }
//    Vector::set_epsilon(1e-6);
//    Matrix::set_epsilon(1e-6);

    Vector x_hat(4, 0.); x_hat(0) = 1; x_hat(1) = 0; x_hat(2) = 0;
    Vector y_hat(4, 0.); y_hat(0) = 0; y_hat(1) = 1; y_hat(2) = 0;
    Vector z_hat(4, 0.); z_hat(0) = 0; z_hat(1) = 0; z_hat(2) = 1;


    // constructor
    {
        Quaternion q;
//        TEST_TRUE(q.norm() == 1);
        TEST_MAT_EQUAL(q.get_rotation_matrix(), create_identity_matrix(4));
    }

    // TEST EULER ROTATIONS: Euler -> matrix
    {
        Quaternion q = Quaternion(M_PI/2, M_PI/2, M_PI/2, Quaternion::ZXZR);
//        TEST_TRUE(q.norm() == 1);

        Matrix m = q.get_rotation_matrix();

        // rotated axis vectors
        Vector x_hat_r = m.get_col(0);
        Vector y_hat_r = m.get_col(1);
        Vector z_hat_r = m.get_col(2);

        TEST_VEC_EQUAL(x_hat_r, z_hat);
        TEST_VEC_EQUAL(y_hat_r, -y_hat);
        TEST_VEC_EQUAL(z_hat_r, x_hat);

        // Lets try a non-symmetric one:
        q = Quaternion(M_PI/2, M_PI/2, 3*M_PI/2, Quaternion::ZXZR);

        m = q.get_rotation_matrix();

        // rotated axis vectors
        x_hat_r = m.get_col(0);
        y_hat_r = m.get_col(1);
        z_hat_r = m.get_col(2);

        TEST_VEC_EQUAL(x_hat_r, -z_hat);
        TEST_VEC_EQUAL(y_hat_r, y_hat);
        TEST_VEC_EQUAL(z_hat_r, x_hat);



        // Same, but w/ odd parity
        q = Quaternion(M_PI/2, M_PI/2, 3*M_PI/2, Quaternion::ZYZR);

        m = q.get_rotation_matrix();

        // rotated axis vectors
        x_hat_r = m.get_col(0);
        y_hat_r = m.get_col(1);
        z_hat_r = m.get_col(2);

        TEST_VEC_EQUAL(x_hat_r, x_hat);
        TEST_VEC_EQUAL(y_hat_r, -z_hat);
        TEST_VEC_EQUAL(z_hat_r, y_hat);



        // HOW ABOUT A NON-REPEATING FORM?
        q = Quaternion(M_PI/2, M_PI/2, 3*M_PI/2, Quaternion::ZXYR);

        m = q.get_rotation_matrix();

        // rotated axis vectors
        x_hat_r = m.get_col(0);
        y_hat_r = m.get_col(1);
        z_hat_r = m.get_col(2);

        TEST_VEC_EQUAL(x_hat_r, x_hat);
        TEST_VEC_EQUAL(y_hat_r, z_hat);
        TEST_VEC_EQUAL(z_hat_r, -y_hat);

        // HOW ABOUT A NON-REPEATING WITH ODD PARITY?
        q = Quaternion(M_PI/2, M_PI/2, 3*M_PI/2, Quaternion::ZYXR);
        m = q.get_rotation_matrix();

        // rotated axis vectors
        x_hat_r = m.get_col(0);
        y_hat_r = m.get_col(1);
        z_hat_r = m.get_col(2);


        TEST_VEC_EQUAL(x_hat_r, -z_hat);
        TEST_VEC_EQUAL(y_hat_r, -y_hat);
        TEST_VEC_EQUAL(z_hat_r, -x_hat);

    }

    // TEST EULER ROTATIONS: Matrix -> eulers
    {
        Matrix m = create_identity_matrix(4);
        Quaternion q(m);
        TEST_TRUE(q == Quaternion().init_identity());
        
        // manually construct a rotation matrix representing a
        // ZYX euler rotation
        float angle = M_PI/4.2;
        float cos_a = cos(angle);
        float sin_a = sin(angle);

        m(0,0) = cos_a; m(0,1) = -sin_a; m(0,2) = 0; m(0,3) = 0;
        m(1,0) =+sin_a; m(1,1) =  cos_a; m(1,2) = 0; m(1,3) = 0;
        m(2,0) = 0;     m(2,1) = 0;      m(2,2) = 1; m(2,3) = 0;
        m(3,0) = 0;     m(3,1) = 0;      m(3,2) = 0; m(3,3) = 1;

        Matrix m_2 = Matrix(4,4);
        Matrix m_3 = Matrix(4,4);

        float angle_2 = M_PI/8.2;
        cos_a = cos(angle_2);
        sin_a = sin(angle_2);

        m_2(0,0) =  cos_a; m_2(0,1) = 0; m_2(0,2) =+sin_a; m_2(0,3) = 0;
        m_2(1,0) = 0;      m_2(1,1) = 1; m_2(1,2) = 0;     m_2(1,3) = 0;
        m_2(2,0) = -sin_a; m_2(2,1) = 0; m_2(2,2) = cos_a; m_2(2,3) = 0;
        m_2(3,0) = 0;      m_2(3,1) = 0; m_2(3,2) = 0;     m_2(3,3) = 1;

        float angle_3 = M_PI/2.2;
        cos_a = cos(angle_3);
        sin_a = sin(angle_3);

        m_3(0,0) = 1; m_3(0,1) = 0;     m_3(0,2) = 0;      m_3(0,3) = 0;
        m_3(1,0) = 0; m_3(1,1) = cos_a; m_3(1,2) = -sin_a; m_3(1,3) = 0;
        m_3(2,0) = 0; m_3(2,1) =+sin_a; m_3(2,2) =  cos_a; m_3(2,3) = 0;
        m_3(3,0) = 0; m_3(3,1) = 0;     m_3(3,2) = 0;      m_3(3,3) = 1;

        Quaternion q_eul;
        Vector eulers;

        // just around Z
        q = Quaternion(m);
        q.set_euler_mode(Quaternion::ZYXR);
        eulers = Vector(3,0.); eulers(0) = angle;

        TEST_VEC_EQUAL(eulers, q.get_euler_angles());

        q_eul = Quaternion(angle, 0, 0, Quaternion::ZYXR);
        TEST_TRUE(q == q_eul);


        // just around y
        q = Quaternion(m_2);
        q.set_euler_mode(Quaternion::ZYXR);
        eulers = Vector(3,0.); eulers(1) = angle_2;

//        DB(eulers);

        Vector derived_eulers = q.get_euler_angles();
        q_eul = Quaternion(derived_eulers(0), derived_eulers(1), derived_eulers(2), Quaternion::ZYXR);

        TEST_MAT_EQUAL(m_2, q_eul.get_rotation_matrix());
        TEST_TRUE(q == q_eul);

        TEST_VEC_EQUAL(eulers, q.get_euler_angles());

        // just around x
        q = Quaternion(m_3);
        q.set_euler_mode(Quaternion::ZYXR);
        eulers = Vector(3,0.); eulers(2) = angle_3;

        TEST_VEC_EQUAL(eulers, q.get_euler_angles());

        q_eul = Quaternion(0, 0, angle_3, Quaternion::ZYXR);
        TEST_TRUE(q == q_eul);

        // Around Z, Y, X
        {
            // ROTATE BY COPOSING MATRIX
            Matrix rotation_matrix = m * m_2 * m_3;
            q = Quaternion(rotation_matrix);
            q.set_euler_mode(Quaternion::ZYXR);
            eulers = Vector(3); eulers(0) = angle; eulers(1) = angle_2; eulers(2) = angle_3;
            // Matrix -> eulers
            TEST_VEC_EQUAL(eulers, q.get_euler_angles());

            // reverse: eulers -> Matrix
            q_eul = Quaternion(eulers(0), eulers(1), eulers(2), Quaternion::ZYXR);

            TEST_TRUE(q == q_eul);
            TEST_MAT_EQUAL(q.get_rotation_matrix(), q_eul.get_rotation_matrix());


            // ROTATE BY COMPOSING QUATERNIONS
            Quaternion q_1(m), q_2(m_2), q_3(m_3);
            q = q_1 * q_2 * q_3;
            TEST_MAT_EQUAL(q.get_rotation_matrix(), rotation_matrix);

            q = q_1;
            q *= q_2;
            q *= q_3;
            TEST_MAT_EQUAL(q.get_rotation_matrix(), rotation_matrix);
        }
    }

    // COPYING
    {
        Vector axis(3); axis(0) = 0; axis(1) = 1; axis(2) = 0;
        float angle = M_PI/2;
        Quaternion q(axis, angle);
        Quaternion q_2(M_PI/2, 0, 0, Quaternion::YZXR);
//        TEST_TRUE(q.norm == 1);
        TEST_TRUE(q == q_2);

        q_2 = Quaternion(q);
        TEST_TRUE(q == q_2);

        q_2 = q.clone();
        TEST_TRUE(q == q_2);
    }
    
    // EQUIVALENT REPRESENTATIONS #1
    Quaternion q_mat_1, q_eul_1, q_axis_1;

    q_eul_1 = Quaternion(M_PI/2, 0, 0, Quaternion::ZXZR);

    Vector axis_1 = z_hat.resize(3);
    float angle_1 = M_PI/2;
    q_axis_1 = Quaternion(axis_1, angle_1);

    Matrix mat_1;
    mat_1 = create_identity_matrix(4);
    mat_1(0,0) = 0; mat_1(0,1) = -1;
    mat_1(1,0) = 1; mat_1(1,1) = 0;
    q_mat_1 = Quaternion(mat_1);

    TEST_TRUE(q_eul_1 == q_axis_1);
    TEST_TRUE(q_mat_1 == q_axis_1);

    // EQUIVALENT REPRESENTATIONS #2
    Quaternion q_mat_2, q_eul_2, q_axis_2;

    q_eul_2 = Quaternion(M_PI/2, 0, 0, Quaternion::YXYR);

    Vector axis_2 = y_hat.resize(3);
    float angle_2 = M_PI/2;
    q_axis_2 = Quaternion(axis_2, angle_2);

    Matrix mat_2;
    mat_2 = create_identity_matrix(4);
    mat_2(0,0) = 0; mat_2(0,2) = 1;
    mat_2(2,0) = -1; mat_2(2,2) = 0;
    q_mat_2 = Quaternion(mat_2);

    TEST_TRUE(q_eul_2 == q_axis_2);

//    DB(q_axis_2);
//    DB(q_mat_2);
    TEST_TRUE(q_mat_2 == q_axis_2);

    // CHECK CONVERSIONS BETWEEN REPRESENTATIONS
    {
        // create from eulers
        q_eul_1;

        TEST_VEC_EQUAL( q_axis_1.get_axis(), q_eul_1.get_axis());
        TEST_FLT_EQUAL( q_axis_1.get_angle(), q_eul_1.get_angle());
        TEST_MAT_EQUAL( q_mat_1.get_rotation_matrix(), q_eul_1.get_rotation_matrix());

        // create from axis
        q_axis_1;

        q_axis_1.set_euler_mode(Quaternion::ZXZR);
        TEST_VEC_EQUAL(q_axis_1.get_euler_angles(), q_eul_1.get_euler_angles());
        TEST_MAT_EQUAL(q_axis_1.get_rotation_matrix(), q_mat_1.get_rotation_matrix());

        // create from rotation
        q_mat_1;

        q_mat_1.set_euler_mode(Quaternion::ZXZR);
        TEST_VEC_EQUAL(q_mat_1.get_euler_angles(), q_eul_1.get_euler_angles());
        TEST_FLT_EQUAL(q_mat_1.get_angle(), q_axis_1.get_angle());
        TEST_VEC_EQUAL(q_mat_1.get_axis(), q_axis_1.get_axis());
        
        // Equivalent representations #2
        // create from eulers
        q_eul_2;

        TEST_VEC_EQUAL( q_axis_2.get_axis(), q_eul_2.get_axis());
        TEST_FLT_EQUAL( q_axis_2.get_angle(), q_eul_2.get_angle());
        TEST_MAT_EQUAL( q_mat_2.get_rotation_matrix(), q_eul_2.get_rotation_matrix());

        // create from axis
        q_axis_2;

        q_axis_2.set_euler_mode(Quaternion::YXYR);
        TEST_VEC_EQUAL(q_axis_2.get_euler_angles(), q_eul_2.get_euler_angles());
        TEST_MAT_EQUAL(q_axis_2.get_rotation_matrix(), q_mat_2.get_rotation_matrix());

        // create from rotation
        q_mat_2;

        q_mat_2.set_euler_mode(Quaternion::YXYR);
        TEST_VEC_EQUAL(q_mat_2.get_euler_angles(), q_eul_2.get_euler_angles());
        TEST_FLT_EQUAL(q_mat_2.get_angle(), q_axis_2.get_angle());
        TEST_VEC_EQUAL(q_mat_2.get_axis(), q_axis_2.get_axis());

    }

    // ENSURE MUTATION CHANGES REPRESENTATIONS
    {
        // create from eulers1
        Quaternion q_eul = q_eul_1;
        // set matrix2
        q_eul.set_rotation_matrix(mat_2);
        q_eul.set_euler_mode(Quaternion::YXYR);
        // get eulers2
        TEST_VEC_EQUAL(q_eul.get_euler_angles(), q_eul_2.get_euler_angles());
        // get axis2
        TEST_VEC_EQUAL(q_eul.get_axis(), q_axis_2.get_axis());
        TEST_FLT_EQUAL(q_eul.get_angle(), q_axis_2.get_angle());

        // create from eulers1
        q_eul = q_eul_1;
        // set axis
        q_eul.set_axis_angle(axis_2, angle_2);
        q_eul.set_euler_mode(Quaternion::YXYR);
        // get eulers2
        TEST_VEC_EQUAL(q_eul.get_euler_angles(), q_eul_2.get_euler_angles());
        // get matrix2
        TEST_MAT_EQUAL(q_eul.get_rotation_matrix(), q_mat_2.get_rotation_matrix());

        // create from matrix1
        Quaternion q_mat = q_mat_1;
        // set eulers2
        q_mat.set_euler_mode(Quaternion::YXYR);
        q_mat.set_euler_angles(
                q_mat_2.get_euler_angles()(0),
                q_mat_2.get_euler_angles()(1),
                q_mat_2.get_euler_angles()(2));
        // get matrix2
        TEST_MAT_EQUAL(q_mat.get_rotation_matrix(), q_mat_2.get_rotation_matrix());
        // get axis2
        TEST_VEC_EQUAL(q_mat.get_axis(), q_axis_2.get_axis());
        TEST_FLT_EQUAL(q_mat.get_angle(), q_axis_2.get_angle());

        Vector euls;
        // change euler representation
        q_eul.set_euler_mode(Quaternion::XYZR);
        euls = Vector(3,0.); euls(1) = M_PI/2;
        TEST_VEC_EQUAL(q_eul.get_euler_angles(), euls);
        
        q_eul.set_euler_mode(Quaternion::XZYR);
        euls = Vector(3,0.); euls(2) = M_PI/2;
        TEST_VEC_EQUAL(q_eul.get_euler_angles(), euls);
    }

    // IMAG(), REAL()
    {
        Quaternion q(0.5, 0.5, 0.5, 0.5);
        Vector v(3); v(0) = v(1) = v(2) = 0.5;
        TEST_VEC_EQUAL(v, q.get_imag());
        TEST_FLT_EQUAL(0.5, q.get_real());

        /*
        q = Quaternion(v);
        v = Vector(3, std::sqrt(1.0/3.0));
        TEST_TRUE(v == q.get_imag());
        TEST_TRUE(0 == q.get_real());
        */
    }

    // OPERATORS
    {
        Quaternion q1(x_hat.resize(3), M_PI/4);
        Quaternion q2(y_hat.resize(3), M_PI/2);
        Quaternion q3;
//        DB(q3);
        // rotating reference frame
        q3.set_euler_mode(Quaternion::XYZR);
        q3.set_euler_phi(M_PI/4);
        TEST_TRUE(q3 == q1);

        Quaternion q = q1 * q2;
        q3.set_euler_theta(M_PI/2);
        TEST_TRUE(q == q3);

        // stationary reference frame
        q3 = Quaternion();
        q3.set_euler_mode(Quaternion::XYZS);
        q3.set_euler_phi(M_PI/4);
        TEST_TRUE(q3 == q1);
        q3.set_euler_theta(M_PI/2);
        q = q2 * q1;
        TEST_TRUE(q == q3);
    }


    // VECTOR ROTATION
    {
        Quaternion q(y_hat.resize(3), M_PI/2);
        Vector v = q.rotate(x_hat.resize(3));
        TEST_VEC_EQUAL(v, -z_hat.resize(3));
    }

    // CONJUGATE
    {
        Quaternion q(0.5, 0.5, 0.5, 0.5);
        Quaternion q_conj(-0.5, -0.5, -0.5, 0.5);
        Quaternion q_test = q.conj();
        TEST_TRUE(q_conj == q_test);

//        std::cout << "q: " << q << std::endl;
//        std::cout << "q_conj: " << q_conj << std::endl;
    }

    /*
     * specific oddball tests
     */
    {
        Quaternion q1 = Quaternion(
                3.53098744e+00,
                -1.29101055e+00,
                4.89539111e+00,
                Quaternion::XESS);
        Quaternion q2 = Quaternion(
                q1.get_axis(),
                q1.get_angle());

        q2.set_euler_mode(Quaternion::XESS);
        TEST_VEC_EQUAL(q1.get_euler_angles(), q2.get_euler_angles());

        // April 8, 2011: set_from_directions fails when directions are close to parallel
        Vector v1(0.0, 0.0, 1.0);
        Vector v2(0.037113271453119587, -0.26457264317930562, -0.96365134855048218);

        Quaternion q;
        q.set_from_directions(v1, v2);

        for(double i = 0.01; i < 10; i *= 1.5)
        {
            Vector v3(i, 0.0, 0.0);
            Vector v4 = q.rotate(v3);
        }

    }

    /**
     * At this point, all tests have used "safe" euler angles,
     * i.e. in range [0,PI).  In general, a rotation can be represented
     * in an infinite number of equivalent representations.  Our class
     * should handle these ambiguities by ensuring the euler angles are
     * inside a limited range. 
     *
     * The following code picks a representation (euler angles, matrix, axis/angle),
     * generates a random quaternion using that representation, and then 
     * converts it to another random representation and checks if the 
     * quaternions are equivalent.
     */
    srand((unsigned) time(0));

//    Vector::set_epsilon(1e-5);
//    Matrix::set_epsilon(1e-5);
    for(int i = 0; i < time_factor * 10000; i++)
    {
//        std::cout << "\b\b\b\b\b\b\b\b" << i ;
        Quaternion q1, q2;
        Quaternion::Euler_mode eul_mode;
        enum Quat_mode {EULER = 0, AXIS, MATRIX};
        Quat_mode mode = Quat_mode((int) (rand()%3));

        const char * q1_mode;
        const char * q2_mode;

        bool (*compare_func)(const Quaternion & q1, const Quaternion & q2) = NULL;
        std::string (*print_1)(const Quaternion & q1) = NULL;
        std::string (*print_2)(const Quaternion & q1) = NULL;

        // generate euler mode
        eul_mode = Quaternion::Euler_mode((int) (rand()%48));

        // rand 1-3
        switch(mode)
        {
            case EULER:
                {
                    q1_mode = "Euler Angles";
                    print_1 = print_euler_angles;
                    compare_func = compare_euler_angles;



                    float e1 = 0, e2 = 0, e3 = 0;

                    const double MARGIN = 0;
                    const double THETA_MARGIN = 1e-5;

                    e1 = (float) rand()/RAND_MAX * (M_PI/2 - 2*MARGIN);
                    e1 += MARGIN;

                    e2 = (float) rand()/RAND_MAX * (M_PI/2 - 2*THETA_MARGIN);
                    e2 += THETA_MARGIN;

                    e3 = (float) rand()/RAND_MAX * (M_PI/2 - 2*MARGIN);
                    e3 += MARGIN;

                    // all angles are roughly between 0 and PI/2
                    // with a small margin around 0 and PI/2
                    
                    // pick a random quadrant at either side of zero
                    // and add our value to it
                    e1 += (rand()%20 - 10) * M_PI/2;  // pick a quadrant
                    e2 += (rand()%20 - 10) * M_PI/2;
                    e3 += (rand()%20 - 10) * M_PI/2;

                    q1 = Quaternion(e1,e2,e3, eul_mode);

                    break;
                }
            case AXIS:
                {
                    q1_mode = "Axis/Angle";
                    print_1 = print_axis_angle;
                    compare_func = compare_axis_angle;

                    kjb_c::Vector* u_c = NULL;
                    ETX(get_random_unit_vector(&u_c, 3));
                    Vector axis(u_c);

                    double angle = kjb_c::sample_from_uniform_distribution(-M_PI, M_PI);
                    q1 = Quaternion(axis, angle);
                    break;
                }
            case MATRIX:
                {
                    q1_mode = "Rotation Matrix";
                    print_1 = print_rotation_matrix;
                    compare_func = compare_rotation_matrix;

                    // generate x axis
                    Vector x = Vector().randomize(3).normalize();
                    Vector y = Vector().randomize(3).normalize();
                    y = cross(y,x).normalize();

                    assert(fabs(dot(y,x)) < FLT_EPSILON);

                    Vector z = cross(x, y).normalize();

                    assert(fabs(dot(z,x)) < FLT_EPSILON);
                    assert(fabs(dot(z, y)) < FLT_EPSILON);

                    assert(fabs(dot(x, x) - 1.0) < FLT_EPSILON);
                    assert(fabs(dot(y, y) - 1.0) < FLT_EPSILON);
                    assert(fabs(dot(z, z) - 1.0) < FLT_EPSILON);

                    Matrix rot_mat = Matrix(4,4,0.);
                    rot_mat(3,3) = 1;

                    rot_mat(0,0) = x(0); rot_mat(0,1) = y(0); rot_mat(0,2) = z(0);
                    rot_mat(1,0) = x(1); rot_mat(1,1) = y(1); rot_mat(1,2) = z(1);
                    rot_mat(2,0) = x(2); rot_mat(2,1) = y(2); rot_mat(2,2) = z(2);

                    TEST_MAT_EQUAL(Matrix(rot_mat).transpose(), Matrix(rot_mat).inverse());

                    q1 = Quaternion(rot_mat);

                    break;
                }
        }

        q1.set_euler_mode(eul_mode);

        mode = Quat_mode((int) (rand()%3));
        switch(mode)
        {
            case EULER:
                q2_mode = "Euler Angles";
                print_2 = print_euler_angles;
                q2 = Quaternion(
                        q1.get_euler_angles()(0),
                        q1.get_euler_angles()(1),
                        q1.get_euler_angles()(2),
                        eul_mode);
                break;
            case AXIS:
                q2_mode = "Axis/Angle";
                print_2 = print_axis_angle;
                q2 = Quaternion(q1.get_axis(), q1.get_angle());
                break;
            case MATRIX:
                q2_mode = "Rotation Matrix";
                print_2 = print_rotation_matrix;
                q2 = Quaternion(q1.get_rotation_matrix());
                break;
        }

        q2.set_euler_mode(eul_mode);

        if(!compare_func(q1, q2)) 
        {
            std::cout << "TEST FAILED: Random equivalence\n";
            std::cout << "Euler mode: " << get_euler_mode_string(q2.get_euler_mode()) << "\n";
            std::cout << "q1: " << q1_mode << ";\tq2: " << q2_mode << std::endl;
            std::cout << "q1: " << print_1(q1) << ";\nq2: " << print_1(q2) << std::endl << std::endl;
            std::cout << "q1: " << print_2(q1) << ";\nq2: " << print_2(q2) << std::endl << std::endl;

            abort();
        } else {
//            std::cout << "TEST PASSED: Random equivalence\n";
    
        }


    }

    // vector constructor
    std::cout << "All tests passed\n";
//    std::cout << "gimbal lock counter: " << gimbal_lock_counter << "\n";
    return 0;
}

bool compare_euler_angles(const Quaternion & q1, const Quaternion & q2)
{
    bool equal = VEC_EQUAL(q1.get_euler_angles(), q2.get_euler_angles());

    if(q1._gimbal_lock || q2._gimbal_lock) {
        gimbal_lock_counter++;
//        std::cout << "Gimbal lock\n";
        if(!q1._gimbal_lock || !q2._gimbal_lock) {
//            std::cout << "ONLY ONE!\n";
        }
    }

    return equal || q1._gimbal_lock && q2._gimbal_lock;
}

bool compare_axis_angle(const Quaternion & q1, const Quaternion & q2)
{
    Vector axis = q1.get_axis();
    Vector axis2 = q2.get_axis();

    double angle = q1.get_angle();
    double angle2 = q2.get_angle();
    if(!VEC_EQUAL(axis, axis2))
    {
        axis2 *= -1;
        angle2 *= -1;
    }

    return q1._gimbal_lock && q2._gimbal_lock || VEC_EQUAL(axis, axis2) && FLT_EQUAL(angle, angle2);
}

bool compare_rotation_matrix(const Quaternion & q1, const Quaternion & q2)
{
    return q1._gimbal_lock && q2._gimbal_lock || MAT_EQUAL(q1.get_rotation_matrix(), q2.get_rotation_matrix());
}

std::string print_euler_angles(const Quaternion & q1)
{
    std::ostringstream out;
    out << q1.get_euler_angles();
    return out.str();

}

std::string print_axis_angle(const Quaternion & q1)
{
    std::ostringstream out;
    out << q1.get_axis() << " " << q1.get_angle();
    return out.str();
}

std::string print_rotation_matrix(const Quaternion & q1)
{
    std::ostringstream out;
    out << q1.get_rotation_matrix();
    return out.str();
}

const char* get_euler_mode_string(Quaternion::Euler_mode mode)
{
    switch(mode) {
        case 0: return "XESS";
        case 1: return "XYXS";
        case 2: return "XEDS";
        case 3: return "XYZS";
        case 4: return "XOSS";
        case 5: return "XZXS";
        case 6: return "XODS";
        case 7: return "XZYS";
        case 8: return "YESS";
        case 9: return "YZYS";
        case 10: return "YEDS";
        case 11: return "YZXS";
        case 12: return "YOSS";
        case 13: return "YXYS";
        case 14: return "YODS";
        case 15: return "YXZS";
        case 16: return "ZESS";
        case 17: return "ZXZS";
        case 18: return "ZEDS";
        case 19: return "ZXYS";
        case 20: return "ZOSS";
        case 21: return "ZYZS";
        case 22: return "ZODS";
        case 23: return "ZYXS";
        case 24: return "XESR";
        case 25: return "XYXR";
        case 26: return "XEDR";
        case 27: return "ZYXR";
        case 28: return "XOSR";
        case 29: return "XZXR";
        case 30: return "XODR";
        case 31: return "YZXR";
        case 32: return "YESR";
        case 33: return "YZYR";
        case 34: return "YEDR";
        case 35: return "XZYR";
        case 36: return "YOSR";
        case 37: return "YXYR";
        case 38: return "YODR";
        case 39: return "ZXYR";
        case 40: return "ZESR";
        case 41: return "ZXZR";
        case 42: return "ZEDR";
        case 43: return "YXZR";
        case 44: return "ZOSR";
        case 45: return "ZYZR";
        case 46: return "ZODR";
        case 47: return "XYZR";
    }
}
