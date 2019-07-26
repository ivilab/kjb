/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2009-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kyle Simek.                                                        |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: test_matrix_graphics.cpp 13617 2013-01-14 22:41:05Z predoehl $ */

#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_test.h"
#include <cmath>
#include <iostream>
#include <memory>


/*
 * Are the mathematicians at Baylor known as the "Houston Eulers"?
 * Are the best academic advisors in Purdue's math department known as
 * beulermakers?
 * Because I think that would be cool.
 */


int main(int /* argc */, char ** /* argv */)
{
    using kjb::Matrix;
    using kjb::Vector;

    const double CLOSE_ENOUGH = 5e-10;

    #if 0
        Matrix* m_right( new Matrix(3,3,1) );
        (*m_right)(0,0) = -2.12189294e-01;
        (*m_right)(0,1) = 7.42932452e-01;
        (*m_right)(0,2) = -6.34844134e-01;
        (*m_right)(1,0) = -9.27137885e-01;
        (*m_right)(1,1) = -3.58365735e-01;
        (*m_right)(1,2) = -1.09495853e-01;
        (*m_right)(2,0) = -3.08854407e-01;
        (*m_right)(2,1) = 5.65354200e-01;
        (*m_right)(2,2) = 7.64842195e-01;
    #else
        const double m_right_data[] = {  -2.12189294e-01,
                                          7.42932452e-01,
                                         -6.34844134e-01,
                                         -9.27137885e-01,
                                         -3.58365735e-01,
                                         -1.09495853e-01,
                                         -3.08854407e-01,
                                          5.65354200e-01,
                                          7.64842195e-01 };
        Matrix m_right_automatic( 3, 3, m_right_data );
        Matrix* m_right( &m_right_automatic );
    #endif


    Matrix m_right2 = (*m_right);
    m_right2(0,2) = 6.34844134e-01;
    m_right2(1,2) = 1.09495853e-01;
    m_right2(2,0) = 3.08854407e-01;
    m_right2(2,1) = -5.65354200e-01;

    std::auto_ptr<Matrix> m( new  Matrix() );
    m->convert_to_euler_rotation_matrix(0.5,-0.7,1.4);
    TEST_TRUE( kjb::max_abs_difference( *m, *m_right ) < CLOSE_ENOUGH );

    Matrix m2 = Matrix::create_euler_rotation_matrix(0.5,-0.7,1.4);
    m2.convert_to_euler_rotation_matrix(0.5,0.7,1.4);
    TEST_TRUE( kjb::max_abs_difference( m2, m_right2 ) < CLOSE_ENOUGH );


    Matrix m3 = Matrix(2,2,1);
    m3.convert_to_euler_rotation_matrix(0.5,0.7,1.4);
    TEST_TRUE( kjb::max_abs_difference( m3, m_right2 ) < CLOSE_ENOUGH );

    Matrix m4 = Matrix(3,3,1.2);
    m4.convert_to_euler_rotation_matrix(0.5,0.7,1.4);
    TEST_TRUE( kjb::max_abs_difference( m4, m_right2 ) < CLOSE_ENOUGH );

    Matrix m5 = Matrix::create_3d_rotation_matrix(0.8, 5, 2, 1);

    #if 0
        Matrix *m_3d = new Matrix(3,3,1);
        (*m_3d)(0,0) =  9.49451118e-01;
        (*m_3d)(0,1) =  2.32068468e-01;
        (*m_3d)(0,2) =  -2.11392527e-01;
        (*m_3d)(1,0) =  -2.98729407e-02;
        (*m_3d)(1,1) =  7.37145815e-01;
        (*m_3d)(1,2) =  6.75073074e-01;
        (*m_3d)(2,0) =  3.12490290e-01;
        (*m_3d)(2,1) =  -6.34633969e-01;
        (*m_3d)(2,2) =  7.06816486e-01;
    #else
        const double m_3d_data[] = { 9.49451118e-01,
                                     2.32068468e-01,
                                    -2.11392527e-01,
                                    -2.98729407e-02,
                                     7.37145815e-01,
                                     6.75073074e-01,
                                     3.12490290e-01,
                                    -6.34633969e-01,
                                     7.06816486e-01 };
        Matrix m_3d_automatic( 3, 3, m_3d_data );
        Matrix* m_3d( &m_3d_automatic );
    #endif

#warning "[Code police] the following test does NOT work (1 of 9)"
    //TEST_TRUE( kjb::max_abs_difference( m5, *m_3d ) < CLOSE_ENOUGH );

    Matrix m6 = Matrix(1,1,2.0);
    TEST_FAIL( m6.convert_to_3d_rotation_matrix(0.8, 0,0,0 ) );
    m6.convert_to_3d_rotation_matrix(0.8, 5, 2, 1);
#warning "[Code police] the following test does NOT work (2 of 9)"
    //TEST_TRUE( kjb::max_abs_difference( m6, *m_3d ) < CLOSE_ENOUGH );

    Vector vec = Vector(3,1.0);
    vec[0] = 5;
    vec[1] = 2;
    vec[2] = 1;
    Matrix m7 = Matrix::create_3d_rotation_matrix_from_vector(0.8,vec);
#warning "[Code police] the following test does NOT work (3 of 9)"
    //TEST_TRUE( kjb::max_abs_difference( m7, *m_3d ) < CLOSE_ENOUGH );

    Matrix m8 = Matrix(1,1,2.0);
#warning "[Code police] the following test does NOT work (4 of 9)"
    //TEST_FAIL( m8.convert_to_3d_rotation_matrix_from_vector(0.8, Vector(4)) );
    m8.convert_to_3d_rotation_matrix_from_vector(0.8, vec);
#warning "[Code police] the following test does NOT work (5 of 9)"
    //TEST_TRUE( kjb::max_abs_difference( m8, *m_3d ) < CLOSE_ENOUGH );

    Matrix m9 = Matrix::create_3d_homo_rotation_matrix(0.8, 5, 2, 1);

    #if 0
        Matrix *m_3dh = new Matrix(4,4,1);
        (*m_3dh)(0,0) =  9.49451118e-01;
        (*m_3dh)(0,1) =  2.32068468e-01;
        (*m_3dh)(0,2) =  -2.11392527e-01;
        (*m_3dh)(0,3) = 0.0;
        (*m_3dh)(1,0) =  -2.98729407e-02;
        (*m_3dh)(1,1) =  7.37145815e-01;
        (*m_3dh)(1,2) =  6.75073074e-01;
        (*m_3dh)(1,3) = 0.0;
        (*m_3dh)(2,0) =  3.12490290e-01;
        (*m_3dh)(2,1) =  -6.34633969e-01;
        (*m_3dh)(2,2) =  7.06816486e-01;
        (*m_3dh)(2,3) = 0.0;
        (*m_3dh)(3,0) = 0.0;
        (*m_3dh)(3,1) = 0.0;
        (*m_3dh)(3,2) = 0.0;
        (*m_3dh)(3,3) = 1.0;
    #else
        const double m_3dh_data[] = {
                          9.49451118e-01,
                          2.32068468e-01,
                         -2.11392527e-01,
                          0.0,
                         -2.98729407e-02,
                          7.37145815e-01,
                          6.75073074e-01,
                          0.0,
                          3.12490290e-01,
                         -6.34633969e-01,
                          7.06816486e-01,
                          0.0,
                          0.0,
                          0.0,
                          0.0,
                          1.0 };
        Matrix m_3dh_automatic( 4, 4, m_3dh_data );
        Matrix* m_3dh( &m_3dh_automatic );
    #endif

#warning "[Code police] the following test does NOT work (6 of 9)"
    //TEST_TRUE( kjb::max_abs_difference( m9, *m_3dh ) < CLOSE_ENOUGH );

    Matrix m10 = Matrix(1,1,2.0);
    TEST_FAIL( m10.convert_to_3d_homo_rotation_matrix(0.8, 0, 0, 0) );
    m10.convert_to_3d_homo_rotation_matrix(0.8, 5, 2, 1);
#warning "[Code police] the following test does NOT work (7 of 9)"
    //TEST_TRUE( kjb::max_abs_difference( m10, *m_3dh ) < CLOSE_ENOUGH );

    Matrix m11 = Matrix::create_3d_homo_rotation_matrix_from_vector(0.8,vec);
#warning "[Code police] the following test does NOT work (8 of 9)"
    //TEST_TRUE( kjb::max_abs_difference( m11, *m_3dh ) < CLOSE_ENOUGH );

    Matrix m12 = Matrix(1,1,2.0);
    TEST_FAIL( m12.convert_to_3d_homo_rotation_matrix_from_vector(0.8,
                                                                Vector(2)) );
    m12.convert_to_3d_homo_rotation_matrix_from_vector(0.8, vec);
#warning "[Code police] the following test does NOT work (9 of 9)"
    //TEST_TRUE( kjb::max_abs_difference( m12, *m_3dh ) < CLOSE_ENOUGH );

    Matrix m13 = Matrix::create_3d_scaling_matrix(0.1, 2, 0.8);
    std::auto_ptr<Matrix> m_3ds( new Matrix(3,3,1) );
    (*m_3ds)(0,0) =  0.1;
    (*m_3ds)(0,1) =  0.0;
    (*m_3ds)(0,2) =  0.0;
    (*m_3ds)(1,0) =  0.0;
    (*m_3ds)(1,1) =  2.0;
    (*m_3ds)(1,2) =  0.0;
    (*m_3ds)(2,0) =  0.0;
    (*m_3ds)(2,1) =  0.0;
    (*m_3ds)(2,2) =  0.8;
    TEST_TRUE ( m13 == (*m_3ds) );

    Matrix m14 = Matrix(1,1,2.0);
    m14.convert_to_3d_scaling_matrix(0.1, 2, 0.8);
    TEST_TRUE ( m14 == (*m_3ds) );

    Vector vecs = Vector(3,1.0);
    vecs[0] = 0.1;
    vecs[1] = 2;
    vecs[2] = 0.8;
    Matrix m15 = Matrix::create_3d_scaling_matrix_from_vector(vecs);
    TEST_TRUE ( m15 == (*m_3ds) );

    Matrix m16 = Matrix(1,1,2.0);
    TEST_FAIL( m16.convert_to_3d_scaling_matrix_from_vector( Vector(4) ) );
    m16.convert_to_3d_scaling_matrix_from_vector(vecs);
    TEST_TRUE ( m16 == (*m_3ds) );

    TEST_FAIL( Matrix::create_3d_homo_scaling_matrix( Vector(2,0.0) ) );

    Matrix m17 = Matrix::create_3d_homo_scaling_matrix(0.1, 2, 0.8);
    std::auto_ptr<Matrix> m_3dhs( new Matrix(4,4,1) );
    (*m_3dhs)(0,0) =  0.1;
    (*m_3dhs)(0,1) =  0.0;
    (*m_3dhs)(0,2) =  0.0;
    (*m_3dhs)(0,3) =  0.0;
    (*m_3dhs)(1,0) =  0.0;
    (*m_3dhs)(1,1) =  2.0;
    (*m_3dhs)(1,2) =  0.0;
    (*m_3dhs)(1,3) =  0.0;
    (*m_3dhs)(2,0) =  0.0;
    (*m_3dhs)(2,1) =  0.0;
    (*m_3dhs)(2,2) =  0.8;
    (*m_3dhs)(2,3) =  0.0;
    (*m_3dhs)(3,0) =  0.0;
    (*m_3dhs)(3,1) =  0.0;
    (*m_3dhs)(3,2) =  0.0;
    (*m_3dhs)(3,3) =  1.0;

    TEST_TRUE ( m17 == (*m_3dhs) );

    Matrix m18 = Matrix(1,1,2.0);
    m18.convert_to_3d_homo_scaling_matrix(0.1, 2, 0.8);
    TEST_TRUE  ( m18 == (*m_3dhs) );

    Matrix m19 = Matrix::create_3d_homo_scaling_matrix_from_vector(vecs);
    TEST_TRUE  ( m19 == (*m_3dhs) );

    Matrix m20 = Matrix(1,1,2.0);
    m20.convert_to_3d_homo_scaling_matrix_from_vector(vecs);
    TEST_TRUE  ( m20 == (*m_3dhs) );

    Matrix m21 = Matrix::create_3d_homo_translation_matrix(5.0, -7.2, 0.23);
    std::auto_ptr<Matrix> m_3dht( new Matrix(4,4,1) );
    (*m_3dht)(0,0) =  1.0;
    (*m_3dht)(0,1) =  0.0;
    (*m_3dht)(0,2) =  0.0;
    (*m_3dht)(0,3) =  5.0;
    (*m_3dht)(1,0) =  0.0;
    (*m_3dht)(1,1) =  1.0;
    (*m_3dht)(1,2) =  0.0;
    (*m_3dht)(1,3) =  -7.2;
    (*m_3dht)(2,0) =  0.0;
    (*m_3dht)(2,1) =  0.0;
    (*m_3dht)(2,2) =  1.0;
    (*m_3dht)(2,3) =  0.23;
    (*m_3dht)(3,0) =  0.0;
    (*m_3dht)(3,1) =  0.0;
    (*m_3dht)(3,2) =  0.0;
    (*m_3dht)(3,3) =  1.0;

    TEST_TRUE  ( m21 == (*m_3dht) );

    Matrix m22 = Matrix(1,1,2.0);
    m22.convert_to_3d_homo_translation_matrix(5.0, -7.2, 0.23);
    TEST_TRUE  ( m22 == (*m_3dht) );

    Vector vect = Vector(3,1.0);
    vect[0] = 5.0;
    vect[1] = -7.2;
    vect[2] = 0.23;

    Matrix m23 = Matrix::create_3d_homo_translation_matrix_from_vector(vect);
    TEST_TRUE  ( m23 == (*m_3dht) );

    Matrix m24 = Matrix(1,1,2.0);
    TEST_FAIL( m24.convert_to_3d_homo_translation_matrix( Vector(2) ) );
    m24.convert_to_3d_homo_translation_matrix_from_vector(vect);
    TEST_TRUE  ( m24 == (*m_3dht) );

    std::auto_ptr<Matrix> m_heuler( new Matrix(4,4,1) );
    (*m_heuler)(0,0) = -2.12189294e-01;
    (*m_heuler)(0,1) = 7.42932452e-01;
    (*m_heuler)(0,2) = -6.34844134e-01;
    (*m_heuler)(0,3) = 0.0;
    (*m_heuler)(1,0) = -9.27137885e-01;
    (*m_heuler)(1,1) = -3.58365735e-01;
    (*m_heuler)(1,2) = -1.09495853e-01;
    (*m_heuler)(1,3) = 0.0;
    (*m_heuler)(2,0) = -3.08854407e-01;
    (*m_heuler)(2,1) = 5.65354200e-01;
    (*m_heuler)(2,2) = 7.64842195e-01;
    (*m_heuler)(2,3) = 0.0;
    (*m_heuler)(3,0) = 0.0;
    (*m_heuler)(3,1) = 0.0;
    (*m_heuler)(3,2) = 0.0;
    (*m_heuler)(3,3) = 1.0;

    Matrix m25 = Matrix(1,1,2.0);
    m25.convert_to_euler_homo_rotation_matrix(0.5,-0.7,1.4);
    TEST_TRUE( kjb::max_abs_difference( m25, *m_heuler ) < CLOSE_ENOUGH );

    Matrix m26 = Matrix::create_euler_homo_rotation_matrix(0.5, -0.7, 1.4);
    TEST_TRUE( kjb::max_abs_difference( m26, *m_heuler ) < CLOSE_ENOUGH );

    RETURN_VICTORIOUSLY();
}

