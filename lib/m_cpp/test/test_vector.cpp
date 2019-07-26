/* ======================================================================== *
 |                                                                          |
 | Copyright (c) 2010, by members of University of Arizona Computer         |
 | Vision group (the authors) including                                     |
 |       Andrew Predoehl.                                                   |
 |                                                                          |
 | For use outside the University of Arizona Computer Vision group please   |
 | contact Kobus Barnard.                                                   |
 |                                                                          |
 * ======================================================================== */

/* $Id: test_vector.cpp 18278 2014-11-25 01:42:10Z ksimek $ */

#include "l/l_incl.h"
#include "l_cpp/l_stdio_wrap.h"
#include "l_cpp/l_test.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"
#include "m_cpp/m_mat_view.h"
#include "m_cpp/m_vec_view.h"
#include "m_cpp/m_int_vector.h"
#include "l_cpp/l_int_matrix.h"
#include <sstream>
#include <fstream>

#ifdef KJB_HAVE_BST_SERIAL
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif



namespace {

// I'm trying to limit the number of places that "kjb_c" appears anywhere
const int SUCCESS_CODE = kjb_c::NO_ERROR;


template< typename VEC_T, typename LENGTH_T >
int test_ctors()
{
    VEC_T v1;                           // zero-length structure (implicit)
    TEST_TRUE( 0 == v1.get_length() );
    TEST_FAIL( 0 == v1.at(0) );
    TEST_FAIL( v1.at(0) = 0 );

    VEC_T v3( LENGTH_T(0) );             // ditto (explicit)
    TEST_TRUE( 0 == v3.get_length() );
    TEST_FAIL( 0 == v3.at(0) );
    TEST_FAIL( v3.at(0) = 0 );

    VEC_T v2( LENGTH_T(3) );             // nonzero size -- it's a container
    v2[0] = 17;
    v2[1] = 29;
    v2[2] = -5;
    TEST_TRUE( 3 == v2.get_length() );
    TEST_FAIL( v2.at(3) = 65 );
    TEST_FAIL( 65 == v2.at(3) );

    TEST_TRUE( 17 == v2[0] );           // make sure the storage isn't aliased
    TEST_TRUE( 29 == v2[1] );
    TEST_TRUE( -5 == v2[2] );
    TEST_FALSE( 17 == v2[1] );
    TEST_FALSE( 29 == v2[2] );
    TEST_FALSE( -5 == v2[0] );
    TEST_FALSE( 17 == v2[2] );
    TEST_FALSE( 29 == v2[0] );
    TEST_FALSE( -5 == v2[1] );

    TEST_FALSE( 17 != v2[0] );          // ... and that it is unaliased
    TEST_FALSE( 29 != v2[1] );
    TEST_FALSE( -5 != v2[2] );
    TEST_TRUE( 17 != v2[1] );
    TEST_TRUE( 29 != v2[2] );
    TEST_TRUE( -5 != v2[0] );
    TEST_TRUE( 17 != v2[2] );
    TEST_TRUE( 29 != v2[0] );
    TEST_TRUE( -5 != v2[1] );

    return SUCCESS_CODE;
}




template< typename VEC_T >
int test_ctors_2()
{
    typedef typename VEC_T::Value_type DATUM_T;

    VEC_T v1( 10, DATUM_T(M_PI) );          // ctor fills with initial value
    for( int iii = 0; iii < 10; ++iii )
    {
        TEST_TRUE( DATUM_T(M_PI) == v1.at(iii) );
    }

    const DATUM_T filler[] = { DATUM_T(-M_PI), DATUM_T(M_E),
                                        DATUM_T(M_PI_2), DATUM_T(-M_SQRT2) };
    VEC_T v2( 4, filler );                  // ctor fills from an array
    TEST_TRUE( 4 == v2.get_length() );
    TEST_TRUE( DATUM_T(-M_PI) == v2.at(0) );
    TEST_TRUE( DATUM_T(M_E) == v2.at(1) );
    TEST_TRUE( DATUM_T(M_PI_2) == v2.at(2) );
    TEST_TRUE( DATUM_T(-M_SQRT2) == v2.at(3) );

    return SUCCESS_CODE;
}


template< typename VEC_T >
int test_ctor_3()
{
    typedef typename VEC_T::Mat_type MAT_T;

    const int SZ=10;
    /* VEC_T rr = kjb::create_random_vector( SZ ); */
    VEC_T rr(SZ);
    rr.randomize();

    /* For integer vectors, this next step makes rr's contents able to be
     * doubled without overflow.
     */
    rr /= 2;

    VEC_T ss( rr );
    const VEC_T ss_const( ss );
    VEC_T zz( rr );
    zz.resize( 1+SZ );

    TEST_TRUE( SZ == rr.get_length() );
    TEST_TRUE( SZ == ss.get_length() );
    TEST_FALSE( SZ == zz.get_length() );
    for( int iii = 0; iii < SZ; ++iii )
    {
        TEST_TRUE( rr.at( iii ) == ss.at( iii ) );
        TEST_TRUE( rr.at( iii ) == zz.at( iii ) );
    }
    VEC_T rrrr = rr * 2;
    rr *= 2;    // even floating point mult. by powers of 2 can be pretty exact
    for( int iii = 0; iii < SZ; ++iii )
    {
        TEST_FALSE( rr.at( iii ) == ss.at( iii ) );
        TEST_TRUE( rr.at( iii ) == 2 * ss.at( iii ) );
        TEST_TRUE( rr.at( iii ) == rrrr.at( iii ) );
    }
    rr /= 2;
    for( int iii = 0; iii < SZ; ++iii )
    {
        TEST_TRUE( rr.at( iii ) == ss.at( iii ) );
        TEST_TRUE( rr.at( iii ) == zz.at( iii ) );
        TEST_FALSE( rr.at( iii ) == rrrr.at( iii ) );
    }
    TEST_TRUE( rr == ss );
    TEST_FALSE( rr != ss );
    TEST_TRUE( rr != zz );
    TEST_TRUE( ss != zz );
    TEST_FALSE( rr == zz );
    TEST_FALSE( ss == zz );
    TEST_FALSE( rr == rrrr );

    VEC_T rrrr_also = 2 * rr;
    TEST_TRUE( rrrr == rrrr_also );
    rrrr.multiply( 2 );
    TEST_FALSE( rrrr == rrrr_also );
    VEC_T rrdupe = rrrr / 2;
    rrrr.divide( 2 );
    TEST_TRUE( rrrr == rrrr_also );
    TEST_TRUE( rrrr == rrdupe );
    rrrr = rrrr;                // self-assignment?  more like SAFE-assignment!
    TEST_TRUE( rrrr == rrdupe );

    MAT_T mu1 = kjb::create_column_matrix( rr );
    MAT_T mu2 = kjb::create_row_matrix( rr );
    MAT_T dud1, dud2( 2,2 );
    MAT_T boring(1,1,-.654321);

    TEST_TRUE( SZ == mu1.get_num_rows() );
    TEST_TRUE( SZ == mu2.get_num_cols() );
    TEST_TRUE( 1 == mu1.get_num_cols() );
    TEST_TRUE( 1 == mu2.get_num_rows() );
    for( int iii = 0; iii < SZ; ++iii )
    {
        TEST_TRUE( mu1.at( iii ) == ss.at( iii ) );
        TEST_TRUE( mu2.at( iii ) == ss.at( iii ) );
        TEST_TRUE( mu1.at( iii, 0 ) == ss.at( iii ) );
        TEST_TRUE( mu2.at( 0, iii ) == ss.at( iii ) );
        TEST_TRUE( ss.at( iii ) == ss_const.at( iii ) );
    }

    VEC_T tt( mu1 ), uu( mu2 );
    TEST_TRUE( tt == ss );
    TEST_TRUE( uu == ss );
    TEST_FAIL( tt = VEC_T( dud1 ) );
    TEST_FAIL( tt = VEC_T( dud2 ) );
    TEST_SUCCESS( tt = VEC_T( boring ) );

    return SUCCESS_CODE;
}


void do_nothing(const char*)
{
}


int test_ctor_4()
{
    kjb::Vector singleton(1, M_PI);
    TEST_TRUE( 1 == singleton.get_length() );
    TEST_TRUE( singleton.at(0) == M_PI );
    singleton.at(0) = M_E;
    TEST_FALSE( singleton.at(0) == M_PI );

    const double dat[] = { M_E, M_E * 2, M_E * 4, M_E * 8, M_E * 16, M_E * 32};
    const kjb::Vector ref( 6, dat );

    kjb::Int_vector iref( kjb::floor( ref ) );
    TEST_TRUE( 6 == iref.size() );
    TEST_TRUE( 2 == iref[0] );  // floor 2.72
    TEST_TRUE( 5 == iref[1] );  // floor 5.44
    TEST_TRUE( 10 == iref[2] ); // floor 10.87
    TEST_TRUE( 21 == iref[3] ); // floor 21.75
    TEST_TRUE( 43 == iref[4] ); // floor 43.49
    TEST_TRUE( 86 == iref[5] ); // floor 86.99

    kjb::Vector slice1 = kjb::create_vector_from_vector_section( ref, 0, 6 );
    TEST_TRUE( slice1 == ref );

    kjb_c::set_bug_handler( do_nothing );
    // bad start index
    TEST_FAIL( kjb::create_vector_from_vector_section( ref, -1,  1 ) );
    // length too long
    TEST_FAIL( kjb::create_vector_from_vector_section( ref,  0,  7 ) );
    // length negative
    TEST_FAIL( kjb::create_vector_from_vector_section( ref,  0, -1 ) );
    // start + length too long
    TEST_FAIL( kjb::create_vector_from_vector_section( ref,  4,  4 ) );
    kjb_c::set_bug_handler( kjb_c::default_bug_handler );

    // and speaking of "section" --
    // as of nov 26 2010 this member does not exist...i have no idea
    // where it went

    /* kjb::Vector vec( 10, M_PI );
    vec.copy_from_vector_section( ref, 3, 0, 6 );
    vec.copy_from_vector_section( ref, 1, 0, 0 );
    TEST_TRUE( vec.at(0) == M_PI );
    TEST_TRUE( vec.at(1) == M_PI );
    TEST_TRUE( vec.at(2) == M_PI );
    TEST_TRUE( vec.at(3) == ref.at(0) );
    TEST_TRUE( vec.at(4) == ref.at(1) );
    TEST_TRUE( vec.at(5) == ref.at(2) );
    TEST_TRUE( vec.at(6) == ref.at(3) );
    TEST_TRUE( vec.at(7) == ref.at(4) );
    TEST_TRUE( vec.at(8) == ref.at(5) );
    TEST_TRUE( vec.at(9) == M_PI );
    TEST_FAIL( vec.copy_from_vector_section( ref, 5, 0, 6 ) );
    TEST_FAIL( vec.copy_from_vector_section( ref, -1, 0, 0 ) );
    TEST_FAIL( vec.copy_from_vector_section( ref, 0, -1, 0 ) );
    TEST_FAIL( vec.copy_from_vector_section( ref, 0, 0, -1 ) );
    TEST_FAIL( vec.copy_from_vector_section( ref, 0, 0, 7 ) ); */
    return SUCCESS_CODE;
}


int test_ctor_5f()
{
    const int SZ = 5;
    kjb_c::Vector *raw = 0, *nil = 0;
    kjb_c::get_target_vector( &raw, SZ );
    for( int iii = 0; iii < SZ; ++iii )
    {
        double val = M_E * (double)(1<<iii);
        raw -> elements[ iii ] = val;
    }

    kjb::Vector safe( *raw ), shallow( nil );
    kjb::Vector zer0 = kjb::create_zero_vector( SZ );
    TEST_TRUE( SZ == safe.get_length() );
    TEST_TRUE( SZ == zer0.get_length() );
    TEST_TRUE( 0 == shallow.get_length() );
    for( int iii = 0; iii < SZ; ++iii )
    {
        double val = M_E * (double)(1<<iii);
        TEST_TRUE( val == safe.at(iii) );       // it made a copy, right?
        TEST_FALSE( val == zer0.at(iii) );
        TEST_TRUE( 0 == zer0.at(iii) );
    }
    TEST_TRUE( safe == *raw );
    TEST_TRUE( *raw == safe );
    TEST_FALSE( safe != *raw );
    TEST_FALSE( *raw != safe );

    for( int iii = 0; iii < SZ; ++iii )
    {
        raw -> elements[ iii ] = 0;
    }
    for( int iii = 0; iii < SZ; ++iii )
    {
        double val = M_E * (double)(1<<iii);
        TEST_TRUE( val == safe.at(iii) );       // that copy was deep, right?
    }
    TEST_TRUE( safe != *raw );
    TEST_TRUE( *raw != safe );
    TEST_FALSE( safe == *raw );
    TEST_FALSE( *raw == safe );
    TEST_TRUE( zer0 == kjb::Vector( *raw ) );
    kjb_c::free_vector( raw );

    // test the int-to-double conversion ctor
    kjb::Int_vector ivec( 5 );
    kjb::Vector ref_vec( 5 );
    for( int iii = 0; iii < 5; ++iii )
    {
        ivec.at(iii) = 17 + iii;
        ref_vec.at(iii) = double( 17 + iii );
    }
    kjb::Vector test_vec( ivec );
    TEST_TRUE( test_vec == ref_vec );

    return SUCCESS_CODE;
}


int test_ctor_5i()
{
    const int SZ = 5;
    kjb_c::Int_vector* raw = 0;
    kjb_c::get_target_int_vector( &raw, SZ );
    for( int iii = 0; iii < SZ; ++iii )
    {
        int val = M_E * (double)(1<<iii);
        raw -> elements[ iii ] = val;
    }

    kjb::Int_vector safe( *raw );
    kjb::Int_vector zer0( SZ, 0 );
    TEST_TRUE( SZ == safe.get_length() );
    TEST_TRUE( SZ == zer0.get_length() );
    for( int iii = 0; iii < SZ; ++iii )
    {
        int val = M_E * (double)(1<<iii);
        TEST_TRUE( val == safe.at(iii) );       // it made a copy, right?
        TEST_FALSE( val == zer0.at(iii) );
        TEST_TRUE( 0 == zer0.at(iii) );
    }
    TEST_TRUE( safe == *raw );
    TEST_TRUE( *raw == safe );
    TEST_FALSE( safe != *raw );
    TEST_FALSE( *raw != safe );

    for( int iii = 0; iii < SZ; ++iii )
    {
        raw -> elements[ iii ] = 0;
    }
    for( int iii = 0; iii < SZ; ++iii )
    {
        int val = M_E * (double)(1<<iii);
        TEST_TRUE( val == safe.at(iii) );       // that copy was deep, right?
    }
    TEST_TRUE( safe != *raw );
    TEST_TRUE( *raw != safe );
    TEST_FALSE( safe == *raw );
    TEST_FALSE( *raw == safe );
    TEST_TRUE( zer0 == kjb::Int_vector( *raw ) );
    kjb_c::free_int_vector( raw );
    return SUCCESS_CODE;
}

int test_insert()
{
    kjb::Vector v0;
    kjb::Vector v1(4);
    v1[0] = 0;
    v1[1] = 1;
    v1[2] = 4;
    v1[3] = 5;

    kjb::Vector v2(2);
    v2[0] = 2;
    v2[1] = 3;

    v1.insert(v1.begin()+2, v2.begin(), v2.end());
    // v1 is now [0,1,2,3,4,5];

    v0.insert(v0.begin(), v1.begin(), v1.end());
    // v0 is now same as v1;
    
    // test for {v0, v2} = [0,1,2,3,4,5];
    size_t N = v0.size();
    for(size_t i = 0; i < N; ++i)
    {
        TEST_TRUE(v1[i] == i);
        TEST_TRUE(v0[i] == i);
    }

    // insert at end
    v1.insert(v1.end(), v0.begin(), v0.end());
    for(size_t i = 0; i < N; ++i)
        TEST_TRUE(v1[i] == i);
    for(size_t i = 0; i < N; ++i)
        TEST_TRUE(v1[N + i] == i);

    // fill insert (overlap and extend)
    v1.insert(v1.begin() + N, 2*N, -1.0);

    // [0 1 2 3 4 5 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 0 1 2 3 4 5]
    for(size_t i = 0; i < N; ++i)
        TEST_TRUE(v1[i] == i);
    for(size_t i = N; i < 3*N; ++i)
        TEST_TRUE(v1[i] == -1);
    for(size_t i = 0; i < N; ++i)
        TEST_TRUE(v1[3*N + i] == i);

    // single element insert
    size_t I = 2;
    v1.insert(v1.begin() + I, -2);
    // [0 1 -2 2 3 4 5 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 0 1 2 3 4 5]
    for(size_t i = 0; i < N+1; ++i)
    {
        if(i < I)
            TEST_TRUE(v1[i] == i);
        else if(i == I)
            TEST_TRUE(v1[i] == -2);
        else
            TEST_TRUE(v1[i] == i-1);
    }

    for(size_t i = N+1; i < 3*N+1; ++i)
        TEST_TRUE(v1[i] == -1);
    for(size_t i = 0; i < N; ++i)
        TEST_TRUE(v1[3*N+1+i] == i);

    return SUCCESS_CODE;
}


int test_work_1()
{
    kjb::Vector vec( 5, M_PI );
    vec.zero_out();
    TEST_TRUE( 5 == vec.get_length() );

    bool is_all_0 = true;
    for( int iii = 0; iii < 5; ++iii )
    {
        if ( vec.at(iii) != 0 )
        {
            is_all_0 = false;
        }
    }
    TEST_TRUE( is_all_0 );

    vec.randomize();
    TEST_TRUE( 5 == vec.get_length() );

    for( int iii = 0; iii < 5; ++iii )
    {
        if ( vec.at(iii) != 0 )
        {
            is_all_0 = false;
        }
    }
    TEST_FALSE( is_all_0 ); // there's an infinitesimal (read: very, very small :P) chance this won't work

    const kjb::Vector& cvec = vec;
    is_all_0 = true;
    for( int iii = 0; iii < 5; ++iii )
    {
        if ( cvec.at(iii) != 0 )
        {
            is_all_0 = false;
        }
    }
    TEST_FALSE( is_all_0 ); // there's an infinitesimal chance this won't work

     kjb::Vector vec2, vec3;
    const kjb_c::Vector *alias = vec.get_c_vector();
    vec2.randomize( 5 );
    TEST_TRUE( vec.get_length() == vec2.get_length() );
    TEST_FALSE( vec == vec2 ); // infinitesimal risk of failure
    vec3.zero_out( 5 );
    TEST_TRUE( vec3.get_length() == vec2.get_length() );
    TEST_FALSE( vec3 == vec2 ); // infinitesimal risk of failure
    TEST_TRUE( vec == *alias );

    vec3 = vec2;    // assignment
    TEST_TRUE( vec3 == vec2 );
    vec3.at(0) += 0.5;
    TEST_FALSE( vec3 == vec2 );
    kjb::Vector *enigma1 = &vec3;
    kjb::Vector *enigma2 = 0;
    enigma2 = enigma1;
    kjb::Vector& enigma3 = *enigma2;
    enigma3 = vec3; // no assignment, since they are the same object

    return SUCCESS_CODE;
}


int test_work_2()
{
    const double    data1[] = { M_PI,       -(M_E), M_LN2,      -(M_SQRT2)  },
                    data2[] = { M_SQRT1_2,  M_1_PI, M_LOG2E,    M_PI_4      };
    double          data3[] = { 0,          0,      0,          0           };
    for( int iii = 0; iii < 4; ++iii )
    {
        data3[iii] = data1[iii] + data2[iii];
    }

    const kjb::Vector v1_ref( 4, data1 );
    const kjb::Vector v2_ref( 4, data2 );
    const kjb::Vector v3_ref( 4, data3 );
    const kjb::Vector v4_ref;
    kjb::Vector v1( v1_ref );
    kjb::Vector v2( v2_ref );
    kjb::Vector v3_test = v1 + v2;

    kjb::Vector v1cp( v1 );
    v1cp.add( v2 );
    v2 += v1;

    for( int iii = 0; iii < 4; ++iii )
    {
        TEST_TRUE( v3_ref.at( iii ) == v3_test.at( iii ) );
        TEST_TRUE( v3_ref.at( iii ) == v1cp.at( iii ) );
        TEST_TRUE( v3_ref.at( iii ) == v2.at( iii ) );
    }

    TEST_FAIL( kjb::max_abs_difference( kjb::Vector(3), v1cp - v1_ref ) < 1e-15 );
    TEST_TRUE( kjb::max_abs_difference( v2_ref, v1cp - v1_ref ) < 1e-15 );
    v2 -= v2_ref;
    TEST_TRUE( kjb::max_abs_difference( v1_ref, v2 ) < 1e-15 );
    v2.subtract( v1_ref );
    TEST_TRUE( kjb::max_abs_difference( v2, kjb::Vector(4,0.0) ) < 1e-15 );

    v2 = -v2_ref;
    for( int iii = 0; iii < 4 ; ++iii )
    {
        TEST_TRUE( v2.at(iii) * -1 == v2_ref.at(iii) );
    }
    v2.negate();
    TEST_TRUE( v2 == v2_ref );

    TEST_TRUE( kjb::min(v1_ref) == -(M_E) );
    TEST_TRUE( kjb::min(v2_ref) == M_1_PI );
    TEST_TRUE( kjb::max(v1_ref) == M_PI );
    TEST_TRUE( kjb::max(v2_ref) == M_LOG2E );

    TEST_TRUE( v1_ref.min(0) == -(M_E) );
    TEST_TRUE( v2_ref.min(0) == M_1_PI );
    TEST_TRUE( v1_ref.max(0) == M_PI );
    TEST_TRUE( v2_ref.max(0) == M_LOG2E );

    int index;
    TEST_TRUE( v1_ref.min( &index ) == -(M_E) );
    TEST_TRUE( 1 == index );
    TEST_TRUE( v2_ref.min( &index ) == M_1_PI );
    TEST_TRUE( 1 == index );
    TEST_TRUE( v1_ref.max( &index ) == M_PI );
    TEST_TRUE( 0 == index );
    TEST_TRUE( v2_ref.max( &index ) == M_LOG2E );
    TEST_TRUE( 2 == index );
    TEST_FAIL( v4_ref.min( &index ) );
    TEST_FAIL( v4_ref.max( &index ) );
    TEST_FAIL( v4_ref.min(0) );
    TEST_FAIL( v4_ref.max(0) );

    double refdot = 0, mag1sq = 0, mag2sq = 0;
    for( int iii = 0; iii < 4; ++iii )
    {
        refdot += data1[iii] * data2[iii];
        mag1sq += data1[iii] * data1[iii];
        mag2sq += data2[iii] * data2[iii];
    }
    TEST_TRUE( fabs( refdot - dot( v1_ref, v2_ref ) ) < 1e-15 );
    TEST_TRUE( fabs( mag1sq - v1_ref.magnitude_squared() ) < 2e-15 );
    TEST_TRUE( fabs( mag2sq - v2_ref.magnitude_squared() ) < 1e-15 );

    double mag1 = sqrt(mag1sq), mag2 = sqrt(mag2sq);
    TEST_TRUE( fabs( mag1 - v1_ref.magnitude() ) < 1e-15 );
    TEST_TRUE( fabs( mag2 - v2_ref.magnitude() ) < 1e-15 );

    kjb::Vector n1( v1_ref ), n2( v2_ref );
    n1.normalize();
    n2.normalize();
    for( int iii = 0; iii < 4; ++iii )
    {
        TEST_TRUE( fabs( n1.at(iii) - data1[iii]/mag1 ) < 1e-15 );
        TEST_TRUE( fabs( n2.at(iii) - data2[iii]/mag2 ) < 1e-15 );
    }
    TEST_TRUE( fabs( n1.magnitude_squared() - 1 ) < 1e-15 );
    TEST_TRUE( fabs( n2.magnitude_squared() - 1 ) < 1e-15 );

    return SUCCESS_CODE;
}


template< typename VEC_T >
int test_work_3()
{
    kjb::Temporary_File tf1, tf2;

    VEC_T vec;
    vec.randomize( 100 );
    vec.write_row( tf1.get_filename().c_str() );
    fflush(tf1);
    vec.write_col( tf2.get_filename().c_str() );
    fflush(tf2);

    VEC_T x1( tf1.get_filename() );
    TEST_TRUE( kjb::max_abs_difference( vec, x1 ) < 5e-6 );
    VEC_T x2( tf2.get_filename() );
    TEST_TRUE( kjb::max_abs_difference( vec, x2 ) < 5e-6 );
    VEC_T x3;
    TEST_FAIL( x3 = VEC_T( (const char*)0 ) );

    return SUCCESS_CODE;
}

template <class VEC_T>
void test_serialization(const VEC_T& v)
{
#ifdef KJB_HAVE_BST_SERIAL
    VEC_T v2;
    v2.randomize( 100 );

    kjb::Temporary_File tf;

    do
    {
        std::ofstream ofs(tf.get_filename().c_str());
        boost::archive::text_oarchive oa(ofs);
        oa << v;
    }
    while(0);
    fflush(tf);

    do
    {
        std::ifstream ifs(tf.get_filename().c_str());
        boost::archive::text_iarchive ia(ifs);
        ia >> v2;
    }
    while(0);

    TEST_TRUE( max_abs_difference(v, v2) <= FLT_EPSILON );
#else
    std::cerr << "Library boost::serialization not found so serialization was not tested." << std::endl;

#endif
}


template< typename VEC_T>
int test_work_4()
{
    typedef typename VEC_T::Value_type DATUM_T;
    typedef typename VEC_T::Mat_type MAT_T;

    DATUM_T dat[9];
    for( int iii = 0; iii < 9; ++iii )
    {
        dat[ iii ] = DATUM_T( M_PI + iii - 4 );
    }
    MAT_T mat( 3, 3, dat );

    VEC_T v1(3,0.0), v2(3,0.0), v3(3,0.0);
    v1.at(0)=1;
    v2.at(1)=1;
    v3.at(2)=1;

    VEC_T w1 = v1 * mat, w2 = v2 * mat, w3 = v3 * mat, wfail;

    TEST_FAIL( wfail = v1 * MAT_T( 4, 3, DATUM_T(0) ) );

    for( int iii = 0; iii < 3; ++iii )
    {
        TEST_TRUE( w1.at(iii) == dat[ iii     ] );
        TEST_TRUE( w2.at(iii) == dat[ iii + 3 ] );
        TEST_TRUE( w3.at(iii) == dat[ iii + 6 ] );
    }

    return SUCCESS_CODE;
}


template< typename VEC_T >
int test_hat()
{
    VEC_T fail1( 4 ), fail2( 3 );
    TEST_FAIL( cross( fail1, fail2 ) );
    TEST_FAIL( cross( fail2, fail1 ) );
    TEST_FAIL( fail1.hat() );

    VEC_T aye( 3 ), jay( 3 ), kay_ref( 3 ), kay;
    aye[0] = 29;
    aye[1] = 3;
    aye[2] = 5;
    jay[0] = 7;
    jay[1] = 31;
    jay[2] = 2;
    kay_ref[0] = -149;
    kay_ref[1] = -23;
    kay_ref[2] = 878;

    kay = aye;
    TEST_TRUE( kay_ref == kay.cross_with( jay ) );   // cross is an in-place op -- not anymore: now it's 'cross_with'
    TEST_TRUE( kay_ref == kay );

    TEST_TRUE( -kay_ref == jay.hat() * aye );

    return SUCCESS_CODE;
}


int test_work_5f()
{
    const kjb::Vector::Value_type data[]={M_PI, M_E, M_SQRT2, M_LOG2E, M_LN2};
    kjb::Vector v( 5, data );
    std::ostringstream so;
    so << v;
    TEST_TRUE( so.str() ==
                            "  3.14159265e+00  2.71828183e+00  1.41421356e+00"
                            "  1.44269504e+00  6.93147181e-01"
                );

    // zero length assignment (a corner case that causes trouble for Matrix)
    kjb_c::Vector *danger = 0;
    kjb_c::get_target_vector( &danger, 0 );
    kjb::Vector vv( 3 );
    TEST_TRUE( 3 == vv.get_length() );
    vv = *danger;
    TEST_TRUE( 0 == vv.get_length() );
    kjb_c::free_vector( danger );
    return SUCCESS_CODE;
}


int test_work_5i()
{
    const kjb::Int_vector::Value_type data[] = { 123, 0, -765, 999, -1 };
    kjb::Int_vector v( 5, data );
    std::ostringstream so;
    so << v;
    TEST_TRUE( so.str() ==
                            "             123               0            -765"
                            "             999              -1"
                );


    kjb_c::Int_vector *nassty(0), *nassty2(0);
    kjb_c::get_target_int_vector( &nassty, 100 );
    kjb::Int_vector raii( nassty );
    kjb::Int_vector raii2( nassty2 ); // doesn't freak out if nassty2 equals 0
    TEST_TRUE( 0 == raii2.get_length() );
    // The "test" here is that we never call free_int_vector() for nassty,
    // yet nevertheless there is no memory leak (no scolding message at exit).
    // To verify the effectiveness of the test itself, try commenting out the
    // definition of 'raii' and verify that the altered test program ends with
    // a message about "residual memory allocation," something that you do not
    // see when 'raii' (google it) takes responsibility for freeing 'nassty'.



    // One last miscellaneous thing:  this should throw, for integer vectors.
    TEST_FAIL( v.divide( 0 ) );

    // zero length assignment (a corner case that causes trouble for Matrix)
    kjb_c::Int_vector *danger = 0;
    kjb_c::get_target_int_vector( &danger, 0 );
    kjb::Int_vector vv( 3 );
    TEST_TRUE( 3 == vv.get_length() );
    vv = *danger;
    TEST_TRUE( 0 == vv.get_length() );
    kjb_c::free_int_vector( danger );

    return SUCCESS_CODE;
}

template< typename VEC_T>
int test_work_6()
{
    typedef typename VEC_T::Value_type DATUM_T;
    typedef typename VEC_T::Mat_type MAT_T;

    const DATUM_T primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37 };
    const VEC_T optimi( 12, primes ), dtcons( 12, DATUM_T(-3) ), empty;

    const VEC_T voltron( optimi + dtcons );
    TEST_TRUE( DATUM_T(-1) == voltron[ 0 ] );
    TEST_TRUE( DATUM_T( 0) == voltron[ 1 ] );
    TEST_TRUE( DATUM_T( 2) == voltron[ 2 ] );
    TEST_TRUE( DATUM_T( 4) == voltron[ 3 ] );
    TEST_TRUE( DATUM_T( 8) == voltron[ 4 ] );
    TEST_TRUE( DATUM_T(10) == voltron[ 5 ] );
    TEST_TRUE( DATUM_T(14) == voltron[ 6 ] );
    TEST_TRUE( DATUM_T(16) == voltron[ 7 ] );
    TEST_TRUE( DATUM_T(20) == voltron[ 8 ] );
    TEST_TRUE( DATUM_T(26) == voltron[ 9 ] );
    TEST_TRUE( DATUM_T(28) == voltron[ 10 ] );
    TEST_TRUE( DATUM_T(34) == voltron[ 11 ] );
    TEST_FAIL( DATUM_T(-273) < voltron.at( 12 ) || voltron.at( 12 ) <= 0 );

    VEC_T thundercats( optimi );

    /* The following does not cause a fail, because the C library treats this
     * as a "bug" as opposed to an "error" and interactively asks the user
     * whether he would like to abort.  So the following test is no good:
        TEST_FAIL( thundercats += VEC_T(7,DATUM_T(0)) );
     */

    thundercats += VEC_T(12,DATUM_T(0));    // add zero vector
    TEST_TRUE( optimi == thundercats );     // verify it is the identity
    thundercats.add( VEC_T(12,DATUM_T(0)) );
    TEST_TRUE( optimi == thundercats );     // also the identity
    thundercats.add( optimi );
    TEST_TRUE( 2 * optimi == thundercats );
    thundercats -= optimi;
    TEST_TRUE( optimi == thundercats );
    thundercats.subtract( optimi );
    TEST_FALSE( optimi == thundercats );
    TEST_TRUE( VEC_T(12,DATUM_T(0)) == thundercats );
    TEST_TRUE( (thundercats = optimi - voltron) == -dtcons );
    thundercats.negate();
    TEST_TRUE( thundercats == dtcons );
    TEST_TRUE( kjb::max(optimi) == 37 );
    TEST_TRUE( kjb::min(optimi) == 2 );

    int index;
    TEST_TRUE( optimi.max( &index ) == 37 );
    TEST_TRUE( 11 == index );
    TEST_TRUE( optimi.min( &index ) == 2 );
    TEST_TRUE( 0 == index );
    TEST_TRUE( optimi.max(0) == 37 );
    TEST_TRUE( optimi.min(0) == 2 );
    TEST_FAIL( empty.max( &index ) );
    TEST_FAIL( empty.min( &index ) );
    TEST_FAIL( empty.max(0) );
    TEST_FAIL( empty.min(0) );

    TEST_TRUE( 34 == kjb::max_abs_difference( VEC_T(12,DATUM_T(36)), optimi ) );
    TEST_FAIL( 34 == kjb::max_abs_difference( VEC_T(13,DATUM_T(36)), optimi ) );
    TEST_TRUE( 3 == kjb::max_abs_difference( voltron, optimi ) );

    TEST_TRUE( 4136 == dot( voltron, optimi ) );

    TEST_TRUE( fabs( 60.4401 - voltron.magnitude() ) < 5e-5 );
    TEST_TRUE( 3653 == voltron.magnitude_squared() );

    return SUCCESS_CODE;
}

template< typename VEC_T>
void test_fail()
{
    using kjb::create_zero_matrix;
    using kjb::create_zero_vector;

    typedef typename VEC_T::Value_type DATUM_T;
    typedef typename VEC_T::Mat_type MAT_T;

    const DATUM_T primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37 };
    VEC_T optimi( 12, primes );

//    TEST_FAIL(optimi += VEC_T(100));
//    TEST_FAIL(optimi + VEC_T(100));
//    TEST_FAIL(optimi -= VEC_T(100));
//    TEST_FAIL(optimi - VEC_T(100));
    TEST_FAIL(dot(optimi, VEC_T(100)));
    TEST_FAIL(create_zero_matrix(100) * optimi);
}

int test_resize()
{
    // test resize bug
    {
        kjb::Vector v;
        v.resize(1);
    }
    static const int SZ = 10;

    kjb::Vector a10 = kjb::create_random_vector(SZ);
    kjb::Vector a11(a10);
    a11.resize(SZ + 1, 1234);

    TEST_TRUE(a11.back() == 1234);
    for(size_t i = 0; i < SZ; ++i)
        TEST_TRUE(a11[i] = a10[i]);

    const kjb_c::Vector* a11_vp = a11.get_c_vector();
    a11.resize(SZ);
    // actually resized?
    TEST_TRUE(a11.size() == SZ);
    // reusing storage?
    TEST_TRUE(a11_vp == a11.get_c_vector());

    for(size_t i = 0; i < SZ; ++i)
        TEST_TRUE(a11[i] = a10[i]);

    return SUCCESS_CODE;
}


} // anonymous namespace


int main()
{
    using kjb_c::ERROR;
    using kjb_c::kjb_debug_level;
    using kjb_c::add_error;

    try {
        // Usually I dislike reusing little objects like ints, but now I want to.
        // The multi-argument templates make me do it.
        int rc;
        //////////////////////////////////////////////////////////////////
        rc = test_ctors<kjb::Vector, int>();                    ERE( rc );
        rc = test_ctors<kjb::Vector, unsigned>();               ERE( rc );
        rc = test_ctors<kjb::Vector, unsigned long>();          ERE( rc );
        //////////////////////////////////////////////////////////////////
        rc = test_ctors<kjb::Int_vector, int>();                ERE( rc );
        rc = test_ctors<kjb::Int_vector, unsigned>();           ERE( rc );
        rc = test_ctors<kjb::Int_vector, unsigned long>();      ERE( rc );
        //////////////////////////////////////////////////////////////////

        ERE( test_ctors_2<kjb::Vector>() );
        ERE( test_ctors_2<kjb::Vector>() );
        ERE( test_ctors_2<kjb::Int_vector>() );

        ERE( test_ctor_3<kjb::Vector>() );
        ERE( test_ctor_3<kjb::Int_vector>() );

        ERE( test_ctor_4() );

        ERE( test_ctor_5f() );
        ERE( test_ctor_5i() );
        ERE(test_insert() );

        ERE( test_work_1() );

        ERE( test_work_2() );

        ERE( test_work_3<kjb::Vector>() );
        ERE( test_work_3<kjb::Int_vector>() );

        ERE( test_work_4<kjb::Vector>() );
        ERE( test_work_4<kjb::Int_vector>() );

        ERE( test_hat< kjb::Vector >() );
        ERE( test_hat< kjb::Int_vector >() );

        ERE( test_work_5f() );
        ERE( test_work_5i() );

        ERE( test_work_6<kjb::Vector>() );
        ERE( test_work_6<kjb::Int_vector>() );

        {
            kjb::Int_vector iv;
            iv.randomize(1000);
            kjb::Vector v(iv);

            test_serialization(iv);
            test_serialization(v);
        }

        test_fail<kjb::Vector>();
        test_fail<kjb::Int_vector>();

        ERE( test_resize() );
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
