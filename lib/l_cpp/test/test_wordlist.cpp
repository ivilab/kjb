/**
 * @file
 * @brief Test the Word_list functionality
 * @author Andrew Predoehl
 */

/*
 * $Id: test_wordlist.cpp 21356 2017-03-30 05:34:45Z kobus $
 */

#include "l/l_incl.h"
//include "l_cpp/l_cpp_incl.h"
#include "l_cpp/l_word_list.h"

#include <vector>
#include <string>
#include <cstdlib>

#ifndef HERE
#define HERE TEST_PSE(( "now at %s:%d\n", __FILE__, __LINE__ ));
#endif

#define WE_EXPECT( P )                                                      \
        if ( ! (P) )                                                        \
        {                                                                   \
            TEST_PSE(( "Test failed on line %d of function %s, "            \
                "file %s:\n\t%s\n", __LINE__, __func__, __FILE__, (#P)));   \
            kjb_c::kjb_exit( EXIT_FAILURE );                                \
        }                                                                   \
        else{ ; }

namespace {

int is_anything( const char* )
{
    return 1;
}

// search for a glob of ordinary files, and verify we have the correct set
void test_1()
{
    const size_t SIZE = 8;

    kjb::Word_list wl1( "input/nobody/file*" );
    WE_EXPECT( wl1.size() == SIZE );

    if ( kjb_c::is_interactive() )
    {
        kjb_c::pso( "wl size is %d\n", wl1.size() );
        for( size_t iii = 0; iii < wl1.size(); ++iii )
        {
            kjb_c::pso( "%s\n", wl1[ iii ] );
        }
    }

    std::vector< std::string > u, v(SIZE);
    v[0] = std::string("input/nobody/file");
    v[1] = std::string("input/nobody/file_a");
    v[2] = std::string("input/nobody/file_b");
    v[3] = std::string("input/nobody/file_c");
    v[4] = std::string("input/nobody/file_d");
    v[5] = std::string("input/nobody/file_e");
    v[6] = std::string("input/nobody/file_f");
    v[7] = std::string("input/nobody/file_g");

    u = wl1;
    for( size_t iii = 0; iii < SIZE; ++iii )
    {
        WE_EXPECT( u[ iii ] == v[ iii ] );
    }

    size_t hit = wl1.match("input/nobody/file_d");
    WE_EXPECT( hit == 4 );
    size_t miss = wl1.match("bad");
    WE_EXPECT( miss == wl1.size() );
}


// same as test_1 except we search for directories.
void test_2()
{
    const size_t SIZE = 3;
    const char* pointy = "porcupine";

    kjb::Word_list wl2( "input/nobody/file*", kjb_c::is_directory );
    kjb::Word_list wm2( wl2 ), wn2( 17 );

    WE_EXPECT( wl2.size() == SIZE );

    if ( kjb_c::is_interactive() )
    {
        kjb_c::pso( "wl size is %d\n", wl2.size() );
        for( size_t iii = 0; iii < wl2.size(); ++iii )
        {
            kjb_c::pso( "%s\n", wl2[ iii ] );
        }
    }

    std::vector< std::string > u, v(SIZE);
    v[0] = std::string("input/nobody/file_h");
    v[1] = std::string("input/nobody/file_i");
    v[2] = std::string("input/nobody/file_j");

    u = wl2;
    for( size_t iii = 0; iii < SIZE; ++iii )
    {
        WE_EXPECT( u[ iii ] == v[ iii ] );
    }

    size_t hit = wl2.match("input/nobody/file_h");
    WE_EXPECT( hit == 0 );
    size_t miss = wl2.match("bad");
    WE_EXPECT( miss == wl2.size() );


    /*
     * Bonus section:  test some other stuff
     */

    // test appending a string, causing size to enlarge (unspecified how much).
    wl2.append(pointy);
    WE_EXPECT( SIZE < wl2.size() );
    // appended string should appear in the first newly-created space.
    WE_EXPECT( wl2.match(pointy) == SIZE );
    // previous search behavior should remain consistent.
    WE_EXPECT( wl2.match("bad") == wl2.size() );
    WE_EXPECT( wl2.match("input/nobody/file_h") == 0 );


    // test copy ctor
    WE_EXPECT( wm2.size() == SIZE );
    for( size_t iii = 0; iii < SIZE; ++iii )
    {
        WE_EXPECT( u[ iii ] == wl2[ iii ] );
        WE_EXPECT( u[ iii ] == wm2[ iii ] );
    }
    WE_EXPECT( wm2.match(pointy) == wm2.size() ); // no porcupine in wm2


    // test assignment
    wn2 = wl2;
    WE_EXPECT( wl2.match(pointy) < wl2.size() );
    WE_EXPECT( wn2.match(pointy) < wn2.size() );
    WE_EXPECT( wn2.match("bad") == wn2.size() );


    // test concatenation
    kjb::Word_list wp2( wl2 + wm2 );
    WE_EXPECT( wp2.size() == wl2.size() + wm2.size() );
    WE_EXPECT( 0 == strcmp( wl2[ 0 ], wp2[ 0 ] ) );
    WE_EXPECT( 0 == strcmp( wm2[ 0 ], wp2[ wl2.size() ] ) );

    // test empties-in-middle postulate (described in C comments)
    wp2.append("sparrow"); // wp2 had empties in middle, so sparrow lands there
    WE_EXPECT( wp2.match("sparrow") == 1+SIZE ); // just after porcupine!
    WE_EXPECT( 1+SIZE < wl2.size() );
    WE_EXPECT( 1+SIZE == wl2.count_strings() );

    // test append some more
    kjb::Word_list xyz( 10 );
    WE_EXPECT( 10 == xyz.size() );
    WE_EXPECT( 0 == xyz.count_strings() );
    xyz.append( "alpha" ); xyz.append( "beta" ); xyz.append( "gamma" );
    WE_EXPECT( 10 == xyz.size() );
    WE_EXPECT( 3 == xyz.count_strings() );
    xyz.trim_empty_entries_at_tail();
    WE_EXPECT( 3 == xyz.size() );
    WE_EXPECT( 3 == xyz.count_strings() );
}


// search for any named things in the file system -- files, dirs, sockets, etc.
void test_3()
{
    const size_t SIZE = 11;

    kjb::Word_list wl3( "input/nobody/file*", is_anything );
    WE_EXPECT( wl3.size() == SIZE );

    if ( kjb_c::is_interactive() )
    {
        kjb_c::pso( "wl size is %d\n", wl3.size() );
        for( size_t iii = 0; iii < wl3.size(); ++iii )
        {
            kjb_c::pso( "%s\n", wl3[ iii ] );
        }
    }

    std::vector< std::string > u, v(SIZE);
    v[0] = std::string("input/nobody/file");
    v[1] = std::string("input/nobody/file_a");
    v[2] = std::string("input/nobody/file_b");
    v[3] = std::string("input/nobody/file_c");
    v[4] = std::string("input/nobody/file_d");
    v[5] = std::string("input/nobody/file_e");
    v[6] = std::string("input/nobody/file_f");
    v[7] = std::string("input/nobody/file_g");
    v[8] = std::string("input/nobody/file_h");
    v[9] = std::string("input/nobody/file_i");
    v[10]= std::string("input/nobody/file_j");

    u = wl3;
    for( size_t iii = 0; iii < SIZE; ++iii )
    {
        WE_EXPECT( u[ iii ] == v[ iii ] );
    }

    size_t hit = wl3.match("input/nobody/file_h");
    WE_EXPECT( hit == 8 );
    size_t miss = wl3.match("bad");
    WE_EXPECT( miss == wl3.size() );
}


// test iterators
void test_4()
{
    kjb::Word_list wl4( "input/nobody/file*" );

    // test that reference type works (it was a pain to get right)
    kjb::Word_list::const_iterator::reference wdi = *wl4.begin();
    WE_EXPECT( 0 == strcmp( "input/nobody/file", wdi ) );

    // the predicate is explained in sec. 18.4.4.4 of Stroustrup, TC++PL/3e
    kjb::Word_list::const_iterator ci = std::find_if( wl4.begin(), wl4.end(),
        std::not1( std::bind2nd( std::ptr_fun( strcmp ), "input/nobody/file_f"
        ) ) );

    // test that the iterator emitted by find_if really points to what we asked
    WE_EXPECT( ci != wl4.end() );
    WE_EXPECT( 0 == strcmp( "input/nobody/file_f", *ci ) );

    // test that this iterator can be incremented, and hits the end as expected
    ++ci;
    WE_EXPECT( ci != wl4.end() );
    WE_EXPECT( 0 == strcmp( "input/nobody/file_g", *ci ) );
    ++ci;
    WE_EXPECT( ! (ci != wl4.end()) );

    // test that a begin() iterator really begins at the beginning
    WE_EXPECT( 0 == strcmp( "input/nobody/file", * wl4.begin() ) );

    // test that find fails for strings not in the list
    kjb::Word_list::const_iterator cj = std::find( wl4.begin(), wl4.end(),
                                                                "impossible" );
    WE_EXPECT( ! (cj != wl4.end()) ); // i.e., it DOES equal end
}


// test argc/argv style ctor
void test_5()
{
    const char* argv[] = { "cat", "in", "the", "hat", 0 };
    kjb::Word_list seuss( 4, argv );
    WE_EXPECT( 4 == seuss.size() );
    WE_EXPECT( 4 == seuss.count_strings() );
    WE_EXPECT( 0 == strcmp( seuss[0], "cat" ) );
    WE_EXPECT( 0 == strcmp( seuss[1], "in" ) );
    WE_EXPECT( 0 == strcmp( seuss[2], "the" ) );
    WE_EXPECT( 0 == strcmp( seuss[3], "hat" ) );
}


} // anon. ns

int main()
{
    try 
    {
        test_1();
        test_2();
        test_3();
        test_4();
        test_5();
    }
    catch( kjb::Exception& e )
    {
        e.print_details_exit();
    }

    if ( kjb_c::is_interactive() )
    {
        kjb_c::kjb_fputs( stdout, "Success!\n" );
    }

    return EXIT_SUCCESS;
}
