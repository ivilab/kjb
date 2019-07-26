/**
 * @file
 * @author Andrew Predoehl
 * @brief unit test program for red-black tree storing subtree sums
 *
 * This tests tree structure but it relies heavily on the tree's own internal
 * invariant checking.  It does lots of operations and then expects the tree to
 * check itself for correctness; in that sense it is a kind of disappointing
 * test program.
 */
/*
 * $Id: test_redblack.cpp 20087 2015-11-16 23:42:44Z predoehl $
 */

#include <l/l_init.h>
#include <l/l_error.h>
#include <l/l_global.h>
#include <l_cpp/l_test.h>
#include <qd_cpp/redblack.h>

#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <sstream>

#define VERBOSE 0   /**< if pos. then provide wordy explanatory output */

namespace {

int ALOT = -1;

/// @brief dictionary type used for this test
typedef kjb::qd::Redblack_subtree_sum< const char* > Tree;

/// @brief insert a record in a tree then test its validity
Tree::Loc_tp do_a_thing( Tree* tree, float key, const char* str )
{
    TEST_TRUE( tree );
    Tree::Loc_tp loc = tree -> insert( key, str );
#if VERBOSE
    tree -> debug_print( std::cerr );
    std::cerr << "\n\n******************************\n\n";
#endif
    TEST_TRUE( tree -> tree_valid_in_nlogn_time() );
    return loc;
}


/// @brief remove a key from a tree then test the validity of its structure
void removal( Tree* tree, float key )
{
    TEST_TRUE( tree );
#if REDBLACK_H_CHECK_LOTS_OF_INVARIANTS && VERBOSE
    std::cerr << "\n\n======== REMOVING " << key << " ============\n\n";
#endif
    tree -> erase_key( key, 00 );
#if VERBOSE
    tree -> debug_print( std::cerr );
#endif
    TEST_TRUE( tree -> tree_valid_in_nlogn_time() );
}


/// @brief verify the accuracy of a locator
void loc_check( const Tree& tree, Tree::Loc_tp loc, Tree::Key_tp in_key )
{
    Tree::Key_tp out_key;
    bool rc = tree.access_loc( loc, & out_key, 00 );
    TEST_TRUE( rc );
    TEST_TRUE( in_key == out_key ); // floating point equality is chancy!
}


/// @brief test locators in a small tree
int test1()
{
    TEST_TRUE( 0 < ALOT );

    Tree derevo;
    derevo.clear();

    std::vector< Tree::Loc_tp > locvec( 9 );
    std::vector< Tree::Loc_tp >::iterator ppp = locvec.begin();

    *ppp++ = do_a_thing( & derevo, 120, "monkeys" );
    *ppp++ = do_a_thing( & derevo, 50, "ez pcs" );
    *ppp++ = do_a_thing( & derevo, 0, "exit" );
    *ppp++ = do_a_thing( & derevo, 130, "friday" );
    *ppp++ = do_a_thing( & derevo, 20, "timer" );
    *ppp++ = do_a_thing( & derevo, 80, "enough" );
    *ppp++ = do_a_thing( & derevo, 60, "omen" );
    *ppp++ = do_a_thing( & derevo, 30, "little pigs" );
    *ppp++ = do_a_thing( & derevo, 40, "seasons" );
    TEST_TRUE( locvec.end() == ppp );

    ppp = locvec.begin();
    loc_check( derevo, *ppp++, 120 );
    loc_check( derevo, *ppp++, 50 );
    loc_check( derevo, *ppp++, 0 );
    loc_check( derevo, *ppp++, 130 );
    loc_check( derevo, *ppp++, 20 );
    loc_check( derevo, *ppp++, 80 );
    loc_check( derevo, *ppp++, 60 );
    loc_check( derevo, *ppp++, 30 );
    loc_check( derevo, *ppp++, 40 );
    TEST_TRUE( locvec.end() == ppp );

    TEST_TRUE( 9 == derevo.size() );
    derevo.clear();
    TEST_TRUE( 0 == derevo.size() );

    return EXIT_SUCCESS;
}


void audit( const Tree& tree, int bad_line )
{
    bool valid = tree.tree_valid_in_nlogn_time();
    if ( ! valid ) {
        tree.debug_print( std::cerr );
        std::cerr << __FILE__ << ": Tree bad from line " << bad_line << '\n';
    }
    TEST_TRUE( valid );
}



/// @brief return a tree with a bunch of inserted records
std::auto_ptr< Tree > test2( bool make_a_copy = false )
{
    std::auto_ptr< Tree > tree( new Tree() );
    do_a_thing( tree.get(), 0, "a" );
    do_a_thing( tree.get(), 10, "b" );
    do_a_thing( tree.get(), 20, "c" );
    do_a_thing( tree.get(), 30, "d" );
    do_a_thing( tree.get(), 40, "e" );
    do_a_thing( tree.get(), 50, "f" );
    do_a_thing( tree.get(), 60, "g" );
    do_a_thing( tree.get(), 70, "h" );
    do_a_thing( tree.get(), 80, "i" );
    do_a_thing( tree.get(), 90, "j" );
    do_a_thing( tree.get(), 100, "k" );
    do_a_thing( tree.get(), 110, "l" );
    do_a_thing( tree.get(), 120, "m" );
    do_a_thing( tree.get(), 130, "n" );
    do_a_thing( tree.get(), 140, "o" );
    do_a_thing( tree.get(), 150, "p" );
    do_a_thing( tree.get(), 160, "q" );
    do_a_thing( tree.get(), 170, "r" );
    do_a_thing( tree.get(), 180, "s" );
    do_a_thing( tree.get(), 190, "t" );
    do_a_thing( tree.get(), 200, "u" );
    do_a_thing( tree.get(), 210, "v" );
    do_a_thing( tree.get(), 220, "w" );
    do_a_thing( tree.get(), 230, "x" );
    do_a_thing( tree.get(), 240, "y" );
    do_a_thing( tree.get(), 250, "z" );
    do_a_thing( tree.get(), 260, "aa" );
    do_a_thing( tree.get(), 270, "ab" );

    if (make_a_copy)
    {
        Tree copycat( *tree ); // make a copy, just to exercise the copy ctor.
        audit( copycat, __LINE__ );
        Tree copy2cat, copy3cat(copy2cat);
        copy2cat = *tree; // exercise the assignment operator also.
        audit( copy2cat, __LINE__ );
        copy3cat = copy3cat; // self-assignment should be inocuous.
        audit( copy3cat, __LINE__ );
        copy3cat = copycat; // assignment but give it something to discard too.
        audit( copy3cat, __LINE__ );
        // now throw away the copies
    }

    return tree;
}

/// @brief test the tree by deleting elements down to empty in random order
int test3()
{
    TEST_TRUE( 0 < ALOT );

    std::auto_ptr< Tree > tp;

    // empirically found troublemaker sequence 1
    // bug fixed:  defs of far_newphew, near_nephew were transposed.
    // bug fixed:  bad copy ctor (was untested, for about two and a half years)
    tp = test2( true );
    float test_vec1[] = { 40, 100, 110, 150, 240, -1 };
    for( size_t iii = 0; 0 <= test_vec1[ iii ]; ++iii )
        removal( tp.get(), test_vec1[ iii ] );

    // empirically found troublemaker sequence 2
    // bug fixed:  bad assumption about non-nil status of xblack
    float test_vec2[] = { 40, 100, 110, 150, 140, 160, 170, -1 };
    tp = test2();
    for( size_t iii = 0; 0 <= test_vec2[ iii ]; ++iii )
        removal( tp.get(), test_vec2[ iii ] );

    // random hitlists
    for( int jjj = 0; jjj < ALOT; ++jjj ) {
#if REDBLACK_H_CHECK_LOTS_OF_INVARIANTS && VERBOSE
        std::cerr << " jjj = " << jjj << "################################\n";
#endif
        tp = test2();
        std::vector< float > hitlist( tp -> size() );
        for( size_t iii = 0; iii < tp -> size(); ++iii )
            hitlist[ iii ] = 10.0 * iii;
        std::random_shuffle( hitlist.begin(), hitlist.end() );
        for( size_t iii = 0; iii < hitlist.size(); ++iii )
            removal( tp.get(), hitlist[ iii ] );
    }
    return EXIT_SUCCESS;
}

/// @brief helper class used for characterizing tree node test data.
struct TN { float k; const char* s; Tree::Loc_tp l; };


/// @brief test the tree with randomized insertions, queries, deletions
int test4()
{
    TEST_TRUE( 0 < ALOT );

    TN stuff[] = {
        /*  key     string      loc */
        {   0,      "a",        0 },
        {   10,     "b",        0 },
        {   20,     "c",        0 },
        {   30,     "d",        0 },
        {   40,     "e",        0 },
        {   50,     "f",        0 },
        {   60,     "g",        0 },
        {   70,     "h",        0 },
        {   80,     "i",        0 },
        {   90,     "j",        0 },
        {   100,    "k",        0 },
        {   110,    "l",        0 },
        {   120,    "m",        0 },
        {   130,    "n",        0 },
        {   140,    "o",        0 },
        {   150,    "p",        0 },
        {   160,    "q",        0 },
        {   170,    "r",        0 },
        {   180,    "s",        0 },
        {   190,    "t",        0 },
        {   200,    "u",        0 },
        {   210,    "v",        0 },
        {   220,    "w",        0 },
        {   230,    "x",        0 },
        {   240,    "y",        0 },
        {   250,    "z",        0 },
        {   260,    "aa",       0 },
        {   270,    "ab",       0 },
    };
    const size_t tnsz = sizeof stuff / sizeof( TN );

    for( int rrr = 0; rrr < ALOT; ++rrr ) {
        std::vector< TN > records( stuff, stuff + tnsz );
        std::random_shuffle( records.begin(), records.end() );

        // build the tree
        Tree tree;
        for( size_t iii = 0; iii < tnsz; ++iii ) {
            const TN& tn( records[ iii ] );
            Tree::Loc_tp loc = tree.insert( tn.k, tn.s );
            records[ iii ].l = loc;
        }
        TEST_TRUE( tree.size() == tnsz );

        // query the tree
        std::random_shuffle( records.begin(), records.end() );
        for( size_t iii = 0; iii < tnsz; ++iii ) {
            TN tn = { 0, 00, records[ iii ].l };
            bool rc = tree.access_loc( tn.l, & tn.k, & tn.s );
            TEST_TRUE( rc );
            TEST_TRUE( records[ iii ].k == tn.k );
            TEST_TRUE( records[ iii ].s == tn.s );
            TEST_TRUE( tree.size() == tnsz );
        }

        // delete from the tree
        std::random_shuffle( records.begin(), records.end() );
        for( size_t iii = 0; iii < tnsz; ++iii ) {
            TN tn = { 0, 00, records[ iii ].l };
            TEST_TRUE( tree.size() == tnsz - iii );
            bool rc = tree.erase_loc( tn.l );
            TEST_TRUE( rc );
            bool rc2 = tree.erase_loc( tn.l );
            TEST_TRUE( ! rc2 );
        }
        TEST_TRUE( tree.size() == 0 );
    }
    return EXIT_SUCCESS;
}

/// @brief return a random float value in interval [0,1] not sure if 0, 1 poss.
float randf()
{
    return rand() / float( RAND_MAX );
}


/// @brief test the tree with random mix of insertions, fetches, and deletions
int test5()
{
    TEST_TRUE( 0 < ALOT );

    Tree tree;
    std::vector< TN > recs;
    const size_t CRIT = 1000;

    for( int rrr = 0; rrr < ALOT; ++rrr ) {
#if VERBOSE
        tree.debug_print( std::cerr );
#endif

        // randomly insert a key
        if (        0 == recs.size()
                ||  (recs.size() < CRIT && randf() < 0.8) // redundant parens
                ||  randf() < 0.6 ) {
            TN tn;
            tn.s = "cows";
            tn.k = randf();
            tn.l = tree.insert( tn.k, tn.s );
            recs.push_back( tn );
            TEST_TRUE( tree.size() == recs.size() );
            #if VERBOSE
                std::cerr << 'i' << recs.size();
            #endif
            audit( tree, __LINE__ );
        }

        // randomly query a key
        if ( recs.size() && randf() < 0.5 ) {
            size_t ix = recs.size() * randf();
            TN &tn = recs.at( ix );
            TN xx;
            tree.access_loc( tn.l, & xx.k, 00 );
            TEST_TRUE( xx.k == tn.k );
            TEST_TRUE( tree.size() == recs.size() );
            #if VERBOSE
                std::cerr << 'q';
            #endif
            audit( tree, __LINE__ );
        }

        // randomly delete a key
        if ( recs.size() )
            if (    (CRIT < recs.size() && randf() < 0.8) // redundant parens
                ||  randf() < 0.6 ) {
                size_t ix = recs.size() * randf();
                TN &tn = recs.at( ix );
                bool rc = tree.erase_loc( tn.l );
                if ( ix + 1 < recs.size() )
                    recs.at( ix ) = recs.back();
                recs.pop_back();
                TEST_TRUE( rc );
                TEST_TRUE( tree.size() == recs.size() );
                #if VERBOSE
                    std::cerr << 'd';
                #endif
                audit( tree, __LINE__ );
            }
    }
    return EXIT_SUCCESS;
}


// test rekeying
int test6()
{
    TEST_TRUE( 0 < ALOT );

    std::auto_ptr< Tree > tp;

    // get a nice example tree, with key/sat pairs e/40, k/100, p/150, others.
    tp = test2();
    TEST_TRUE( tp -> tree_valid_in_nlogn_time() );

    Tree::Loc_tp eloc, ploc;
    const char *esa = 00, *psa = 00;
    bool    s1 = tp -> find_key( 40, &esa, &eloc ),
            s2 = tp -> find_key( 150, &psa, &ploc );
    TEST_TRUE( s1 && s2 );
    TEST_TRUE( 'e' == esa[0] );
    TEST_TRUE( 'p' == psa[0] );
    TEST_TRUE( tp -> tree_valid_in_nlogn_time() );

    // rekey node e/40 to e/100
    bool s3 = tp -> rekey_loc( eloc, 100 );
    TEST_TRUE( s3 );
    TEST_TRUE( tp -> tree_valid_in_nlogn_time() );

    // rekey node p/150 to p/100
    bool s4 = tp -> rekey_loc( ploc, 100 );
    TEST_TRUE( s4 );
    TEST_TRUE( tp -> tree_valid_in_nlogn_time() );

    // "try" to delete keys 40, 150 -- that should no longer work
    bool s5 = tp -> erase_key( 40, 00 );
    TEST_TRUE( ! s5 );
    bool s6 = tp -> erase_key( 150, 00 );
    TEST_TRUE( ! s6 );

    // try to delete key 100 four times; first 3 tries should succeed, then not
    const char *x1, *x2, *x3, *x4;
    x1 = x2 = x3 = x4 = 00;
    bool s7 = tp -> erase_key( 100, &x1 );
    bool s8 = tp -> erase_key( 100, &x2 );
    bool s9 = tp -> erase_key( 100, &x3 );
    bool s10 = tp -> erase_key( 100, &x4 );
    TEST_TRUE( s7 && s8 && s9 && !s10 );
    TEST_TRUE( x1 && x2 && x3 && 00 == x4 );

    // verify that the set of satellite data that came out is {e,k,p}
    std::vector<char> ekp(3);
    ekp[0] = *x1;
    ekp[1] = *x2;
    ekp[2] = *x3;
    std::sort( ekp.begin(), ekp.end() ); // force results into ascending order
    TEST_TRUE( ekp[0] == 'e' );
    TEST_TRUE( ekp[1] == 'k' );
    TEST_TRUE( ekp[2] == 'p' );

    return EXIT_SUCCESS;
}


// test creating an empty tree, copying it, and deleting both.
// This is a trivial test, but less trivial if you run it with valgrind.
int test7()
{
    Tree* p1 = new Tree, *p2 = new Tree(*p1);
    delete p1;
    p2 -> insert( 3.148873, "abble pie" );
    delete p2;
    return EXIT_SUCCESS;
}


// test swap -- NO, SWAP DOES NOT WORK
int test8()
{
#if 0
    const size_t SZ = 100;
    typedef kjb::qd::Redblack_subtree_sum< int > T;
    T t1, t2;
    std::vector< int > v1(SZ), v2(SZ);
    std::vector< size_t > l1(SZ), l2(SZ);
    for (size_t i = 0; i < SZ; ++i)
    {
        v1[i] = rand();
        v2[i] = rand();
        l1[i] = t1.insert(v1[i], v1[i]);
        l2[i] = t2.insert(v2[i], v2[i]);
    }

    // verify those satellite values are really in there.
    size_t i = 0;
    for (T::const_iterator j = t1.begin(); j != t1.end(); ++i)
    {
        int r;
        const T::Loc_tp l = *j++;
        TEST_TRUE( l1[i] == l );
        TEST_TRUE( t1.access_loc(l, 0, &r) );
        TEST_TRUE( v1[i] == r );
    }
    i = 0;
    for (T::const_iterator j = t2.begin(); j != t2.end(); ++i)
    {
        int r;
        const T::Loc_tp l = *j++;
        TEST_TRUE( l2[i] == l );
        TEST_TRUE( t2.access_loc(l, 0, &r) );
        TEST_TRUE( v2[i] == r );
    }

    TEST_TRUE( t1.tree_valid_in_nlogn_time() );
    TEST_TRUE( t2.tree_valid_in_nlogn_time() );
    std::cout << "pre swap\n";

    // Tell trees to swap their internal contents, and see that they changed.
    t2.debug_print( std::cout );
    t1.swap(t2);
    std::cout << "post swap\n";
    t1.debug_print( std::cout );
    TEST_TRUE( t1.tree_valid_in_nlogn_time() );
    TEST_TRUE( t2.tree_valid_in_nlogn_time() );

    // If you doubt that this test is "probative," try omitting the swap.
    i = 0;
    for (T::const_iterator j = t1.begin(); j != t1.end(); ++i)
    {
        int r;
        const T::Loc_tp l = *j++;
        std::cout << "l=" << l << ", ref=" << l2[i] << '\n';
        TEST_TRUE( l2[i] == l );
        TEST_TRUE( t1.access_loc(l, 0, &r) );
        TEST_TRUE( v2[i] == r );
    }
    i = 0;
    for (T::const_iterator j = t2.begin(); j != t2.end(); ++i)
    {
        int r;
        const T::Loc_tp l = *j++;
        TEST_TRUE( l1[i] == l );
        TEST_TRUE( t2.access_loc(l, 0, &r) );
        TEST_TRUE( v1[i] == r );
    }
#endif
    return EXIT_SUCCESS;
}



// test "deep deletion" cycles, which force the structure to morph
// from linear array to tree and back to linear array.  The latter
// switch needs to be tested thoroughly, both for a random deletion
// pattern and in a LIFO and FIFO pattern of insertion/deletion.
int test9()
{
    const size_t N = 1<<14;
    std::vector< Tree::Loc_tp > l(N);
    Tree tree;
    for (size_t i = 0; i < N; ++i)
    {
        l[i] = tree.insert( rand(), "" );
    }
    audit(tree, __LINE__);
    std::random_shuffle(l.begin(), l.end());
    for (size_t i = 0; i < N-2; ++i)
    {
        KJB(ASSERT(! l.empty()));
        TEST_TRUE(tree.erase_loc(l.back()));
        l.pop_back();
    }
    audit(tree, __LINE__);

    // do it again without the shuffle.  (LIFO test)
    tree.clear();
    audit(tree, __LINE__);
    l.clear();
    for (size_t i = 0; i < N; ++i)
    {
        l.push_back( tree.insert( rand(), "" ) );
    }
    audit(tree, __LINE__);
    for (size_t i = 0; i < N-2; ++i)
    {
        KJB(ASSERT(! l.empty()));
        TEST_TRUE(tree.erase_loc(l.back()));
        l.pop_back();
    }
    audit(tree, __LINE__);

    // do it again, FIFO.
    tree.clear();
    audit(tree, __LINE__);
    l.clear();
    for (size_t i = 0; i < N; ++i)
    {
        l.push_back( tree.insert( rand(), "" ) );
    }
    audit(tree, __LINE__);
    std::reverse(l.begin(), l.end());
    for (size_t i = 0; i < N-2; ++i)
    {
        KJB(ASSERT(! l.empty()));
        TEST_TRUE(tree.erase_loc(l.back()));
        l.pop_back();
    }
    audit(tree, __LINE__);
    return EXIT_SUCCESS;
}



/// @brief test the red-black tree.  Optional argument argv[1].
int main2( int argc, const char* const* argv )
{
    /*
     * if argv[1] exists it can be zero or a positive decimal integer.
     * If it is present and zero, then this test will be quick and cursory.
     * If it is absent or positive, the test will take a LONG time.
     */
    typedef int (*PTest)(void);

    PTest suite[] = { test1, test3, test4, test5,
                      test6, test7, test8, test9, 00 };

    // update the number of test iterations based on the optional argument
    int test_factor = 1;
    KJB(EPETE(scan_time_factor(argv[1], &test_factor)));

    for( ALOT = 100; test_factor > 0; --test_factor )
    {
        ALOT *= 100;
    }

    // perform all the tests
    srand( 7654321 );
    for( PTest* p = suite; *p; ++p )
    {
        int rc = (*p)();
        if ( rc != EXIT_SUCCESS )
        {
            KJB(TEST_PSE(( "failure in test index %d.\n", p - suite )));
            return rc;
        }
    }

    RETURN_VICTORIOUSLY();
}

} // end anonymous ns


int main( int argc, const char* const* argv )
{
    int rc;
    KJB(EPETE(kjb_init()));
    try
    {
        rc = main2( argc, argv );
    }
    catch (const kjb::Exception& e)
    {
        e.print_details_exit();
    }
    kjb_c::kjb_cleanup();
    return rc;
}

