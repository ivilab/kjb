/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2009-2010, by members of University of Arizona Computer    |
 | Vision group (the authors) including                                     |
 |       Kyle Simek, Andrew Predoehl                                        |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: test_matrix.cpp 20369 2016-02-14 18:17:27Z jguan1 $ */

#include "l/l_sys_io.h"
#include "m/m_matrix.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_int_matrix.h"
#include "m_cpp/m_mat_util.h"
#include "m_cpp/m_mat_view.h"
#include "l_cpp/l_exception.h"
#include "l_cpp/l_test.h"
#include <l_cpp/l_stdio_wrap.h>
#include <l_cpp/l_serialization.h>

#include <cmath>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <vector>
#include <list>

#ifdef KJB_HAVE_BST_SERIAL
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif

//#define NOW_I_AM_HERE() std::cout << "At line " << __LINE__ << '\n'

// **** UTILITY FUNCTIONS ****
int rand_int() { return kjb_c::kjb_rand() * INT_MAX - INT_MAX/2; }

template <class T>
T random_matrix(int r, int c)
{}

template <>
inline kjb::Matrix random_matrix<kjb::Matrix>(int r, int c)
{
    return kjb::create_random_matrix(r, c);
}

template <>
inline kjb::Int_matrix random_matrix<kjb::Int_matrix>(int r, int c)
{
    kjb::Int_matrix m(r,c);
    std::generate(&m(0,0), &m(0,0) + m.get_length(), rand_int);
    return m;
}


// **** TEST FUNCTIONS ****

// Reference function for zxz euler rotations, taken from Alternaria project code.
void get_euler_matrix_zxz_reference(kjb_c::Matrix** m_out, float phi, float theta, float psi)
{
    using std::cos;
    using std::sin;

    float cos_phi, sin_phi;
    float cos_theta, sin_theta;
    float cos_psi, sin_psi;

    cos_phi   = (float)cos(phi);
    sin_phi   = (float)sin(phi);
    cos_theta = (float)cos(theta);
    sin_theta = (float)sin(theta);
    cos_psi   = (float)cos(psi);
    sin_psi   = (float)sin(psi);

    kjb_c::get_target_matrix(m_out, 3, 3);
    (*m_out)->elements[0][0] = cos_psi*cos_phi - cos_theta*sin_phi*sin_psi;
    (*m_out)->elements[0][1] = cos_psi*sin_phi + cos_theta*cos_phi*sin_psi;
    (*m_out)->elements[0][2] = sin_psi*sin_theta;
    (*m_out)->elements[1][0] = -sin_psi*cos_phi - cos_theta*sin_phi*cos_psi;
    (*m_out)->elements[1][1] = -sin_psi*sin_phi + cos_theta*cos_phi*cos_psi;
    (*m_out)->elements[1][2] = cos_psi*sin_theta;
    (*m_out)->elements[2][0] = sin_theta*sin_phi;
    (*m_out)->elements[2][1] = -sin_theta*cos_phi;
    (*m_out)->elements[2][2] = cos_theta;
}

template <class T>
void test_resize(){
    std::cerr << "Error: this function should never be called" << std::endl;
    abort();
}

template <>
void test_resize<kjb::Int_matrix>()
{
    // Not implemented
}

template <>
void test_resize<kjb::Matrix>()
{
    typedef kjb::Matrix MAT_T;

    // is resize reusing storage?
    MAT_T test_1 = random_matrix<MAT_T>(6,6);
    const MAT_T::Impl_type* old_ptr = test_1.get_c_matrix();
    test_1.resize(5,6);
    TEST_TRUE(old_ptr == test_1.get_c_matrix());
    // re-increase size
    test_1.resize(6,6); // back up to old size
    TEST_TRUE(old_ptr == test_1.get_c_matrix());
    // fewer columns
    test_1.resize(6,1); // fewer number of columns
    TEST_TRUE(old_ptr == test_1.get_c_matrix());
    // fewer columns
    test_1.resize(36,1); // reshape
    TEST_TRUE(old_ptr == test_1.get_c_matrix());
    test_1.resize(1,36); // reshape
    TEST_TRUE(old_ptr == test_1.get_c_matrix());
    // finally, we must reallocate:
    test_1.resize(1,100); 
    TEST_TRUE(old_ptr != test_1.get_c_matrix());


    test_1.realloc(9, 10);
    TEST_TRUE(test_1.get_num_rows() == 9);
    TEST_TRUE(test_1.get_num_cols() == 10);

    // random testing of resize
    const size_t MAX_DIM = 200;
    const size_t MAX_IT = 500;
    
    for(size_t i = 0; i < MAX_IT; ++i)
    {
        MAT_T::Value_type padding = INT_MAX * kjb_c::kjb_rand();
        int old_rows = kjb_c::kjb_rand() * MAX_DIM + 1;
        int old_cols = kjb_c::kjb_rand() * MAX_DIM + 1;
        int new_rows = kjb_c::kjb_rand() * MAX_DIM + 1;
        int new_cols = kjb_c::kjb_rand() * MAX_DIM + 1;
        int min_rows = std::min(old_rows, new_rows);
        int min_cols = std::min(old_cols, new_cols);

        test_1 = random_matrix<MAT_T>(old_rows, old_cols);
        MAT_T test_2 = test_1;
        test_2.resize(new_rows, new_cols, padding);

        for(size_t r = 0; r < min_rows; ++r)
        {
            for(size_t c = 0; c < min_cols; ++c)
                TEST_TRUE(test_2.at(r,c) == test_1.at(r,c));
            for(size_t c = min_cols; c < new_cols; ++c)
                TEST_TRUE(test_2.at(r,c) == padding);
        }
        for(size_t r = min_rows; r < new_rows; ++r)
        {
            for(size_t c = 0; c < new_cols; ++c)
                TEST_TRUE(test_2.at(r,c) == padding);
        }
    }
    
    // Test resizing the same matrix multiple times
    for(size_t i = 0; i < MAX_IT; ++i)
    {
        MAT_T::Value_type padding = INT_MAX * kjb_c::kjb_rand();
        int nrows = kjb_c::kjb_rand() * MAX_DIM + 1;
        int dim1 = kjb_c::kjb_rand() * MAX_DIM + 1;
        int dim2 = kjb_c::kjb_rand() * MAX_DIM + 1;
        int dim3 = kjb_c::kjb_rand() * MAX_DIM + 1;
        if(dim1 > dim2)
            std::swap(dim1, dim2);
        if(dim2 > dim3)
            std::swap(dim2, dim3);
        if(dim1 > dim2)
            std::swap(dim1, dim2);

        { // resize columns only
            test_1 = random_matrix<MAT_T>(nrows, dim3);
            MAT_T test_2 = test_1;
            test_2.resize(nrows, dim1, padding);
            test_2.resize(nrows, dim2, padding);

            for(size_t r = 0; r < nrows; ++r)
            {
                for(size_t c = 0; c < dim1; ++c)
                    TEST_TRUE(test_2.at(r,c) == test_1.at(r,c));
                for(size_t c = dim1; c < dim2; ++c)
                    TEST_TRUE(test_2.at(r,c) == padding);
            }
        }
        { // resize rows only
            int ncols = nrows;
            test_1 = random_matrix<MAT_T>(dim3, ncols);
            MAT_T test_2 = test_1;
            test_2.resize(dim1, ncols, padding);
            test_2.resize(dim2, ncols, padding);

            for(size_t c = 0; c < ncols; ++c)
            {
                for(size_t r = 0; r < dim1; ++r)
                    TEST_TRUE(test_2.at(r,c) == test_1.at(r,c));
                for(size_t r = dim1; r < dim2; ++r)
                    TEST_TRUE(test_2.at(r,c) == padding);
            }
        }
        { // resize rows and columns only
            test_1 = random_matrix<MAT_T>(dim3, dim3);
            MAT_T test_2 = test_1;
            test_2.resize(dim1, dim1, padding);
            test_2.resize(dim2, dim2, padding);

            for(size_t r = 0; r < dim1; ++r)
            {
                for(size_t c = 0; c < dim1; ++c)
                    TEST_TRUE(test_2.at(r,c) == test_1.at(r,c));
                for(size_t c = dim1; c < dim2; ++c)
                    TEST_TRUE(test_2.at(r,c) == padding);
            }
            for(size_t r = dim1; r < dim2; ++r)
            {
                for(size_t c = 0; c < dim2; ++c)
                    TEST_TRUE(test_2.at(r,c) == padding);
            }
        }
    }

    // test a specific failure case Colin found:
    // resize to (nrow,ncol+1), delete column i, resize to (nrow,ncol+1)
    for(size_t i = 0; i < MAX_IT; ++i)
    {
        MAT_T::Value_type padding = INT_MAX * kjb_c::kjb_rand();
        int nrows = kjb_c::kjb_rand() * MAX_DIM + 1;
        int ncols = kjb_c::kjb_rand() * MAX_DIM + 1;
        int delete_i = kjb_c::kjb_rand() * MAX_DIM + 1;
        if(delete_i > ncols)
            std::swap(delete_i, ncols);
        if(delete_i == ncols)
            delete_i -= 1;

        test_1 = random_matrix<MAT_T>(nrows, ncols);
        MAT_T test_2 = test_1;
        // add right column with zeros
        test_2.resize(test_2.get_num_rows(), test_2.get_num_cols()+1, padding);
        TEST_TRUE(test_2.get_num_cols() == ncols + 1);

        // delete random column
        test_2.remove_column(delete_i);
        TEST_TRUE(test_2.get_num_cols() == ncols);

        // add right column with zeros
        test_2.resize(test_2.get_num_rows(), test_2.get_num_cols()+1, padding);
        TEST_TRUE(test_2.get_num_cols() == ncols + 1);

        for(size_t r = 0; r < nrows; ++r)
        {
            for(size_t c = 0; c < delete_i; ++c)
                TEST_TRUE(test_2.at(r,c) == test_1.at(r,c));
            for(size_t c = delete_i; c < ncols-1; ++c)
                TEST_TRUE(test_2.at(r,c) == test_1.at(r,c+1));
            for(size_t c = ncols-1; c < ncols; ++c)
                TEST_TRUE(test_2.at(r,c) == padding);
        }
    }
}

template< typename MAT_T >
void test_ctors_1()
{
    // default constructor
    MAT_T* test_1 = new MAT_T();
    TEST_TRUE( 0 == test_1 -> get_num_rows() );
    TEST_TRUE( 0 == test_1 -> get_num_cols() );
    TEST_FAIL(test_1->at(0,0));
    TEST_FAIL(test_1->at(1,1));
    TEST_FAIL(test_1 -> at(0) );
    TEST_FAIL(test_1 -> at(1) );
    delete test_1;

    // uninitialized constructor
    test_1 = new MAT_T(5, 5);
    TEST_TRUE(test_1->get_num_rows() == 5);
    TEST_TRUE(test_1->get_num_cols() == 5);
    TEST_TRUE(test_1->get_length() == 5 * 5);

    TEST_SUCCESS( test_1->at(0,0) );
    TEST_SUCCESS( test_1->at(0,0) = 11); // test return-by-reference version
    TEST_FAIL(    test_1->at(5,0) );
    TEST_FAIL(    test_1->at(0,5) );
    TEST_FAIL(    test_1->at(0,5) = 11);
    TEST_SUCCESS( test_1->at(24) );
    TEST_SUCCESS( test_1->at(24) = 11);
    TEST_FAIL(    test_1->at(25) );
    TEST_FAIL(    test_1->at(100) );
    TEST_FAIL(    test_1->at(25) = 11 );
    TEST_FAIL(    test_1->at(100) = 11 );

    test_1->resize(6,6);
    TEST_SUCCESS( test_1->at(5,0) );
    TEST_SUCCESS( test_1->at(0,5) );
    TEST_TRUE( 11 == test_1 -> at(0,0) );   // did resize preserve prev. vals?
    TEST_TRUE( 11 == test_1 -> at(4,4) );

    delete test_1;

}



template< typename MAT_T, typename UINT_T, typename VALUE_T >
void test_u_ctor( const MAT_T& ref, VALUE_T val )
{
    // ctors that use the unsigned integers 
    MAT_T test_u( UINT_T(3), UINT_T(3) );
    for( int i = 0; i < 3*3; ++i )
        test_u[i] = ref[i];
    TEST_TRUE( test_u == ref );
    TEST_FAIL( test_u.at(0,4) );
    TEST_FAIL( test_u.at(4,0) );
    TEST_FAIL( test_u.at(9) );
    MAT_T test_u_v( UINT_T(3), UINT_T(3), val );
    TEST_TRUE( test_u_v == ref );
}


template < typename MAT_T, typename VALUE_T >
void test_max_abs( VALUE_T v1, VALUE_T v2 )
{
    // test max_abs_difference method
    MAT_T mat1( 10, 10, VALUE_T(0) );
    MAT_T mat2( 10, 10, VALUE_T(0) );
    MAT_T mat3( 10, 10, VALUE_T(1) );
    MAT_T mat4( 10, 5, VALUE_T(0) );
    MAT_T mat5( 5, 10, VALUE_T(0) );

    TEST_FAIL( kjb::max_abs_difference( mat1, mat4 ) );
    TEST_FAIL( kjb::max_abs_difference( mat1, mat5 ) );
    TEST_TRUE( 0 == kjb::max_abs_difference( mat1, mat1 ) );
    TEST_TRUE( 0 == kjb::max_abs_difference( mat1, mat2 ) );
    TEST_TRUE( VALUE_T(1) == kjb::max_abs_difference( mat1, mat3 ) );

    mat1( 4, 8 ) = v1;
    mat2( 7, 1 ) = v2;
    TEST_TRUE( MAX_OF(ABS_OF(v1),ABS_OF(v2)) == kjb::max_abs_difference(mat1, mat2));
    mat2( 7, 1 ) = 0;
    mat2( 4, 8 ) = v2;
    TEST_TRUE( ABS_OF(v1-v2) == kjb::max_abs_difference( mat1, mat2 ) );
}


void test_some_int_mat_properties()
{
    /* Integer matrix construction, row major ordering, min, max,
     * arithmetic (multiplication, addition, subtraction, division, negation)
     * both in-place and returning a new value;
     * get_row, get_col, output to stream,
     * init_identity
     */
    const int foodat[] = {-481, 1167, -436, 555, -741, 324, -7474, 7432, 7400};
    kjb::Int_matrix foo( 3, 3, foodat );
    const kjb::Int_matrix FOO( foo );

    const int* foop = foodat;
    int val, index = 0;
    val = *foop++; TEST_TRUE( FOO.at(index) == val ); index++; // ie tst FOO(0)
    val = *foop++; TEST_TRUE( FOO.at(index) == val ); index++;
    val = *foop++; TEST_TRUE( FOO.at(index) == val ); index++;
    val = *foop++; TEST_TRUE( FOO.at(index) == val ); index++;
    val = *foop++; TEST_TRUE( FOO.at(index) == val ); index++;
    val = *foop++; TEST_TRUE( FOO.at(index) == val ); index++;
    val = *foop++; TEST_TRUE( FOO.at(index) == val ); index++;
    val = *foop++; TEST_TRUE( FOO.at(index) == val ); index++;
    val = *foop++; TEST_TRUE( FOO.at(index) == val ); index++; // ninth time

    foop = foodat;
    int row = 0, col = 0;
    val = *foop++; TEST_TRUE(FOO.at(row, col) == val); row+=((col+=1)%=3)?0:1;
    val = *foop++; TEST_TRUE(FOO.at(row, col) == val); row+=((col+=1)%=3)?0:1;
    val = *foop++; TEST_TRUE(FOO.at(row, col) == val); row+=((col+=1)%=3)?0:1;
    val = *foop++; TEST_TRUE(FOO.at(row, col) == val); row+=((col+=1)%=3)?0:1;
    val = *foop++; TEST_TRUE(FOO.at(row, col) == val); row+=((col+=1)%=3)?0:1;
    val = *foop++; TEST_TRUE(FOO.at(row, col) == val); row+=((col+=1)%=3)?0:1;
    val = *foop++; TEST_TRUE(FOO.at(row, col) == val); row+=((col+=1)%=3)?0:1;
    val = *foop++; TEST_TRUE(FOO.at(row, col) == val); row+=((col+=1)%=3)?0:1;
    val = *foop++; TEST_TRUE(FOO.at(row, col) == val); row+=((col+=1)%=3)?0:1;

    TEST_TRUE( min(foo) == -7474 );
    TEST_TRUE( max(foo) == 7432 );
    TEST_TRUE( foo == FOO );
    const kjb::Int_matrix foosix = 6 * foo;
    TEST_FALSE( foosix == FOO );
    const int sixdat[] = { -2886,  7002, -2616,
                            3330, -4446,  1944,
                          -44844, 44592, 44400 };

    TEST_TRUE( foosix == kjb::Int_matrix( 3, 3, sixdat ) );
    TEST_TRUE( FOO == kjb::Int_matrix( 3, 3, foodat ) );
    TEST_TRUE( foosix.get_row(1) == kjb::Int_vector( 3, 3+sixdat ) );

    kjb::Int_vector col_2nd = foosix.get_col(1);
    TEST_TRUE( 3 == col_2nd.get_length() );
    TEST_TRUE(  7002 == col_2nd.at(0) );
    TEST_TRUE( -4446 == col_2nd.at(1) );
    TEST_TRUE( 44592 == col_2nd.at(2) );

    kjb::Int_matrix another_foo6( foo ), and_another;
    another_foo6.multiply( 6 );
    TEST_TRUE( foosix == another_foo6 );

    and_another = foo;
    and_another.add( foo );
    and_another += foo + foo;
    and_another -= foo;
    and_another += 4 * foo;
    and_another.subtract( foo );
    TEST_TRUE( and_another == foosix );
    TEST_TRUE( -foo * 5 == foo - foosix );
    TEST_SUCCESS( and_another = and_another );
    TEST_TRUE( and_another == foosix );
    TEST_TRUE( and_another.negate() == -foosix );
    TEST_TRUE( and_another == -foosix );

    const int bazdat[] = { -7, 18, -6, 8, -11, 4, -115, 114, 114 };
    const kjb::Int_matrix baz_1 = foosix / 389;
    TEST_TRUE( baz_1 == kjb::Int_matrix( 3, 3, bazdat ) );

    kjb::Int_matrix baz_2( foosix );
    baz_2.divide( 389 );
    TEST_TRUE( baz_2 == baz_1 );

    baz_2 = foosix;
    TEST_FAIL( baz_2 /= 0 );


    std::ostringstream so;
    so << baz_1;
    TEST_TRUE( so.str() ==
                    "              -7              18              -6\n"
                    "               8             -11               4\n"
                    "            -115             114             114\n"
                );

    kjb::Int_matrix eye( kjb::create_identity_int_matrix(3) );
    TEST_TRUE( eye * foosix == foosix );
    TEST_TRUE( foosix * eye == foosix );

    kjb::Int_matrix zer0( kjb::create_zero_int_matrix(3) );
    TEST_TRUE( zer0 + foosix == foosix );
    TEST_TRUE( foosix + zer0 == foosix );
    TEST_TRUE( baz_2.zero_out() + foosix == foosix );
    TEST_FAIL( baz_2.zero_out(4) + foosix == foosix );
    TEST_FAIL( baz_2.zero_out() + foosix == foosix );   // baz_2 still 4x4
    TEST_TRUE( baz_2.zero_out(3) + foosix == foosix );
    TEST_FAIL( baz_2.zero_out(2,7) + foosix == foosix );
    TEST_TRUE( baz_2.zero_out(3,3) + foosix == foosix );
    TEST_TRUE( foosix + baz_2 == foosix );

    TEST_TRUE( kjb::create_identity_int_matrix(3).multiply(6).multiply(FOO) == foosix );
}


template< typename MAT_T >
void test_file_io( const MAT_T& mat )
{
    kjb::Temporary_File tf;
    int rc2 = mat.write( tf.get_filename().c_str() );
    TEST_TRUE( rc2 == kjb_c::NO_ERROR );
    MAT_T approx2( tf.get_filename() );
    TEST_TRUE( mat == approx2 );
    MAT_T fail;
//    TEST_FAIL( fail = MAT_T( (const char*)0 ) );
}

template< typename MAT_T >
void test_serialization( const MAT_T& mat )
{
#ifdef KJB_HAVE_BST_SERIAL
    
    kjb::Temporary_File tf;

    MAT_T new_mat;

    {
        std::ofstream ofs( tf.get_filename().c_str() );
        boost::archive::text_oarchive oa(ofs);
        oa << mat;
    }

    {
        std::ifstream ifs( tf.get_filename().c_str() );
        boost::archive::text_iarchive ia(ifs);
        ia >> new_mat;
    }

    TEST_TRUE( max_abs_difference(mat, new_mat) <= DBL_EPSILON );
#else
    std::cerr << "Library boost::serialization not found so serialization was "
        "not tested." << std::endl;
#endif
}


template< typename MAT_T >
void test_ctors_rc()
{
    typedef typename MAT_T::Vec_type VEC_T;
    typedef typename MAT_T::Value_type VAL_T;

    // construct matrix from a set of rows, cols
    const VAL_T  dr1[] = { 0, 1, 2 },
                 dr2[] = { 3, 4, 5 },
                 dr3[] = { 6, 7, 8 };
    const VEC_T row1(3, dr1), row2(3, dr2), row3(3, dr3);

    std::vector<VEC_T> row123;
    row123.push_back( row1 );
    row123.push_back( row2 );
    row123.push_back( row3 );

    MAT_T row_matrix = kjb::create_matrix_from_rows( row123 );

    TEST_TRUE(row_matrix(0, 0) == 0 &&  row_matrix(0, 1) == 1 &&  row_matrix(0, 2) == 2 && 
            row_matrix(1, 0) == 3   &&  row_matrix(1, 1) == 4 &&  row_matrix(1, 2) == 5 && 
            row_matrix(2, 0) == 6   &&  row_matrix(2, 1) == 7 &&  row_matrix(2, 2) == 8);

    MAT_T col_matrix = kjb::create_matrix_from_columns( row123 );

    TEST_TRUE(col_matrix(0, 0) == 0 &&  col_matrix(0, 1) == 3 &&  col_matrix(0, 2) == 6 && 
            col_matrix(1, 0) == 1   &&  col_matrix(1, 1) == 4 &&  col_matrix(1, 2) == 7 && 
            col_matrix(2, 0) == 2   &&  col_matrix(2, 1) == 5 &&  col_matrix(2, 2) == 8);

    // test with a container of stl-style vectors
    std::list<std::vector<VAL_T> > stl_row_123;
    std::vector<VAL_T> stl_row1(dr1, dr1 + 3);
    std::vector<VAL_T> stl_row2(dr2, dr2 + 3);
    std::vector<VAL_T> stl_row3(dr3, dr3 + 3);
    stl_row_123.push_back(stl_row1);
    stl_row_123.push_back(stl_row2);
    stl_row_123.push_back(stl_row3);

    MAT_T stl_row_matrix = kjb::create_matrix_from_rows(stl_row_123);

    TEST_TRUE(stl_row_matrix(0, 0) == 0 &&  stl_row_matrix(0, 1) == 1 &&  stl_row_matrix(0, 2) == 2 && 
            stl_row_matrix(1, 0) == 3   &&  stl_row_matrix(1, 1) == 4 &&  stl_row_matrix(1, 2) == 5 && 
            stl_row_matrix(2, 0) == 6   &&  stl_row_matrix(2, 1) == 7 &&  stl_row_matrix(2, 2) == 8);

    MAT_T stl_col_matrix = kjb::create_matrix_from_columns(stl_row_123);

    TEST_TRUE(stl_col_matrix(0, 0) == 0 &&  stl_col_matrix(0, 1) == 3 &&  stl_col_matrix(0, 2) == 6 && 
            stl_col_matrix(1, 0) == 1   &&  stl_col_matrix(1, 1) == 4 &&  stl_col_matrix(1, 2) == 7 && 
            stl_col_matrix(2, 0) == 2   &&  stl_col_matrix(2, 1) == 5 &&  stl_col_matrix(2, 2) == 8);


}


template< typename MAT_T >
void test_set_row()
{
    typedef typename MAT_T::Vec_type VEC_T;
    typedef typename MAT_T::Value_type VAL_T;

    MAT_T ref( 3, 3 );
    for( int iii = 0; iii < 9; ++iii )
    {
        ref.at( iii ) = VAL_T( 1 + iii );
    }

    // set_row, set_col
    VEC_T row1( ref.get_row( 1 ) ), col1( ref.get_col( 1 ) );
    TEST_TRUE( 3 == row1.get_length() );
    TEST_TRUE( 3 == col1.get_length() );
    TEST_TRUE( VAL_T(4) == row1[0] );
    TEST_TRUE( VAL_T(5) == row1[1] );
    TEST_TRUE( VAL_T(6) == row1[2] );
    TEST_TRUE( VAL_T(2) == col1[0] );
    TEST_TRUE( VAL_T(5) == col1[1] );
    TEST_TRUE( VAL_T(8) == col1[2] );

    MAT_T set_test(3,3, VAL_T(0) );
    set_test.set_row(1, row1);

    TEST_TRUE( VAL_T(4) == set_test(1,0) );
    TEST_TRUE( VAL_T(5) == set_test(1,1) );
    TEST_TRUE( VAL_T(6) == set_test(1,2) );

    set_test.set_col(1, col1);
    TEST_TRUE( VAL_T(2) == set_test(0,1) );
    TEST_TRUE( VAL_T(5) == set_test(1,1) );
    TEST_TRUE( VAL_T(8) == set_test(2,1) );
}


int exp2( int n )
{
    return 0 <= n ? 1 << n : 0;
}

void test_mapcar()
{
    const double vals[]={ -2, -1, 0, 1, 2, 3, };
    const double expvals[]={ 1/M_E/M_E, 1/M_E, 1, M_E, M_E*M_E, M_E*M_E*M_E };
    kjb::Matrix m(2,3,vals);
    kjb::Vector v(6,vals);
    const kjb::Matrix em(2,3,expvals);
    const kjb::Vector ev(6,expvals);
    TEST_TRUE( kjb::max_abs_difference( em, m.mapcar( exp ) ) < 1e-10 );
    TEST_TRUE( kjb::max_abs_difference( ev, v.mapcar( exp ) ) < 1e-10 );

    const int vals2[] = { -2, -1, 0, 1, 2, 3 };
    const int exp2vals[] = { 0, 0, 1, 2, 4, 8 };
    kjb::Int_matrix m2(2,3,vals2);
    kjb::Int_vector v2(6,vals2);
    const kjb::Int_matrix em2(2,3,exp2vals);
    const kjb::Int_vector ev2(6,exp2vals);
    TEST_TRUE( em2 == m2.mapcar( exp2 ) );
    TEST_TRUE( ev2 == v2.mapcar( exp2 ) );
}

void test_serialization()
{
    using namespace kjb;
    const char* fname = "tmp_matrix.txt";
    Matrix m = create_random_matrix(100, 100);

    // save/load will test the generic kjb::serialize template function
    save(m, fname);
    Matrix m2;
    load(m2, fname);

    kjb_c::kjb_unlink(fname);

    TEST_TRUE(max_abs_difference(m, m2) < FLT_EPSILON);

}

void test_stl_view()
{
    using namespace kjb;
    Matrix mat = create_random_matrix(100, 200);
    Matrix_stl_view view = get_matrix_stl_view(mat);

    for(int row = 0; row < mat.get_num_rows(); row++)
    for(int col = 0; col < mat.get_num_cols(); col++)
    {
        TEST_TRUE(view[row][col] == mat(row, col));
    }

    view[50][20] = 123123;
    TEST_TRUE(mat(50, 20) == 123123);
}

void test_stream_io()
{
    using namespace kjb;
    const char* fname = "tmp_matrix.txt";
    Matrix m1 = create_random_matrix(100, 100);

    // save/load will test the generic kjb::serialize template function
    std::ofstream ofs(fname);
    stream_write_matrix(ofs, m1);
    Matrix m2 = create_random_matrix(100, 100);
    stream_write_matrix(ofs, m2);
    Matrix m3;
    std::ifstream ifs(fname);
    stream_read_matrix(ifs, m3);
    Matrix m4;
    stream_read_matrix(ifs, m4);

    kjb_c::kjb_unlink(fname);

    TEST_TRUE(max_abs_difference(m1, m3) < FLT_EPSILON);
    TEST_TRUE(max_abs_difference(m2, m4) < FLT_EPSILON);
}

int main(int /* argc */, char ** /* argv */)
{
    using kjb::Matrix;
    using kjb::Vector;
    using kjb::create_zero_matrix;

    try {
    Matrix test_2;
    kjb::Int_matrix itest_2;
    Matrix* ref_1;
    Matrix ref_2;

    test_ctors_1< Matrix >();
    test_ctors_1< kjb::Int_matrix >();

    test_resize<Matrix>();
    test_resize<kjb::Int_matrix>();

    const double count_matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };

    const double pi_matrix[3][3] = {
        {M_PI, M_PI, M_PI},
        {M_PI, M_PI, M_PI},
        {M_PI, M_PI, M_PI}
    };


    // initializing constructor
    test_2 = Matrix(3, 3, M_PI);

    kjb_c::Matrix* nassty( 0 );
    kjb_c::get_initialized_matrix( &nassty, 3, 3, M_PI );
    TEST_TRUE( *nassty == test_2 );
    TEST_FALSE( *nassty == Matrix() );
    TEST_FALSE( *nassty != test_2 );
    TEST_TRUE( *nassty != Matrix() );
    kjb_c::free_matrix( nassty );

    // same as above but now using Int_matrix
    { using kjb::Int_matrix;
    kjb_c::Int_matrix *inassty( 0 );
    kjb_c::get_initialized_int_matrix( &inassty, 3, 3, 741 );
    Int_matrix widlar = Int_matrix( 3, 3, 702 ) + Int_matrix( 3, 3, 39 );
    TEST_TRUE( *inassty == widlar );
    TEST_FALSE( *inassty == Int_matrix() );
    TEST_FALSE( *inassty != widlar );
    TEST_TRUE( *inassty != Int_matrix() );
    kjb::Int_matrix deepinassty( *inassty );
    TEST_TRUE( *inassty == deepinassty );
    ++deepinassty.at(5);
    TEST_FALSE( *inassty == deepinassty );
    kjb_c::free_int_matrix( inassty );

    TEST_TRUE( widlar == widlar );      // identical objects (ints)
    TEST_FALSE( widlar != widlar );     // identical objects (ints)
    }

    // test the int-to-double conversion ctor
    {
        kjb::Int_matrix im(5,6);
        Matrix ref_m(5,6), test_m(5,6);

        for( int iii = 0; iii < 30; ++iii )
        {
            im.at(iii) = 17+iii;
            ref_m.at(iii) = double( 17+iii );
        }

        test_m = im;

        TEST_TRUE( test_m == ref_m );
    }

    test_some_int_mat_properties();

    TEST_TRUE( test_2[0]      == M_PI );
    TEST_TRUE( test_2(0)      == M_PI );
    TEST_TRUE( test_2(0, 0)   == M_PI );
    TEST_TRUE( test_2(2, 1)   == M_PI );

    // Matrix elementwise equality
    TEST_TRUE(  test_2 == test_2  );    // identical objects (doubles)
    TEST_FALSE( test_2 == Matrix(6,43,M_PI) );

    // test the ctor for unsigned, unsigned long
    test_u_ctor <Matrix, unsigned, double> ( test_2, M_PI );
    test_u_ctor <Matrix, unsigned long, double> ( test_2, M_PI );

    kjb::Int_matrix test_2int( 3, 3, -17 );
    test_u_ctor <kjb::Int_matrix, unsigned, int> ( test_2int, -17 );
    test_u_ctor <kjb::Int_matrix, unsigned long, int> ( test_2int, -17 );

    // constructor: double[]
    ref_1 = new Matrix(3, 3, &pi_matrix[0][0]);
    TEST_TRUE(  (*ref_1) == test_2  );
    TEST_FALSE( (*ref_1) != test_2 );

    (*ref_1)(0,1) = 2;
    TEST_TRUE(  *ref_1 != test_2);
    TEST_FALSE( *ref_1 == test_2);

    test_2(1) = 2;
    TEST_TRUE(   *ref_1 == test_2);
    TEST_FALSE(  *ref_1 != test_2);

    ref_2 = Matrix(3, 3, &count_matrix[0][0]);
    TEST_TRUE(   ref_2 != test_2  );
    TEST_TRUE( !(ref_2 == test_2) );

    // converting constructor: Matrix
    kjb_c::Matrix* src_matrix2 = NULL;
    kjb_c::get_target_matrix(&src_matrix2, 3, 3);
    src_matrix2->elements[0][0] = 1.;
    src_matrix2->elements[0][1] = 2.;
    src_matrix2->elements[0][2] = 3.;
    src_matrix2->elements[1][0] = 4.;
    src_matrix2->elements[1][1] = 5.;
    src_matrix2->elements[1][2] = 6.;
    src_matrix2->elements[2][0] = 7.;
    src_matrix2->elements[2][1] = 8.;
    src_matrix2->elements[2][2] = 9.;

    Matrix* test_1 = new Matrix(*src_matrix2);
    test_2 = Matrix(3, 3, &count_matrix[0][0]);

    TEST_TRUE( test_2 == *src_matrix2 );
    TEST_TRUE( *test_1 == test_2 );

    test_2 = *src_matrix2;
    TEST_TRUE( *test_1 == test_2 );

    kjb_c::free_matrix(src_matrix2);
    delete test_1;

    // test copy constructor
    test_1 = new Matrix(test_2);
    TEST_TRUE( *test_1 == test_2);

    {
        kjb_c::Matrix *not_really_a_leak = 0;
        Matrix gc1( not_really_a_leak );    // if nil we ignore the argument
        TEST_TRUE( 0 == not_really_a_leak );
        kjb_c::get_target_matrix( &not_really_a_leak, 1000, 1000 );
        Matrix gc2( not_really_a_leak );
        // can't check this well, but notice that the test program does
        // not leak memory -- the program terminates without scolding you.

        kjb_c::Int_matrix *not_really_a_leak_2 = 0;
        kjb::Int_matrix gc3( not_really_a_leak_2 ); // if nil we ignore the arg
        kjb_c::get_target_int_matrix( &not_really_a_leak_2, 1000, 1000 );
        kjb::Int_matrix gc4( not_really_a_leak_2 );
        // ditto -- program ends without scolding you.

        /*
         * To verify the efficacy of this test, comment out the gc2 or gc4
         * def. line, and you'll see the test program "scolds" you at exit
         * about "Residual memory allocation," i.e., un-freed heap memory.
         * (Then restore those lines, please!)
         */
    }

    // constructor for row and column matrices
    {
        const int MTSZ = 4;
        double monkey_typist[MTSZ] = { 29.65, 564.563, -521.707, -1e-33 };
        Vector victor( MTSZ, monkey_typist );
        Matrix rowvic( kjb::create_row_matrix( victor ) );
        Matrix colvic( kjb::create_column_matrix( victor ) );
        TEST_TRUE( MTSZ == colvic.get_num_rows() );
        TEST_TRUE( MTSZ == rowvic.get_num_cols() );
        TEST_TRUE( 1 == rowvic.get_num_rows() );
        TEST_TRUE( 1 == colvic.get_num_cols() );
        for( int iii = 0; iii < MTSZ; ++iii ) {
            TEST_TRUE( rowvic(0,iii) == monkey_typist[iii] );
            TEST_TRUE( colvic(iii,0) == monkey_typist[iii] );
        }
    }

    test_ctors_rc< Matrix >();
    test_ctors_rc< kjb::Int_matrix >();

    // get_row, get_col
    {
        Vector row1( test_2.get_row( 1 ) ), col1( test_2.get_col( 1 ) );
        TEST_TRUE( 3 == row1.get_length() );
        TEST_TRUE( 3 == col1.get_length() );
        TEST_TRUE( 4.0 == row1[0] );
        TEST_TRUE( 5.0 == row1[1] );
        TEST_TRUE( 6.0 == row1[2] );
        TEST_TRUE( 2.0 == col1[0] );
        TEST_TRUE( 5.0 == col1[1] );
        TEST_TRUE( 8.0 == col1[2] );
    }

    test_set_row< Matrix >();
    test_set_row< kjb::Int_matrix >();

    // self assignment
    test_2 = test_2;
    TEST_TRUE( *test_1 == test_2);

    // zero size assignment for Matrix
    {
        Matrix::Impl_type * mevil = 0;
        int rc1 = kjb_c::get_target_matrix( &mevil, 0, 0 );
        assert( kjb_c::NO_ERROR == rc1 );
        Matrix dut( 2, 3 );
        TEST_TRUE( 2 == dut.get_num_rows() );
        TEST_TRUE( 3 == dut.get_num_cols() );
        dut = *mevil;
        TEST_TRUE( 0 == dut.get_num_rows() );
        TEST_TRUE( 0 == dut.get_num_cols() );
        kjb_c::free_matrix( mevil );
    }

    // zero size assignment for Int_matrix
    {
        kjb::Int_matrix::Impl_type * mevil = 0;
        int rc1 = kjb_c::get_target_int_matrix( &mevil, 0, 0 );
        assert( kjb_c::NO_ERROR == rc1 );
        kjb::Int_matrix dut( 2, 3 );
        TEST_TRUE( 2 == dut.get_num_rows() );
        TEST_TRUE( 3 == dut.get_num_cols() );
        dut = *mevil;
        TEST_TRUE( 0 == dut.get_num_rows() );
        TEST_TRUE( 0 == dut.get_num_cols() );
        kjb_c::free_int_matrix( mevil );
    }

    // non-self assignment
    Matrix test_three(7,2, -17.17 );
    TEST_FALSE( test_three == test_2 );
    test_three = test_2;                // deep copy, not an alias
    TEST_TRUE( test_three == test_2 );
    test_three(0) -= 1;                 // so it doesn't change test_2,
    TEST_FALSE( test_three == test_2 ); // as we see here

    // transpose
    test_2 = Matrix(*test_1).transpose();
    for(int row = 0; row < test_2.get_num_rows(); row++)
    {
        for(int col = 0; col < test_2.get_num_cols(); col++)
        {
            TEST_TRUE( test_2(row,col) == (*test_1)(col,row) );
        }
    }

    TEST_TRUE(*test_1 == test_2.transpose());

    delete test_1;

    test_1 = new Matrix(3, 3, &count_matrix[0][0]);
    test_2 = Matrix(3,3, &pi_matrix[0][0]);

    Matrix result;
    Matrix ref_result;

    test_max_abs <Matrix, double> ( M_PI, M_E );
    test_max_abs <kjb::Int_matrix, int> ( -123, 741 );

    // MATRIX MULTIPLICATION
    {
        const double result_matrix[3][3] = {
            {18.84954, 18.84954, 18.84954},
            {47.12385, 47.12385, 47.12385},
            {75.39816, 75.39816, 75.39816}
        };

        ref_result = Matrix(3, 3, &result_matrix[0][0]);

        result = *test_1 * test_2;

        //TEST_TRUE(result == ref_result);
        TEST_TRUE( kjb::max_abs_difference( result, ref_result ) < 1e-4 );

        // try that resizing code a bit more
        Matrix too_big( ref_result );
        too_big.resize( 10, 10, -666 );
        Matrix just_right( too_big );
        just_right.resize( 3, 3, -666 );
        TEST_FAIL( kjb::max_abs_difference( too_big, ref_result ) < 1e-4 );
        TEST_TRUE( kjb::max_abs_difference( just_right, ref_result ) < 1e-4 );

        // The (multiplicative) identity matrix -- does it work?
        Matrix id1( kjb::create_identity_matrix(3) );
        Matrix product1 = ref_result * id1;
        TEST_TRUE( product1 == ref_result);
        Matrix product2 = id1 * ref_result;
        TEST_TRUE( product2 == ref_result);

        test_1->resize(3,4);
        TEST_FAIL(*test_1 * test_2);
        TEST_FAIL(*test_1 *= test_2);
        TEST_FAIL(test_1->multiply(test_2));
        test_1->resize(3,3);
    }


    result = *test_1;
    result *= test_2;
    //TEST_TRUE(result == ref_result);
    TEST_TRUE( kjb::max_abs_difference( result, ref_result ) < 1e-4 );

    result = *test_1;
    result.multiply(test_2);
    //TEST_TRUE(result == ref_result);
    TEST_TRUE( kjb::max_abs_difference( result, ref_result ) < 1e-4 );

    test_2 = Matrix(3,3);
    test_2(0,0) = 19;
    test_2(0,1) = 18;
    test_2(0,2) = 17;
    test_2(1,0) = 16;
    test_2(1,1) = 15;
    test_2(1,2) = 14;
    test_2(2,0) = 13;
    test_2(2,1) = 12;
    test_2(2,2) = 11;

    itest_2 = kjb::floor(test_2);

    // try out the row-major, one-dimensional indexing, and the const indexing
    const Matrix& clone_t2 = test_2;
    TEST_TRUE( clone_t2[6] == 13 );
    TEST_TRUE( clone_t2[0] == 19 );
    TEST_TRUE( clone_t2[8] == 11 );
    TEST_TRUE( clone_t2.at(8) == 11 );
    TEST_TRUE( clone_t2.at(1,1) == 15 );
    test_2.at(8) = 654;
    TEST_FALSE( clone_t2.at(8) == 11 );
    test_2.at(8) = 11;
    TEST_TRUE( clone_t2.at(8) == 11 );
    test_2.at(1,1) = 654;
    TEST_FALSE( clone_t2.at(1,1) == 15 );
    test_2.at(1,1) = 15;
    TEST_TRUE( clone_t2.at(1,1) == 15 );

    // MATRIX MULTIPLICATION
    {
        double result_matrix[3][3] = {
            {90, 84, 78},
            {234, 219, 204},
            {378, 354, 330}
        };

        ref_result = Matrix(3, 3, &result_matrix[0][0]);
        kjb::Int_matrix iref = kjb::create_int_matrix_from_matrix_floor( ref_result );
        kjb::Int_matrix iref_2 = kjb::create_int_matrix_from_matrix_ceil( ref_result );
        TEST_TRUE( iref == iref_2 );
        kjb::Int_matrix factor1 = test_1 -> floor(),    // eq. to count_matrix
                        factor2 = test_2.floor(),       // see line 443
                        prod = factor1 * factor2;

        result = *test_1 * test_2;

        TEST_TRUE(result == ref_result);
        TEST_TRUE( prod == result.floor() );
        TEST_FAIL( prod = factor1 * kjb::Int_matrix(4,3,0) );
        TEST_TRUE( prod == factor1.multiply( factor2 ) );

        Matrix eye4( kjb::create_identity_matrix( 4 ) ), fail;
        TEST_FAIL( fail = result * eye4 );

        TEST_FAIL(create_zero_matrix(100) * result);
        TEST_FAIL(result *= create_zero_matrix(100));
        TEST_FAIL(result.multiply(create_zero_matrix(100)));
    }

    test_file_io( test_2 );
    test_file_io( test_2.floor() );

 
    {
        kjb_c::Matrix* c_m = NULL;
        kjb_c::get_random_matrix(&c_m, 100, 100);
        kjb::Matrix m(c_m);

        kjb::Int_matrix im = kjb::floor(m);

        test_serialization( m );
        test_serialization( im );
    }
    
    // STREAM IO
    {
        std::ostringstream oss;
        oss << test_2;
        TEST_TRUE( oss.str() ==
                        "  1.90000000e+01  1.80000000e+01  1.70000000e+01\n"
                        "  1.60000000e+01  1.50000000e+01  1.40000000e+01\n"
                        "  1.30000000e+01  1.20000000e+01  1.10000000e+01\n"
                    );
    }

    // FLOOR and CEILING
    {
        const
        double mfloat[] = {-10.99, -9.01,-8, -0.001,0.001, 1.99, 2.01, 3, 4.5};
        const
        int mfloor[]    = {-11,   -10,   -8, -1,    0,     1,    2,    3, 4  };
        const
        int mceil[]     = {-10,    -9,   -8,  0,    1,     2,    3,    3, 5  };
        const Matrix mm( 3, 3, mfloat );
        const kjb::Int_matrix floor_m( mm.floor() ), ceil_m( mm.ceil() );
        for( int iii = 0; iii < 9; ++iii ) {
            TEST_TRUE( floor_m(iii) == mfloor[iii] );
            TEST_TRUE( ceil_m(iii) == mceil[iii] );
        }
    }

    // INVERSION
    {
        // monkey-at-keyboard numbers:
        const double aaa[] = { 564,846,56,456,452,21,89,345,921 };
        const double bbb[] = { 34,74,778,124,848,8329,235,213,78 };
        Matrix ma( 3, 3, aaa ), mb( 3, 3, bbb ), mab( ma*mb );
        Matrix bam = matrix_inverse( mab );
        Matrix eye1 = bam * mab;
        Matrix eye2 = mab * bam;
        Matrix eye3( kjb::create_identity_matrix( 3 ) );
        // lazy way to check for inverse (assumes mul. works already).
        TEST_TRUE( kjb::max_abs_difference( eye1, eye3 ) < 1e-9 );
        TEST_TRUE( kjb::max_abs_difference( eye2, eye3 ) < 1e-9 );
        /* Here is a slightly more "pure" inverse test:  the array below holds
         * approximate values of the inverse, correct to half a unit.
         */
        double abinv[] = { -1366,2456,37,1507,-2748,13,-128.8,238,-2 };
        Matrix sloppyabinv( 3, 3, abinv );
        sloppyabinv /= 1e7;
        TEST_TRUE( kjb::max_abs_difference( sloppyabinv, bam ) < 5e-8 );
    }

    result = *test_1;
    result *= test_2;
    TEST_TRUE(result == ref_result);

    // SCALAR MULTIPLICATION
    {
        const double result_matrix[3][3] = {
            {2,  4,  6},
            {8,  10, 12},
            {14, 16, 18}
        };

        ref_result = Matrix(3, 3, &result_matrix[0][0]);

        result = 2 * *test_1 ;
        TEST_TRUE(result == ref_result);

        result = *test_1 * 2;
        TEST_TRUE(result == ref_result);

        result = *test_1;
        result *= 2;
        TEST_TRUE(result == ref_result);

        result *= 0.5;
        TEST_FALSE(result == ref_result);
        result *= 2;
        TEST_TRUE(result == ref_result);

        result /= 2.0;
        TEST_FALSE(result == ref_result);
        result.multiply( 2.0 );
        TEST_TRUE(result == ref_result);
        result.divide( 2.0 );
        TEST_FALSE(result == ref_result);

        TEST_FAIL( result /= 0 );
        Matrix fail;
        TEST_FAIL( fail = result / 0 );
    }

    // ADDITION
    {
        const double result_matrix[3][3] = {
            { 4.14159,   5.14159,   6.14159},
            { 7.14159,   8.14159,   9.14159},
            {10.14159,  11.14159,  12.14159}
        };

        test_2 = Matrix(3,3,M_PI);
        ref_result = Matrix(3, 3, &result_matrix[0][0]);

        result = *test_1 + test_2;

        //TEST_TRUE(result == ref_result);
        TEST_TRUE( kjb::max_abs_difference( result, ref_result ) < 1e-5 );

        result = *test_1;
        result += test_2;
        //TEST_TRUE(result == ref_result);
        TEST_TRUE( kjb::max_abs_difference( result, ref_result ) < 1e-5 );

        // The additive identity -- do it work?
        Matrix id1( kjb::create_zero_matrix(3) );
        Matrix sum1( id1 + ref_result );
        TEST_TRUE( sum1 == ref_result );
        Matrix sum2( ref_result + id1 );
        TEST_TRUE( sum2 == ref_result );
        Matrix id2 = result;
        id2.zero_out();
        Matrix sum3( ref_result + id2 );
        TEST_TRUE( sum3 == ref_result );
        Matrix id3( 47, 47, 17.89 );
        id3.zero_out( 3 );
        Matrix sum4( ref_result + id3 );
        TEST_TRUE( sum4 == ref_result );
        sum4.add( id3 );
        TEST_TRUE( sum4 == ref_result );

        TEST_FAIL(create_zero_matrix(100) + id3);
        TEST_FAIL(id3 += create_zero_matrix(100));
        TEST_FAIL(id3.add(create_zero_matrix(100)));
    }

    // SUBTRACTION
    {
        const double result_matrix[3][3] = {
            {-2.14159,  -1.14159,  -0.14159},
            {0.85841,   1.85841,   2.85841},
            {3.85841,   4.85841,   5.85841}
        };

        ref_result = Matrix(3, 3, &result_matrix[0][0]);

        result = *test_1 - test_2;

        //TEST_TRUE(result == ref_result);
        TEST_TRUE( kjb::max_abs_difference( result, ref_result ) < 1e-5 );

        result = *test_1;
        result -= test_2;
        //TEST_TRUE(result == ref_result);
        TEST_TRUE( kjb::max_abs_difference( result, ref_result ) < 1e-5 );

        result = *test_1;
        result.subtract( test_2 );
        TEST_TRUE( kjb::max_abs_difference( result, ref_result ) < 1e-5 );

        TEST_FAIL(create_zero_matrix(100) - result);
        TEST_FAIL(result -= create_zero_matrix(100));
        TEST_FAIL(result.subtract(create_zero_matrix(100)));
    }

    // NEGATION
    {
        const double result_matrix[3][3] = {
            {-1, -2, -3},
            {-4, -5, -6},
            {-7, -8, -9}
        };
        ref_result = Matrix(3, 3, &result_matrix[0][0]);

        result = -(*test_1);
        TEST_TRUE(result == ref_result);
        TEST_TRUE( abs(result) == *test_1 );
        TEST_FALSE( result == *test_1 );

        result = (*test_1);
        result.negate();
        TEST_TRUE(result == ref_result);
    }

    // min, max
    TEST_TRUE(kjb::min(*test_1) == 1);
    TEST_TRUE(kjb::max(*test_1) == 9);


    test_mapcar();
    test_serialization();

    // init functions
#if 0 /* euler functions are gone now */
    test_2.init_euler_zxz(M_PI_2., M_PI/3., M_PI_4.);

    src_matrix2 = NULL;
    get_euler_matrix_zxz_reference(&src_matrix2, M_PI_2., M_PI/3., M_PI_4.);
    *test_1 = (kjb::Matrix) *src_matrix2;

    TEST_TRUE(*test_1 == test_2);

    test_2.init_euler_zxz(M_PI_2., M_PI/3., M_PI_4., true);

    TEST_TRUE(test_2.get_num_rows() == 4 &&
          test_2.get_num_cols()  == 4);
    TEST_TRUE(test_2(3,0) == test_2(3,1) && test_2(3,1) == test_2(3,2) && test_2(3,2) == test_2(0,3) && test_2(0,3) == test_2(1,3) && test_2(1,3) == test_2(2,3) && test_2(2,3) == 0);
    TEST_TRUE(test_2(3,3)  == 1);
    TEST_TRUE(test_2.resize(3,3) == *test_1);

    // init_indentity
    test_1->init_identity(3);
    TEST_TRUE(test_2 == *test_1 * test_2);

    kjb_c::free_matrix(src_matrix2);
#endif

    // init_indentity
    delete test_1;
    test_1 = new Matrix(kjb::create_identity_matrix(3));

    // VECTOR MULTIPLICATION
    Vector v = Vector(3);
    v(0) = 1;
    v(1) = 2;
    v(2) = 3;

    // Multiplication with identity matrix
#if 0
    TEST_TRUE(v == Vector(v).right_multiply(*test_1));
#endif
    TEST_TRUE(v == *test_1 * v);
    Vector fail;
    TEST_FAIL( fail = kjb::create_identity_matrix(4) * v );

    delete ref_1;
    delete test_1;

    test_stl_view();
    test_stream_io();

    }
    
    catch(kjb::Exception& e)
    {
        e.print_details();
        DOWN_IN_FLAMES("unexpected err 1","(everything)");
    }
    catch(...)
    {
        DOWN_IN_FLAMES("unexpected err 2","(everything)");
    }
    RETURN_VICTORIOUSLY();
}
