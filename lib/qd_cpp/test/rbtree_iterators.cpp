/**
 * @file
 * @author Andrew Predoehl
 * @brief unit test program for red-black tree iterators
 */
/*
 * $Id: rbtree_iterators.cpp 20160 2015-12-08 23:36:20Z predoehl $
 */

#include <l/l_init.h>
#include <l/l_error.h>
#include <l/l_global.h>
#include <l_cpp/l_test.h>
#include <qd_cpp/redblack.h>

#define TEST_RBTREE_ITER_EXTRA_DEBUG 0

#if TEST_RBTREE_ITER_EXTRA_DEBUG
#include <iostream> /* just for debug */
#endif

#include <vector>

namespace
{

/// @brief dictionary type used for this test
typedef kjb::qd::Redblack_subtree_sum< const char* > Tree;

std::vector< Tree::Key_tp > keys;
std::vector< const char* > strings;

/// @brief insert a record in a tree then test its validity
Tree::Loc_tp do_a_thing( Tree* tree, float key, const char* str )
{
    TEST_TRUE( tree );
    Tree::Loc_tp loc = tree -> insert( key, str );
    keys.push_back(key);
    strings.push_back(str);

#if TEST_RBTREE_ITER_EXTRA_DEBUG
    tree -> debug_print( std::cerr );
    std::cerr << "\n\n******************************\n\n";
#endif

    TEST_TRUE( tree -> tree_valid_in_nlogn_time() );
    return loc;
}



/// @brief test locators in a small tree
int test1()
{
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

    std::vector< const char* > s;
    std::vector< Tree::Key_tp > k;

    // pull data back out of the tree via its const_iterator
    for ( Tree::const_iterator i = derevo.begin(); i != derevo.end(); )
    {
        const char *str;
        Tree::Key_tp key;
        bool found = derevo.access_loc(*i++, &key, &str);
        TEST_TRUE(found);
        s.push_back(str);
        k.push_back(key);
    }
    std::sort(s.begin(), s.end());
    TEST_TRUE(std::equal(s.begin(), s.end(), strings.begin()));

    // If the red-black tree uses the array implementation of locator_list, the
    // keys also must be sorted, because iterator does not enumerate them
    // in sorted order; the order is by locator sequence, which is generally
    // unpredictable -- in chronological order if no deletions occur,
    // otherwise the order is difficult to describe.
#if TEST_RBTREE_ITER_EXTRA_DEBUG
    std::cout << "\nK:\n";
    std::copy(k.begin(), k.end(),
              std::ostream_iterator<Tree::Key_tp>(std::cout, ", "));
    std::cout << "\nKeys:\n";
    std::copy(keys.begin(), keys.end(),
              std::ostream_iterator<Tree::Key_tp>(std::cout, ", "));

    std::cout << "\n"
          "(disregard record order, just presence or absence, like a set.)\n";
    derevo.debug_print(std::cout);
#endif
    std::sort(k.begin(), k.end());
    std::sort(keys.begin(), keys.end());
    TEST_TRUE(std::equal(k.begin(), k.end(), keys.begin()));

    return kjb_c::NO_ERROR;
}



} // end anonymous ns

int main( int argc, const char* const* argv )
{
    KJB(EPETE(kjb_init()));
    KJB(EPETE(test1()));
    kjb_c::kjb_cleanup();
    RETURN_VICTORIOUSLY();
}

