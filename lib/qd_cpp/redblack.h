/**
 * @file
 * @author Andrew Predoehl
 * @brief definition of class Redblack_subtree_sum.
 *
 * Recently I've been concerned about the potential for bad performance when a
 * tree that was once very full becomes almost empty.  The array implementation
 * of location_list could be as big as the big tree in that case, which is bad.
 * As a consequence I added a map implementation of location_list, which is
 * much cleaner, but now access to a record via its locator uses O(log n)
 * time, where n is the tree size, rather than O(1) time when it was an array.
 * In tests, the array version uses about half the time as the map version, in
 * production-mode use.  But the complexity of the array version is ugly.
 */
/*
 * $Id: redblack.h 22177 2018-07-14 18:38:32Z kobus $
 */

#ifndef REDBLACK_H_PREDOEHL_12_DEC_2011_VISION
#define REDBLACK_H_PREDOEHL_12_DEC_2011_VISION 1

#include <l/l_sys_lib.h>
#include <l/l_sys_io.h>
#include <l/l_sys_debug.h>
#include <l/l_debug.h>
#include <l/l_error.h>
#include <l_cpp/l_util.h>

#ifdef DEBUGGING
#include <l/l_error.h>
#include <l/l_global.h>
#include <sstream>
#include <map> /* oh the irony */
#endif

#include <qd_cpp/diprique.h>

#include <iosfwd>                       /* std::ostream                  */
#include <algorithm>                    /* std::sort and several others  */
#include <numeric>                      /* std::accumulate               */
#include <limits>                       /* std::numeric_limits           */
#include <new>
#include <iterator>

/** Set this macro to 0 or 1 to select the implementation.  1 is faster. */
#define REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY 1

#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
/* Implement location list in a std::vector. */
#include <vector>
#else
/* Implement location list in a std::map. */
#include <map>
#endif

namespace kjb
{
namespace qd
{


/**
 * @brief a balanced binary search tree storing subtree sums in each node.
 *
 * This is a so-called Red-Black tree, thus it maintains a height of O(log n)
 * where n is the number of nodes.
 *
 * @section redblackinv     Red-Black Tree Invariants
 * A Red-Black Tree is a binary search tree augmented with one bit of "color"
 * information associated with each node.  Each node is either red or black.
 * Keys are stored only in internal nodes, not in the leaves.
 * The critical properties of a Red-Black tree are these:
 * @subsection redblacki1   Root and all leaves are black.
 * @subsection redblacki2   No-Red-Red invariant
 *                          Every red node has two black children.
 * @subsection redblacki3   Black-height invariant
 *                          If p, q are simple descending paths starting from
 *                          the same node and ending at leaves, then p and q
 *                          include the same number of black nodes.
 *
 * @subsection blackheight  Black height
 * The final invariant guarantees that the tree has a well-defined value called
 * the black height, which is the number of black nodes from the root to any
 * leaf, along a simple path of descent.
 *
 * @section rbfeatures Other features of this Red-Black tree
 *
 * @subsection rbsubsum Subtree Sums
 *
 * The tree not only maintains the Red-Black invariants, but it stores
 * real-valued keys and stores in each node the sum of the keys in the subtree
 * rooted at that node.
 *
 * @subsection rbsat Satellite data
 *
 * This is a class template, actually.  The template is parameterized by one
 * type, the satellite data that you want to store in each node.
 * The type for the satellite data must be copyable and it must have a default
 * constructor, just like the requirements for standard containers.
 *
 * @subsection rbmultikey Keys are not constrained to be unique
 *
 * This dictionary allows the user to insert using the same key value as
 * already found in the tree.  Both sets of satellite data will be stored.
 * An erase operation on a key that exists in the tree multiple times will
 * cause one of those nodes to be extracted and removed, but it is impossible
 * to say which one it will be (it does not in general follow a LIFO or FIFO
 * rule).
 *
 * @subsection rbloc Locators
 *
 * Functionally, a "locator" is an opaque pointer (see @ref rbrefs).  This tree
 * offers the user locators that point to the nodes in the tree.  The user can
 * read, modify, and delete nodes using their corresponding locators.
 *
 * The tree data structure returns a locator every time a new record is
 * created and inserted into the tree.  The user can then read the node (@see
 * access_loc), erase it (@see erase_loc), or alter its key value (@see
 * rekey_loc) via reference to its locator.
 *
 * The ability to alter the key value of a node is a specific requirement of
 * Dijkstra's algorithm (and was the motivation for providing this feature).
 *
 * Operationally, a locator is an integer of type size_t.
 * Locator numbers may be recycled.  When the user deletes a node, that node's
 * locator number could be used again for a future insertion.
 * Thus, do not use a stale locator.  (That advice applies to any pointer!)
 *
 * @subsection rbiter Iterators
 *
 * The tree also offers a bidirectional const_iterator, which can be used to
 * enumerate the locators of the nodes.
 *
 * @section rbimp Implementation notes
 *
 * @todo Consider using XOR instead of addition by LOCATOR_BASE, which would
 *       eliminate any possibility of overflow.   Unfortunately then the
 *       weak invariant "loc is invalid or loc >= LOCATOR_BASE" holds no more.
 *       Would "loc is invalid or loc ^ LOCATOR_BASE <= size" be true? useful?
 *
 * @subsection rbnilleaf "Leaves" are empty
 *
 * This tree is (in a sense) organized so that both keys and satellite data are
 * stored in the internal nodes of the tree, not in the leaves.  All leaves are
 * represented by a single node called "nil."  All nodes also have a parent
 * pointer, which is not useful for the root node and for the nil node (their
 * parent pointers should equal zero).  During a deletion, however, the nil
 * node might have its parent pointer set to the parent of the deleted node.
 *
 * @subsection rblinar Linear array representation
 *
 * In an attempt to simplify the code, I've made the class store its nodes in a
 * linear array for dictionaries of size TREE_THRESH or smaller, currently set
 * at a size of seven nodes.  So this doesn't really build a red-black tree
 * unless there are eight or more internal nodes to be kept.
 *
 * @section rbrefs References
 *
 * The Red-Black tree is due to Bayer, Guibas and Sedgewick.  I'm basing my
 * code on the presentation found in CLR, but I didn't follow their pseudocode.
 *
 * CLR = Cormen, Leiserson and Rivest, Introduction to Algorithms, MIT Press.
 *
 * The locator pattern is described in
 * Goodrich and Tamassia, Data Structures and Algorithms in Java, section 6.4,
 * John Wiley, 1998.
 */
template < typename SATELLITE_TYPE >
class Redblack_subtree_sum
:   public DijkstraPriorityQueue< SATELLITE_TYPE >
{
public:
    typedef SATELLITE_TYPE                                              Sat_tp;
    typedef typename DijkstraPriorityQueue< SATELLITE_TYPE >::Key_tp    Key_tp;
    typedef typename DijkstraPriorityQueue< SATELLITE_TYPE >::Loc_tp    Loc_tp;

private:
    enum
    {
        VERBOSE = 0,            ///< flag for verbosity of internal checks
        TREE_THRESH = 7,        ///< max size for a linear array repr.
        BLACKHEIGHT_BAD = -1,   ///< sentinel val means blackheight undef'd
        LOCATOR_BASE = 987000   ///< constant added to all locator indices
    };

    /// @brief basic node of the tree stores key, subtree sum, satellite data
    struct Node
    {
        Key_tp  key,        ///< search key we use to organize the BST
                sum;        ///< sum of keys in the subtree rooted at this node
        Sat_tp  sat;        ///< satellite data
        Loc_tp  locator;    ///< node locator index (like a coat-check number)
        Node    *left,      ///< pointer to left child
                *right,     ///< pointer to right child
                *parent;    ///< pointer to parent
        bool    is_black;   ///< flag: is this node painted black?

        /// @brief default ctor leaves all fields uninitialized -- danger!
        Node()
        {}

        /// @brief basic ctor initializes all fields
        Node(
            const Key_tp& kk,
            const Sat_tp& ss,
            bool bk,
            Node* ll,
            Node* rr,
            Loc_tp loc
        )
        :   key( kk ),
            sum( kk ),
            sat( ss ),
            locator( loc ),
            left( ll ),
            right( rr ),
            parent( 00 ),
            is_black( bk )
        {}

        /// @brief convenience getter makes the code read a little easier
        bool is_red() const
        {
            return ! is_black;
        }

        /// @brief order operator is based on key value
        bool operator<( const Node& other ) const
        {
            return key < other.key;
        }

        /// @brief update sum field assuming left, right are not eq to null
        void update_sum()
        {
            KJB(ASSERT( left && right ));
            sum = key + left -> sum + right -> sum;
        }

        /// @brief link to a child node, and it back to this
        void you_are_my_child( Node* child, Node* Node::* branch )
        {
            KJB(ASSERT( child ));
            this ->* branch = child;
            child -> parent = this;
        }

        /// @brief link to a child node as this node's left child
        void link_left_child( Node* child )
        {
            you_are_my_child( child, & Node::left );
        }

        /// @brief link to a child node as this node's right child
        void link_right_child( Node* child )
        {
            you_are_my_child( child, & Node::right );
        }
    };

    /// @brief super-simple functor used to sum a node's keys to an accumulator
    struct AddKey
    {
        /// @brief add the key of a node to another key-type input, return sum.
        Key_tp operator()( const Key_tp& a, const Node& n ) { return a+n.key; }
    };


    /**
     * @brief a sentinel node that has two helpful properties.
     *
     * Specifically, it is black and it has a zero sum.
     * We use this single node to represent all "leaves" in the tree, because
     * @ref rbnilleaf in the tree.  So any node bearing a key has its left and
     * right fields point either to other key-bearing nodes or to this node.
     * By using this node in place of rather than NULL pointers, we can
     * simplify the code a little bit.
     *
     * Except for temporarily during a deletion, the parent node of nil is
     * always equal to NULL.
     */
    Node m_nil;

    /// @brief either the root of the tree or the base of the array
    Node *root;

    /// @brief number of valid elements in the dictionary
    unsigned m_size;

    /**
     * @brief lookup from locator index to node pointer
     *
     * For the array implementation:
     * When deletions occur, this list might contain unused entries.
     * Unused entries are always set to zero, and their indices are
     * stacked onto the end of free_locator_list.
     * The advantage of this implementation is quick access from locator
     * to node.
     *
     * For the map implementation:
     * The array-based location list design has a weakness.
     * If you have a huge tree, then delete all but a few late records,
     * you could get into a perverse situation of a small tree with a
     * huge, sparse 'location_list' array.  One improvement on that situation
     * is to use a naturally sparse implementation like std::map.
     * Unfortunately it is noticeably slower.
     */
    class LocationList
    {

#if ! REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
    public:
        typedef std::map< Loc_tp, Node* > Loc_list_impl;
        typedef typename Loc_list_impl::const_iterator LLCI;
#endif

    private:

#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        std::vector< Node* > m_location_list;

        /// @brief table of empty spots in location_list, if any
        std::vector< Loc_tp > free_locator_list;

        /**
         * @brief release any easy-to-release idle location_list entries.
         *
         * If there are NULL cells at the tail end of the location_list, and if
         * their indices are at the tail of the free_locator_list in ascending
         * order, it is very easy to clean them up.  So we do.
         * If there are no such entries, this is a quick no-op.
         */
        void opportunistic_cleanup_of_idle_locators()
        {
            while (  m_location_list.size()
                  && 00 == m_location_list.back()
                  && free_locator_list.size()
                  && 1 + free_locator_list.back() == m_location_list.size()
                  )
            {
                m_location_list.pop_back();
                free_locator_list.pop_back();
            }
        }
#else
        Loc_list_impl m_location_list;
#endif
        LocationList(const LocationList&); // copy ctor teaser
        LocationList& operator=(const LocationList&); // assignment teaser

#ifdef DEBUGGING
        /// @brief a type used for testing validity of locators
        typedef std::map< Loc_tp, char > LocCensus_tp;

        /// @brief record all locators found in the tree
        void locators_scan_nlogn_time(
            LocCensus_tp* census_ptr,
            const Node* ppp,
            const Node* const p_nil
        )   const
        {
            KJB(ASSERT( census_ptr ));
            KJB(ASSERT( ppp ));
            LocCensus_tp& census( *census_ptr );
            if ( p_nil == ppp )
            {
                return;
            }
            census[ ppp -> locator ] += 1;
            locators_scan_nlogn_time( census_ptr, ppp -> left, p_nil );
            locators_scan_nlogn_time( census_ptr, ppp -> right, p_nil );
        }
#endif


    public:

        LocationList() {}

        size_t size() const
        {
            return m_location_list.size();
        }

#if ! REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        LLCI begin() const
        {
            return m_location_list.begin();
        }

        LLCI end() const
        {
            return m_location_list.end();
        }

        LLCI find(Loc_tp l) const
        {
            return m_location_list.find(l);
        }
#endif

        void clear()
        {
            m_location_list.clear();
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            free_locator_list.clear();
#endif
        }

        void replica_of(const LocationList& other)
        {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            m_location_list.resize(other.m_location_list.size());
            free_locator_list.resize(other.free_locator_list.size());
            std::copy(
                other.free_locator_list.begin(), other.free_locator_list.end(),
                free_locator_list.begin()
                );
#endif
        }

        bool empty() const
        {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            return free_locator_list.empty() && m_location_list.empty();
#else
            return m_location_list.empty();
#endif
        }


        /// @brief debug print the locator arrays
        void db_print_locators() const
        {
#ifdef DEBUGGING
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            KJB(TEST_PSE(( "Locs in use: " )));
            for ( size_t iii = 0; iii < m_location_list.size(); ++iii )
            {
                if ( m_location_list.at( iii ) )
                {
                    KJB(TEST_PSE(( "%s%d", (iii ? "," : ""), iii )));
                }
            }
            KJB(TEST_PSE(( "\nLocs avail: " )));
            for ( size_t iii = 0; iii < free_locator_list.size(); ++iii )
            {
                KJB(TEST_PSE(( "%s%d", (iii ? ",":""),
                                       free_locator_list.at( iii ) )));
            }
#else
            KJB(TEST_PSE(( "Locs in use:" )));
            for (LLCI i=m_location_list.begin(); i!=m_location_list.end(); ++i)
            {
                KJB(TEST_PSE((" %u", i -> first)));
            }
#endif
            KJB(TEST_PSE(( "\n" )));
#endif
        }


        /// @brief allocate a locator index
        Loc_tp obtain_avail_locator()
        {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            Loc_tp nuloc = m_location_list.size();

            if ( free_locator_list.size() )
            {
                // we can recycle an old one!  good!
                nuloc = free_locator_list.back();
                free_locator_list.pop_back();
            }
            else
            {
                m_location_list.push_back( 00 );
            }

            return nuloc;
#else
            if (m_location_list.empty()) return 0;

            LLCI i = m_location_list.end();
            --i;
            return 1 + i -> first;
#endif
        }

        /// @brief free a locator index
        void release( Loc_tp loc )
        {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            KJB(ASSERT( loc < m_location_list.size() ));

            // also could test whether loc is in free list (shouldn't be)
            free_locator_list.push_back( loc );
            m_location_list[ loc ] = 00;

            // if location_list has LOTS of null entries, try to collapse them.
            if (    free_locator_list.size() & ~0xFFF     // over 4095 nulls,
                &&  size() < free_locator_list.size()>>3  // over 87% nulls,
                &&  00 == m_location_list.back()          // null last entry.
               )
            {
                std::sort(free_locator_list.begin(), free_locator_list.end());
                opportunistic_cleanup_of_idle_locators();

                /*
                 * To encourage the use of low-value locators, put them at the
                 * end of the free_locator_list, because we draw off the back.
                 */
                std::reverse(
                    free_locator_list.begin(), free_locator_list.end() );
            }
#else
#ifdef DEBUGGING
            const size_t count =
#endif
                m_location_list.erase(loc);
            KJB(ASSERT(1 == count));
#endif
        }

        /// @brief a common idiom we use for verifying one elt of location_list
        bool is_good_here( const Node* ppp ) const
        {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            return ppp && m_location_list.at( ppp -> locator ) == ppp;
#else
            if (00 == ppp) return false;
            LLCI i = m_location_list.find( ppp -> locator );
            return i != m_location_list.end() && i -> second == ppp;
#endif
        }

        Node*& operator[](Loc_tp loc)
        {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            KJB(ASSERT( loc < m_location_list.size() ));
#endif
            return m_location_list[ loc ];
        }

        Node*& at(Loc_tp loc)
        {
            return m_location_list.at( loc );
        }

        const Node* operator[](Loc_tp loc) const
        {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            KJB(ASSERT( loc < m_location_list.size() ));
#endif
            return m_location_list[ loc ];
        }

        const Node* at(Loc_tp loc) const
        {
            return m_location_list.at( loc );
        }


#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        // trivial functor to extract locator field from Node object
        struct GetLoc { Loc_tp operator()(const Node& n) {return n.locator;} };

        /*
         * There is a linear array in root -- small, most likely, but we do
         * not care about the size.  The locators recorded in the array are
         * correct, but we assume the back-end bookkeeping is spoiled, i.e.,
         * location_list and free_locator_list are assumed to be hopelessly
         * wrong, and not worth consulting.  This rebuilds them.
         *
         * An earlier software revision tried to repair them, but because of
         * the opportunistic cleanup that was added, their state became too
         * hard to predict, and I decided it's safer just to trash them and
         * start over.
         */
        void rebuild_for_linear_array(Node* root, size_t size)
        {
            // First, get a sorted list of live locators.
            std::vector< Loc_tp > locs( size );
            std::transform(root, root+size, locs.begin(), GetLoc());
            std::sort(locs.begin(), locs.end());

            // Locators in 'locs' must be pairwise-distinct.
            KJB(ASSERT(
                std::adjacent_find(locs.begin(),locs.end()) == locs.end()));

            /*
             * Rebuild list of pointers, one for each possible locator.
             * Unfortunately, this could be a huge number even for small size.
             * It's an inherent weakness of the array implementation.
             * The (slow) std::map implementation is not vulnerable this way.
             *
             * Obviously this does not fully rebuild the list of pointers, it
             * just creates a list of nulls.  The pointers get filled in later
             * using the Leafify functor (which expects nothing but nulls).
             */
            const size_t llsz = 1 + locs.back();
            m_location_list.assign(llsz, 00);

            // Create list of free locators -- it's just set subtraction:
            // the set {0 to max live locator value} minus locs.
            // Store it into free_locator_list (replacing previous contents).
            std::vector< Loc_tp > panloc(llsz);
            for (size_t i = 0; i < llsz; ++i)
            {
                panloc[i] = i;
            }
            free_locator_list.resize(m_location_list.size() - locs.size());
#ifdef DEBUGGING
            typename std::vector< Loc_tp >::iterator z =
#endif
                std::set_difference(panloc.begin(), panloc.end(),
                                locs.begin(), locs.end(),
                                free_locator_list.begin());
            KJB(ASSERT(free_locator_list.end() == z));
            // Put small free locators at the back so they'll be used sooner.
            std::reverse(free_locator_list.begin(), free_locator_list.end());
        }
#endif

#ifdef DEBUGGING
        /// @brief test the validity of the locator data structs (tree or arr.)
        bool locators_valid_in_nlogn_time(
            bool is_small,
            size_t size,
            const Node* root,
            const Node* const p_nil
        )   const
        {
            typedef typename LocCensus_tp::iterator LCI;

            LocCensus_tp census;

            // scan dictionary and record all locators stored in the records
            if ( is_small )
            {
                for ( size_t iii = 0; iii < size; ++iii )
                {
                    Loc_tp loc = root[ iii ].locator;
                    census[ loc ] += 1;
                    if ( ! is_good_here( root + iii ) )
                    {
                        return false; // fail( __LINE__ );
                    }
                }
            }
            else
            {
                locators_scan_nlogn_time( & census, root, p_nil );
            }

            /*
             * Look at census result.  Each map entry should record a locator
             * that is found in exactly one node, and the node claiming the
             * locator should be at the address recorded in locator_list at
             * that index.
             */
            for ( LCI iii = census.begin(); iii != census.end(); ++iii )
            {
                Loc_tp loc = iii -> first;
                if ( iii -> second != 1 )
                {
                    return false; //fail( __LINE__ );
                }
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
                Node* ppp = m_location_list[ loc ];
                if ( 00 == ppp || ppp -> locator != loc )
                {
                    return false; //fail( __LINE__ );
                }
#else
                LLCI i = m_location_list.find(loc);
                if (  m_location_list.end() == i
                   || 00 == i -> second
                   || i -> first != loc
                   )
                {
                    return false; //fail( __LINE__ );
                }
#endif
            }

#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            // Scan the list of unused locators.
            LocCensus_tp unused;
            for ( size_t iii = 0; iii < free_locator_list.size(); ++iii )
            {
                unused[ free_locator_list[ iii ] ] += 1;
            }

            /*
             * Scan the results of the unused list.  Each entry (if any) should
             * record a locator that is found just once, and it should not be
             * found in the tree; likewise that index should mark a NULL entry
             * in the table.
             */
            for ( LCI iii = unused.begin(); iii != unused.end(); ++iii )
            {
                Loc_tp loc = iii -> first;
                // test for free_list duplicates
                if ( iii -> second != 1 ) return false; //fail( __LINE__ );
                // loc must not be found in tree
                if ( census[ loc ] != 0 )
                {
                    KJB(TEST_PSE(( "census loc is %d\n", int(census[loc]) )));
                    return false; //fail( __LINE__ );
                }
                // loc must be valid
                if ( m_location_list.size() <= loc ) return false;
                                                     //fail( __LINE__ );
                // loc must be unused
                if ( m_location_list[ loc ] != 00 ) return false;
                                                    //fail( __LINE__ );
            }
#endif

            /*
             * Scan location_list.  Every entry must correspond to an entry in
             * census (for pointers not equal to NULL) or to unused (for
             * pointers equal to NULL).
             */
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            for ( Loc_tp loc = 0; loc < m_location_list.size(); ++loc )
            {
                if ( m_location_list[ loc ] )
                {
                    if ( census[ loc ] != 1 )
                    {
                        std::ostringstream oss;
                        oss << "locator index " << loc
                            << " has\n\tcensus value " << int(census[loc])
                            << "\n\tptr value " << m_location_list[loc]
                            << "\n\tptr loc " << m_location_list[loc]->locator
                            << '\n';
                        KJB(TEST_PSE(( "%s\n", oss.str().c_str() )));
                        return false; //fail( __LINE__ );
                    }
                }
                else if ( unused[ loc ] != 1 )
                {
                    return false; //fail( __LINE__ );
                }
            }
#else
            for (LLCI i=m_location_list.begin(); i!=m_location_list.end(); ++i)
            {
                const Loc_tp loc = i -> first;
                if (00 == i -> second) return false; //fail( __LINE__ );

                if ( census[ loc ] != 1 )
                {
                    std::ostringstream oss;
                    oss << "locator index " << loc
                        << " has\n\tcensus value " << int(census[loc])
                        << "\n\tptr value " << i -> second
                        << "\n\tptr loc " << i -> second -> locator
                        << '\n';
                    KJB(TEST_PSE(( "%s\n", oss.str().c_str() )));
                    return false; // fail( __LINE__ );
                }
            }
#endif
            return true;
        }
#endif
    };

    LocationList location_list;


#ifdef DEBUGGING
    /// @brief emit a debug message
    void chatter( const char* s ) const
    {
        if ( VERBOSE ) KJB(TEST_PSE(( "CHATTER: %s\n", s )));
    }

    /// @brief emit message intended for entering a function (use __func__)
    void enter( const char* s ) const
    {
        if ( VERBOSE ) KJB(TEST_PSE(( "CHATTER: Entering %s\n", s )));
    }

    /// @brief stub for debug function printing a message and returning false.
    bool fail( const char *s ) const
    {
        chatter( s );
        return false;
    }

    /// @brief stub for debug function printing a number and returning false.
    bool fail( int nnn ) const
    {
        KJB(TEST_PSE(( "FAIL: line %d\n", nnn )));
        return false;
    }

#else
    /// @brief stub used for debug
    void chatter( const char* ) const {}

    /// @brief stub used for debug
    void enter( const char* ) const {}

    /// @brief stub for function used during debugging
    bool fail( const char* ) const { return false; }

    /// @brief stub for function used during debugging
    bool fail( int ) const { return false; }
#endif







    /// @brief initialize a new node set up to be a parent of nil leaves.
    Node* new_node( const Key_tp& key, const Sat_tp& sat, bool black )
    {
        const Loc_tp nuloc = location_list.obtain_avail_locator();
        Node* nn = new Node( key, sat, black, & m_nil, & m_nil, nuloc );
        KJB(ASSERT( nuloc == nn -> locator ));
        location_list[ nuloc ] = nn;
        return nn;
    }


    /// @brief initialize a new node by copying an old node
    Node* new_node( const Node& old_node )
    {
        return new_node( old_node.key, old_node.sat, old_node.is_black );
    }



    /// @brief delete a node and recover its locator
    void dispose( Node* ppp )
    {
        KJB(ASSERT( ppp && location_list.is_good_here( ppp ) ));
        const Loc_tp ploc = ppp -> locator;
        location_list.release( ploc );
        delete ppp;
    }


    /// @brief test whether a node equals this tree's ubiquitous nil node
    bool is_nil( const Node* ppp ) const
    {
        return ppp == & m_nil;
    }


    /// @brief test whether a node does not equal this tree's nil node
    bool is_not_nil( const Node* ppp ) const
    {
        return ppp != & m_nil;
    }


    /// @brief test whether this node is the parent of two nil leaves
    bool are_both_children_nil( const Node* ppp ) const
    {
        return      ppp
                &&  is_not_nil( ppp )
                &&  is_nil( ppp -> left )
                &&  is_nil( ppp -> right );
    }


    /**
     * @brief verify the @ref redblacki3 while computing @ref blackheight
     * @param ppp   pointer to root node of subtree to be verified.
     * @return BLACKHEIGHT_BAD if the @ref redblacki3 is violated.
     * @return the @ref blackheight otherwise.
     */
    int blackheight_in_linear_time( const Node* ppp ) const
    {
        KJB(ASSERT( ppp ));
        if ( is_nil( ppp ) )
        {
            return 1;   // because nil is black
        }

        int bh = blackheight_in_linear_time( ppp -> left );

        if  (       bh != BLACKHEIGHT_BAD
                &&  bh == blackheight_in_linear_time( ppp -> right )
            )
        {
            return bh + ( ppp -> is_black ? 1 : 0 );
        }

        return BLACKHEIGHT_BAD;
    }


    /**
     * @brief release all memory in the subtree rooted at the indicated node.
     *
     * Note how we also test for ppp not equal to 0.  I believe that could
     * happen if std::bad_alloc is thrown.  (Not tested.)
     */
    void recursive_destroy( Node* ppp )
    {
        if ( ppp && is_not_nil( ppp ) )
        {
            recursive_destroy( ppp -> left );
            recursive_destroy( ppp -> right );
            dispose( ppp );
        }
    }


    /// @brief convert the linear array into a binary search tree structure
    void make_it_a_real_tree_now()
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        std::sort( root, root + TREE_THRESH );
        Node sorty[ TREE_THRESH ];
        std::copy( root, root + TREE_THRESH, sorty );
        delete[] root;

        // build tree (unfortunately, this inserts fresh, unwanted locators)
        KJB(ASSERT( 7 == TREE_THRESH ));
        root =                              new_node( sorty[ 3 ] );
        root -> link_left_child(            new_node( sorty[ 1 ] ) );
        root -> link_right_child(           new_node( sorty[ 5 ] ) );
        root -> left -> link_left_child(    new_node( sorty[ 0 ] ) );
        root -> left -> link_right_child(   new_node( sorty[ 2 ] ) );
        root -> right -> link_left_child(   new_node( sorty[ 4 ] ) );
        root -> right -> link_right_child(  new_node( sorty[ 6 ] ) );
        root -> parent = m_nil.parent = 00;

        // unfortunately that little tree is now full of unwanted locators
        location_list.release( root                   -> locator );
        location_list.release( root -> left           -> locator );
        location_list.release( root -> right          -> locator );
        location_list.release( root -> left -> left   -> locator );
        location_list.release( root -> left -> right  -> locator );
        location_list.release( root -> right -> left  -> locator );
        location_list.release( root -> right -> right -> locator );

        // fix the location_list
        location_list[ sorty[ 3 ].locator ] = root;
        location_list[ sorty[ 1 ].locator ] = root -> left;
        location_list[ sorty[ 5 ].locator ] = root -> right;
        location_list[ sorty[ 0 ].locator ] = root -> left -> left;
        location_list[ sorty[ 2 ].locator ] = root -> left -> right;
        location_list[ sorty[ 4 ].locator ] = root -> right -> left;
        location_list[ sorty[ 6 ].locator ] = root -> right -> right;

        // fix the tree's embedded locator indices
        root                    -> locator = sorty[ 3 ].locator;
        root -> left            -> locator = sorty[ 1 ].locator;
        root -> right           -> locator = sorty[ 5 ].locator;
        root -> left -> left    -> locator = sorty[ 0 ].locator;
        root -> left -> right   -> locator = sorty[ 2 ].locator;
        root -> right -> left   -> locator = sorty[ 4 ].locator;
        root -> right -> right  -> locator = sorty[ 6 ].locator;

        // update the sums
        KJB(ASSERT( 7 == TREE_THRESH ));
        root -> left -> update_sum();
        root -> right -> update_sum();
        root -> update_sum();
    }




    /**
     * @brief insert new node into a subtree as an obligate child.
     *
     * @pre toparent does not equal NULL.
     * @pre *toparent does not equal NULL.
     * @pre n00b does not equal NULL.
     *
     * @param[in,out] toparent  pointer to whatever tree pointer links to the
     *                          potential parent of the new node we want to
     *                          insert.
     * @param[in] pn_left       flag true iff the path from parent to child
     *                          node goes through a left branch.
     * @param[in] n00b          new node we want to insert in the tree.
     *
     * Insert new node *n00b (not currently in the tree) somewhere into the
     * subtree rooted at the node that *toparent points to.
     *
     * Note this function is in mutual induction with insert_gp().
     * As noted there, we would need a no-throw guarantee here for exception
     * safety, but I am not yet absolutely certain we can truly guarantee it.
     *
     * @return value in set {0,1,2} to indicate potential problems in tree:
     * - Value 0 means no problems; this subtree satisfies the @ref redblacki2.
     * - Value 1 means the child of *parent potentially breaks @ref redblacki2.
     * - Value 2 means *parent potentially breaks @ref redblacki2.
     */
    int insert_parent( Node** toparent, bool pn_left, Node* n00b )
    {
        KJB(ASSERT( n00b && toparent && *toparent ));
        Node* parent = *toparent;

        Node* Node::* branch = pn_left ? & Node::left : & Node::right;
        if ( is_nil( parent ->* branch ) )
        {
            KJB(ASSERT( n00b -> is_red() ));
            parent -> you_are_my_child( n00b, branch );
            parent -> sum += n00b -> sum;
            return 1; // red child at depth 2 in subtree rooted at gramp
        }
        else
        {
#ifdef DEBUGGING
            int rc = insert_gp( toparent, pn_left, n00b );
            /*
             * If rc is zero, that is fine:  it means no-red-red is satisfied.
             * If rc is 2 or 4, we can handle it:  it means no-red-red might
             * be unsatisfied, but
             */
            KJB(ASSERT( 0 == rc || 2 == rc || 4 == rc ));
            KJB(ASSERT( rc != 2 || ( parent ->* branch ) -> is_red() ));
            KJB(ASSERT( rc != 4 || parent -> is_red() ));

            /*
             * Shift that return value downwards, because it describes the tree
             * status for a deeper context, but we want to return the tree
             * status for the current context.
             */
            return rc >> 1;
#else
            return insert_gp( toparent, pn_left, n00b ) >> 1;
#endif
        }
    }


    /**
     * @brief insert a new node into a subtree as an obligate grandchild.
     *
     * "Obligate grandchild" means the new node will have a parent and a
     * grandparent:  prior to calling rb_balance(), it won't be the root or a
     * child of the root.
     *
     * @param togramp   points to a tree pointer that points to the node that
     *                  will be either the grandparent of n00b or a more
     *                  distant ancestor.
     * @param gp_left   True iff n00b goes into the left subtree of *togramp.
     * @param n00b      points to a new node not already in the tree.
     *
     * @pre togramp must not equal NULL.
     * @pre *togramp must not equal NULL.
     * @pre n00b must not equal NULL.
     *
     * @return  A value indicating whether (and if so, where) a red node in the
     *          tree is potentially violating the @ref redblacki2.
     *          If the value is zero, then the subtree rooted at *gramp
     *          satisfies the invariant.  A nonzero value indicates a potential
     *          violation.  We use powers of two to indicate relative height in
     *          the tree of the red node that is potentially violating.  Thus
     *          values 4, 2, 1 would indicate *gramp, a child of *gramp, and a
     *          grandchild of *gramp (following the path to n00b).
     *          The value of 1 is never returned because it is resolved by a
     *          call to rb_balance().
     *          Larger values (2 or 4) indicate a topology that could require
     *          resolution higher in the tree; so we return and and rely on the
     *          caller to fix the problem.
     *
     * I know it looks stupid to compute gp_left as an argument, but because of
     * mutual recursion we would end up doing twice the number of key
     * comparisons if we did it inside the function.
     *
     * For exception safety, this needs a no-throw guarantee, and I think the
     * code achieves it, but I am not absolutely certain.
     */
    int insert_gp( Node** togramp, bool gp_left, Node* n00b )
    {
        KJB(ASSERT( n00b && togramp && *togramp ));

        Node    *gramp = *togramp,
                **toparent = gp_left ? & gramp -> left : & gramp -> right;

        gramp -> sum += n00b -> sum;

        KJB(ASSERT( toparent ));
        Node *parent = *toparent;

        KJB(ASSERT( parent ));
        bool pn_left = *n00b < *parent;

        /*
         * The semantics of the value in rc and of the value returned by this
         * function are the same.  This is important, pay attention!
         *
         * rc tells us about a red node that might be the child of a red node,
         * i.e., a potential violation of the @ref redblacki2.
         * The possible values are in the set {0, 1, 2}.
         *
         * If 0==rc, then the invariant is satisfied in this subtree.
         * If 1==rc, then new node n00b is red; must check *parent.
         * If 2==rc, then *parent is newly painted red; must check *gramp.
         *
         * In like wise, the value 4 would indicate *gramp is newly painted
         * red (see below).  Thus the value in rc and the return value below
         * have consistent semantics.
         */
        int rc = insert_parent( toparent, pn_left, n00b );
        KJB(ASSERT( 0 == rc || 1 == rc || 2 == rc ));

        /*
         * The return value below is in the set {0, 2, 4}.
         * Values 0 and 2 have the same semantics as 'rc' above.
         * Value 4 means *gramp is newly painted red and the caller must
         * check its parent.
         */
        return rb_balance( togramp, gp_left, pn_left, rc );
    }


    /// @brief convenience fun. returns a Node field pointer the "other" way.
    Node* Node::* opposite_direction( Node* Node::* some_direction ) const
    {
        return some_direction == & Node::left ? & Node::right : & Node::left;
    }


    /**
     * @brief straighten a zigzagged branch of the tree by swapping two nodes
     *
     * @param gramp grandparent node, G in diagram below.
     * @param p_branch  Node branch that indicates the direction from
     *                  grandparent to the parent.
     *
     * Diagram assumes *p_branch is Node::left; diagram would be mirror image
     * otherwise.
     *
     * Key:
     * - G = gramp
     * - B = oldbarent I mean parent
     * - H = oldcHild
     * - U = uncle of H (could be nil)
     * - a = min_tree (could be nil)
     * - D = meD_tree (could be nil)
     * - k = max_tree (could be nil)
     * - + = updated subset-sum
     *
     * Before, with ancestors above descendants, keys ordered left to right:
     * @code
     *         G
     *    B         U
     *  a   H
     *     D k
     * @endcode
     *
     * After:
     * @code
     *        G
     *    H+      U
     *  B+  k
     * a D
     * @endcode
     */
    void unzigzag( Node* gramp, Node* Node::* p_branch )
    {
        Node* Node::* u_branch = opposite_direction( p_branch );//towards uncle

        KJB(ASSERT( gramp ));
        KJB(ASSERT( is_not_nil( gramp ->* p_branch ) ));
        KJB(ASSERT( is_not_nil( gramp ->* p_branch ->* u_branch ) ));

        Node    *oldparent = gramp ->* p_branch,
                *oldchild = oldparent ->* u_branch,
                *med_tree = oldchild ->* p_branch; // could be nil

        gramp -> you_are_my_child( oldchild, p_branch );
        oldchild -> you_are_my_child( oldparent, p_branch );

        oldparent ->* u_branch = med_tree;
        if ( is_not_nil( med_tree ) )
        {
            med_tree -> parent = oldparent;
        }

        /*
         * Now "oldparent" is the child of "oldchild" -- it's freaky friday.
         */
        oldparent -> update_sum();
        oldchild -> update_sum();
    }


    /**
     * @brief rotate tree with black gramp and its red offspring
     *
     * @param togramp   points to whatever tree pointer points to grandparent,
     *                  G in diagram below.  Diagram assumes p_branch indicates
     *                  the left pointer field; diagram would be its mirror
     *                  image if p_branch indicates the right pointer field.
     * @param p_branch  Node branch that indicates the direction from
     *                  grandparent to the parent (C in diagram below).
     *
     * @pre Parent node (C in diagram below) is not nil.
     *
     * @post If parent node C has a non-nil child (D in diagram) in a zig-zag
     *       configuration (i.e., regarding nodes C and D, one is a left child
     *       and one is a right child), THEN, afterwards, D becomes a child of
     *       G and switches direction (if it was a right child before the call,
     *       it is a left child afterwards, or vice versa).
     *
     * Key:
     * - G = oldgramp
     * - C = oldparent
     * - D = medmed_tree child (could be nil)
     * - + = updated subset-sum
     *
     * Configuration before rotation is shown below.  Left-to-right spacing
     * indicates key order, and ancestors are above descendants.
     * @code
     *         G
     *    C         U
     *       D
     * @endcode
     *
     * After rotation:
     * @code
     *         C+
     *              G+
     *           D     U
     * @endcode
     */
    void rotate( Node** togramp, Node* Node::* p_branch )
    {
        KJB(ASSERT( togramp && *togramp ));

        Node* Node::* u_branch = opposite_direction( p_branch ); // to uncle
        Node* oldgramp = *togramp;

        KJB(ASSERT( is_not_nil( oldgramp ) ));
        KJB(ASSERT( is_not_nil( oldgramp ->* p_branch ) ));

        Node    *oldparent = oldgramp ->* p_branch,
                *medmed_tree = oldparent ->* u_branch;

        KJB(ASSERT( oldparent -> parent == oldgramp ));

        // rotate the descendant relationships
        *togramp = oldparent;

        // update the ancestor relationships
        oldparent -> parent = oldgramp -> parent;
        oldparent -> you_are_my_child( oldgramp, u_branch );

        oldgramp ->* p_branch = medmed_tree;
        if ( is_not_nil( medmed_tree ) )
        {
            medmed_tree -> parent = oldgramp;
        }

        /*
         * Now "oldparent" and "oldgramp" have swapped parent-child statuses.
         * Also they have to swap colors.
         * The former sibling of "oldparent" is still the child of "oldgramp"
         * and so it is now become a grandchild of "oldparent."  IF ANY.
         */
        std::swap( oldgramp -> is_black, oldparent -> is_black );

        oldgramp -> update_sum();
        oldparent -> update_sum();
    }


    /**
     * @brief maybe restore the @ref redblacki2
     *
     * @param[in,out] togramp   Points to whatever tree pointer points to the
     *                          grandparent node of the newest child.
     * @param[in] gp_left       Flag set true iff the path from grandparent to
     *                          parent goes through the left branch.
     * @param[in] pn_left       Flag set true iff the path from parent to new
     *                          child goes through the left branch.
     * @param[in] red_child     A bitmap that indicates how the color invariant
     *                          needs to be repaired in the tree (see below).
     *
     * @return a bitmap indicating where, if anywhere, the tree's color
     * invariants need to be fixed.  If red_child is 1, that means that the
     * grandchild of **togramp indicated
     * by gp_left and pn_left is potentially violating @ref redblacki2.
     * If so, this function will resolve that node's violation.  In that case
     * it will return the value 0 or 4.
     * - The return value is 0 under one of two conditions:
     *  - we determine that there is no actual violation (parent is black).
     *  - violation is actual but we resolve it via one or two rotations.
     * - The return value is 4 (= 1<<2) when the violation is actual but we
     *      resolve it in part by turning the grandparent red (which itself
     *      could cause a violation that must be resolved higher in the tree).
     *
     * If red_child is not equal to 1, we immediately return the value in
     * red_child without altering the tree.
     */
    int rb_balance( Node** togramp, bool gp_left, bool pn_left, int red_child )
    {
        /*
         * On input, red_child is...
         * 3 if n00b is newly red and that fact must be handled.
         * 2 if parent is newly red and that fact must be handled.
         * 1 if gramp is newly red and that fact must be handled.
         * 0 if no one's redness must be handled.
         */
        if ( red_child != 1 )
        {
            return red_child;
        }

        KJB(ASSERT( togramp && *togramp ));

        Node* Node::* gp_branch = gp_left ? & Node::left : & Node::right;

        Node    *gramp = *togramp,
                *parent = gramp ->* gp_branch,
                *uncle = gramp ->* opposite_direction( gp_branch );

        KJB(ASSERT( parent ));
        if ( parent -> is_black )
        {
            return 0;   // if parent is black then a red child is not a problem
        }
        KJB(ASSERT( gramp -> is_black )); //parent and gramp cannot both be red

        if ( uncle -> is_black ) // also, uncle could be "nil" (black).
        {
            // unzigzag, if necessary
            if ( gp_left != pn_left )
            {
                unzigzag( gramp, gp_branch );
            }
            // rotate like alexander the great
            rotate( togramp, gp_branch );
            KJB(ASSERT( 00 == m_nil.parent ));
            return 0;
        }

        KJB(ASSERT( is_not_nil( uncle ) ));
        uncle -> is_black = parent -> is_black = true;
        gramp -> is_black = false;
        return 1 << 2;  // now gramp is a red child 2 levels higher
    }


#ifdef DEBUGGING
    /// @brief print the indentation of a debug print
    void dbp_indent( int depth, std::ostream& os ) const
    {
        static const char* INDENT = "\t";
        for ( int iii = 0; iii < depth; ++iii )
        {
            os << INDENT;
        }
    }


    /// @brief during a debug print, this prints a subtree branch
    void db_print_child(
        const Node* ppp,
        int depth,
        Node* Node::* branchpp,
        const char* branchss,
        std::ostream& os
    )   const
    {
        dbp_indent( depth, os );
        os << branchss << " subtree:";
        if ( is_nil( ppp ->* branchpp ) )
        {
            os << " NIL\n";
        }
        else
        {
            if ( ppp != root && ( ppp ->* branchpp ) -> parent != ppp )
            {
                os << " BROKEN!!!! ";
            }
            os << '\n';
            recursive_db_print( ppp ->* branchpp, 1 + depth, os );
        }
    }


    /// @brief during a debug print, this prints a node and its subtree
    void recursive_db_print(
        const Node* ppp,
        int depth,
        std::ostream& os
    ) const
    {
        if ( is_nil( ppp ) )
        {
            return;
        }

        dbp_indent( depth, os );
        os << "key=" << ppp -> key << ", sum=" << ppp -> sum
            << ", color=" << ( ppp -> is_black ? "BLACK" : "red" )
            << ", sat=" << ppp -> sat
            << ", loc=" << ppp -> locator;

        if ( ! dictionary_is_small_linear_array() )
        {
            os << ", blackheight=" << blackheight_in_linear_time( ppp );
        }
        else if ( ! are_both_children_nil( ppp ) )
        {
            os << ", WARNING: children are not nil";
        }

        if ( are_both_children_nil( ppp ) )
        {
            os << '\n';
            return;
        }
        os << ", left=" << ppp -> left << ", right=" << ppp -> right << '\n';

        db_print_child( ppp, depth, & Node::left, "Left", os );
        db_print_child( ppp, depth, & Node::right, "Right", os );
    }
#endif


    /// @brief scan entire tree to see if any red node has a red child
    bool red_node_has_red_child_in_linear_time( const Node* ppp ) const
    {
        KJB(ASSERT( ppp ));
        if ( is_nil( ppp ) || are_both_children_nil( ppp ) )
        {
            return false;
        }
        if ( ppp -> is_red() && ppp -> left -> is_red() )
        {
            return true;
        }
        if ( ppp -> is_red() && ppp -> right -> is_red() )
        {
            return true;
        }
        return      red_node_has_red_child_in_linear_time( ppp -> left )
                ||  red_node_has_red_child_in_linear_time( ppp -> right );
    }


    /// @brief scan entire tree to verify that the sums are exactly correct
    bool sums_are_correct_in_linear_time( const Node* ppp ) const
    {
        KJB(ASSERT( ppp ));
        if ( is_nil( ppp ) ) return true;
        Key_tp sts = ppp -> key;
        if ( is_not_nil( ppp -> left ) ) sts += ppp -> left -> sum;
        if ( is_not_nil( ppp -> right ) ) sts += ppp -> right -> sum;
#ifdef DEBUGGING
        if ( sts != ppp -> sum )
        {
            chatter( "internal sums are incorrect" );
        }
#endif
        return      sts == ppp -> sum
                &&  sums_are_correct_in_linear_time( ppp -> left )
                &&  sums_are_correct_in_linear_time( ppp -> right );
    }


    /// @brief scan tree and see if sums are fairly close to correct
    bool sums_are_close_enough_in_linear_time( const Node* ppp ) const
    {
        const Key_tp SMALL = 1e-3;
        KJB(ASSERT( ppp ));
        if ( is_nil( ppp ) )
            return true;

        Key_tp sts = ppp -> key;
        if ( is_not_nil( ppp -> left ) ) sts += ppp -> left -> sum;
        if ( is_not_nil( ppp -> right ) ) sts += ppp -> right -> sum;

        Key_tp rrr = fabs( sts - ppp -> sum )/(fabs( sts )+fabs( ppp -> sum ));

#ifdef DEBUGGING
        if ( !( sts == ppp -> sum || rrr < SMALL ) )
        {
            chatter( "internal sums are inadequately accurate" );
            std::ostringstream ess;
            ess << "rrr=" <<rrr <<" which exceeds SMALL=" << SMALL << '\n';
            chatter( ess.str().c_str() );
            return fail( __LINE__ );
        }
#endif

        return      ( sts == ppp -> sum || rrr < SMALL )
                &&  sums_are_close_enough_in_linear_time( ppp -> left )
                &&  sums_are_close_enough_in_linear_time( ppp -> right );
    }


    /**
     * @brief search through the linear array (used for tiny dictionaries).
     *
     * @param[in] qkey      Query key for which to search
     * @param[out] sat_out  Optional ptr to object where satellite data can
     *                      be copied (clobberly) in case the key is found.
     * @param[out] loc_out  Optional ptr to obj where the locator for this
     *                      record can be copied, in case the key is found.
     * @param[out] ix       Optional ptr to obj where the linear array index
     *                      of this record can be copied, if key is found.
     *
     * @return true iff the query key is found somewhere in the dictionary.
     *
     * Remember this dictionary does not require keys to be unique, and a
     * search for a key used more than once could return any of the records
     * using that key: the order is undefined.
     * TODO:  use std::find
     */
    bool linear_search(
        const Key_tp& qkey,
        Sat_tp* sat_out,
        Loc_tp* loc_out,
        size_t* ix
    )
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        for ( size_t iii = 0; iii < m_size; ++iii )
        {
            if ( root[ iii ].key == qkey )
            {
                if ( sat_out ) *sat_out = root[ iii ].sat;
                if ( loc_out ) *loc_out = root[ iii ].locator;
                if ( ix ) *ix = iii;
                KJB(ASSERT( location_list.is_good_here( root + iii ) ));
                return true;
            }
        }
        return false;
    }


    /// @brief search through the tree structure for a key
    Node* tree_search(
        const Key_tp& qkey,
        Sat_tp* sat_out,
        Node* ppp,
        Loc_tp* loc_out
    )
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        KJB(ASSERT( ppp ));
        if ( is_nil( ppp ) )
        {
            return 00;
        }
        if ( qkey == ppp -> key )
        {
            if ( sat_out ) *sat_out = ppp -> sat;
            if ( loc_out ) *loc_out = ppp -> locator;
            return ppp;
        }
        Node* Node::* branch= qkey < ppp -> key ? & Node::left : & Node::right;
        return tree_search( qkey, sat_out, ppp ->* branch, loc_out );
    }


    /**
     * @brief shrink a subtree structure into a linear array, inorder
     * @param[in] tree      Root node of subtree to scan (inorder) into dest
     * @param[out] dest     Array into which to store nodes, inorder
     *
     * @return Next available array location into which to write
     *
     * Nil leaves do not get written into dest, naturally.
     */
    Node* recursive_condense( Node* tree, Node* dest )
    {
        KJB(ASSERT( tree && dest ));
        if ( is_nil( tree ) )
        {
            return dest;
        }

        // first, recurse on left subtree
        Node* d2 = recursive_condense( tree -> left, dest );
        // second, store the current node
        *d2++ = *tree;
        // third and last, recurse on right subtree
        Node* d3 = recursive_condense( tree -> right, d2 );
        return d3;
    }




    /*
     * This is a functor to alter a node into a black, parentless leaf (i.e.,
     * ready to be stored in a small linear array).  Also, this rebuilds the
     * locator - pointer links.  We assume the location list holds nulls.
     * Only the key and sat fields are left undisturbed.
     */
    class Leafify
    {
        Redblack_subtree_sum< SATELLITE_TYPE >* const tree;

    public:
        Leafify(Redblack_subtree_sum< SATELLITE_TYPE >* t) : tree(t) {}

        void operator()(Node& node) const
        {
            node.left = node.right = & tree -> m_nil; // children now nil
            node.parent = 00;                         // parent now null
            node.is_black = true;                     // blacken
            node.sum = node.key;                      // sum is key

            // update locator
            KJB(ASSERT( 00 == tree -> location_list[ node.locator ] ));
            tree -> location_list[ node.locator ] = &node;
        }
    };





    /// @brief shrink tree into a linear array and manage memory as required
    void condense_into_linear_array()
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        KJB(ASSERT( size() ));

        /*
         * Copy tree entries into linear array.  Dispose of tree.
         * (Unfortunately, the second step also disposes of the locators.)
         */
        Node *la = new Node[ size() ];
#ifdef DEBUGGING
        const Node *full =
#endif
            recursive_condense( root, la );
        KJB(ASSERT( full - la == int( size() ) ));
        recursive_destroy( root );
        root = la;

#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        // The previous locators were unfairly disposed of, so restore them.
        location_list.rebuild_for_linear_array(root, size());
#else
        KJB(ASSERT(location_list.empty()));
#endif

        // Finally, fix up the node fields in the array, and re-link locators.
        std::for_each(root, root + size(), Leafify(this));
    }


    /// @brief get pointer to the pointer linking to the indicated non-nil node
    Node** to_parent_link( Node* ppp )
    {
        if ( ppp == root )
        {
            return & root;
        }

        KJB(ASSERT( ppp && is_not_nil( ppp ) ));

        Node* parent = ppp -> parent;
        KJB(ASSERT( parent && is_not_nil( parent ) ));
        KJB(ASSERT( parent -> right == ppp || parent -> left == ppp ));

        return parent -> left == ppp ? & parent -> left : & parent -> right;
    }


    /// @brief get a Node field indicating how parent points to a non-nil node
    Node* Node::* parent_branch_to_me( Node* ppp ) const
    {
        KJB(ASSERT( ppp && ppp != root /* && is_not_nil( ppp ) */ ));
        KJB(ASSERT( ppp -> parent ));
        KJB(ASSERT(     ppp -> parent -> right == ppp
                    ||  ppp -> parent -> left == ppp  ));
        return ppp -> parent -> right == ppp ? & Node::right : & Node::left;
    }


    /**
     * @brief remove the indicated node from between its parent and child.
     *
     * Roughly speaking, *ppp is like the middle node of a 3-node linked list.
     * We splice the linked list so the parent of *ppp and the child of *ppp
     * are connected, and they lose their attachments to middle node *ppp.
     * The child of *ppp could be the nil node.
     *
     * @param ppp       pointer to the node to be detached from the tree
     * @param par_br    Node field pointer indicating which way the parent node
     *                  points to *ppp.
     * @param child_br  Node field pointer indicating which way the child node
     *                  descends from node *ppp.
     *
     * @pre the number of non-nil children of *ppp must be one or none.
     * @pre node *ppp must have a parent node (it is not the root).
     * @return ppp -- the node itself is not touched
     * @post parent node of ppp points to the indicated child of ppp via its
     * *par_br branch.
     * @post the child node has its parent pointer set to the parent of *ppp,
     * regardless of whether the child is nil!  That breaks a common invariant!
     */
    Node* splice_me( Node* ppp, Node* Node::* par_br, Node* Node::* child_br )
    {
        KJB(ASSERT( ppp && ppp != root ));

        Node    *child_p = ppp ->* child_br,
                *par_p = ppp -> parent;
        KJB(ASSERT( par_p && is_not_nil( par_p ) ));
        KJB(ASSERT( ppp == par_p ->* par_br ));
        KJB(ASSERT( is_nil( ppp ->* opposite_direction( child_br ) ) ));
        KJB(ASSERT( child_p ));
        KJB(ASSERT( is_nil( child_p ) || ppp == child_p -> parent ));
        KJB(ASSERT( 00 == m_nil.parent ));

        /*
         * NOTE WELL:  we do the following EVEN WHEN CHILD IS THE NIL NODE!!
         * That's because it is the easiest way to remember the bereaved
         * parent node, an entity we often must manipulate during a deletion.
         * The alternative was something like the following:
         *
                par_p ->* par_br = child_p;
                if ( is_not_nil( child_p ) )
                    child_p -> parent = par_p;
        */
        par_p -> you_are_my_child( child_p, par_br );

        KJB(ASSERT( (ppp ->* child_br) -> parent == ppp -> parent ));
        KJB(ASSERT( ppp -> parent ->* par_br == ppp ->* child_br ));

        return ppp;
    }


    /// @brief like splice_me() but also works for root, and figures out par_br
    Node* semiauto_splice( Node* ppp, Node* Node::* child_br )
    {
        if ( ppp == root )
        {
            KJB(ASSERT( is_nil( ppp ->* opposite_direction( child_br ) ) ));
            KJB(ASSERT(     is_nil( ppp ->* child_br )
                        ||  ppp == ( ppp ->* child_br ) -> parent ));
            root = ppp ->* child_br;
            return ppp;
        }
        return splice_me( ppp, parent_branch_to_me( ppp ), child_br );
    }


    /**
     * @brief like semiauto_splice() but figures out child_br
     * @param[in] ppp       pointer to node to be spliced out, if possible.
     * @param[out] tochild  indicates a pointer into which we write a pointer
     *                      to the "bereaved child" formerly a child of ppp; if
     *                      ppp has one non-nil child, that one is the output.
     *                      If both children are nil, this returns the address
     *                      of the nil node, whose parent pointer is set to
     *                      the "bereaved parent," formerly parent of *ppp.
     * @return pointer to indicated node, or ZERO if the node has two children.
     */
    Node* fully_auto_splice( Node* ppp, Node ** tochild )
    {
        if ( is_nil( ppp -> left ) )
        {
            *tochild = ppp -> right;
            return semiauto_splice( ppp, & Node::right );
        }
        if ( is_nil( ppp -> right ) )
        {
            *tochild = ppp -> left;
            return semiauto_splice( ppp, & Node::left );
        }
        return 00;  // cannot splice ppp because it has two children
    }


    /**
     * "Target" is too important to be eliminated (it has two children).
     * Instead we find a less important victim to take the fall.
     * Also, the trace must be augmented to show the path down to the victim.
     * Target will take over the victim's identity; also the subtree-sums must
     * be decreased by victim-key from Target's right-child down to the victim.
     *
     * @param[in] target    Node whose contents, at least, we wish to eliminate
     * @param[out] to_child Indicates a pointer to the "bereaved child" node,
     * i.e., the return value could have been a node with at most one non-nil
     * child, which of course "loses" its former parent.  We write a pointer
     * to that child into this location.  If the returned value pointed to a
     * node with two nil-leaf children, we nevertheless return a pointer to the
     * nil node, and the nil node has its parent pointer set to the "bereaved
     * parent."
     *
     * @return pointer to victim, i.e., the node whose contents replace that of
     * *target, and that gets disconnected from the tree structure -- a node
     * you can delete.
     */
    Node* del_child_not_target( Node* target, Node** to_child )
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        KJB(ASSERT( target && is_not_nil( target ) ));
        KJB(ASSERT( is_not_nil( target -> right ) ));
        KJB(ASSERT( 00 == m_nil.parent ));

        // must find victim (vic)
        Node *vic = target -> right;
        for ( ; is_not_nil( vic -> left ); vic = vic -> left )
            ;

        KJB(ASSERT( is_not_nil( vic ) && is_nil( vic -> left ) ));

        /*
         * Now target assumes the identity of victim, by taking its key, its
         * satellite value, its locator, and its location_list entry.
         */
        target -> key = vic -> key;
        target -> sat = vic -> sat;
        Loc_tp &TaLo( target -> locator ), &ViLo( vic -> locator );
        std::swap( location_list[ TaLo ], location_list[ ViLo ] );
        std::swap( TaLo, ViLo );

        // update subtree-sums between target, victim (i.e., retrace the path).
        for ( Node* ppp = target -> right; ppp != vic; ppp = ppp -> left )
        {
            KJB(ASSERT( is_not_nil( ppp ) ));
            ppp -> sum -= vic -> key;
        }

        // splice out vic; vic's parent vp claims vic's right child (if any).
        *to_child = vic -> right;
        return semiauto_splice( vic, & Node::right );
    }


    /// @brief resolve "double" blackness on node xblack (could be the nil obj)
    void resolve_double_black( Node* xblack )
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        KJB(ASSERT( xblack ));
        KJB(ASSERT( m_nil.is_black ));
        KJB(ASSERT( root && is_not_nil( root ) ));

        if ( xblack -> is_red() )
        {
            // whoa, this is too easy!
            xblack -> is_black = true;
            return;
        }

        if ( xblack == root )
        {
            return; // ignore extra black at root b/c root has no sibling
        }

        // Let "me" = xblack.  Since I am black, I must have a sibling.
        Node* Node::* xb_br = parent_branch_to_me( xblack );
        Node* Node::* sib_br = opposite_direction( xb_br );
        Node* sib = xblack -> parent ->* sib_br;
        KJB(ASSERT( sib && is_not_nil( sib ) )); // CLRS page 290

        // If my sibling is red, it can be rotated to become my (black) parent.
        if ( sib -> is_red() )
        {
            return case_of_the_red_sibling(
                                            xblack,
                                            xb_br
#ifdef DEBUGGING
                                            ,sib
#endif
                                        );
        }

        Node    *near_nephew = sib ->* xb_br,
                *far_nephew = sib ->* sib_br;
        KJB(ASSERT( far_nephew && near_nephew )); // could be nil

        // Resolution depends on my nephews (possibly nil).  Tail recursion:
        if ( far_nephew -> is_red() )
        {
            return case_of_the_far_red_nephew( sib, xb_br );
        }

        if ( near_nephew -> is_black )
        {
            return case_of_the_black_sib_and_nephews( sib );
        }

        return case_of_the_near_red_nephew( xblack, sib, xb_br );
    }


    /**
      @brief case of the three black relatives (sibling and sibling's children)
     *
     * @param sib   pointer to sibling of the extra-black node
     *
     * We handle this by painting the sib red, so that the extra-blackness
     * problem floats a level higher in the tree.  That doesn't solve the
     * problem but it gets us a step closer to resolution.
     * You say, "you can't do that -- parent might be red?"  That's a fair
     * point but we will handle that contingency when we call
     * resolve_double_black() again.
     */
    void case_of_the_black_sib_and_nephews( Node* sib )
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        KJB(ASSERT( sib && is_not_nil( sib ) && sib -> is_black ));
        KJB(ASSERT( sib -> left && sib -> left -> is_black ));
        KJB(ASSERT( sib -> right && sib -> right -> is_black ));
        KJB(ASSERT( m_nil.is_black ));
        sib -> is_black = false;
        m_nil.parent = 00;

        return resolve_double_black( sib -> parent );
    }


    /**
     * Light at the end of the tunnel!   Thanks to the far red nephew, we
     * can shift (black) sibling into the path to the xblack, satisfying the
     * extra black requirement, and red nephew can simply be painted black to
     * make up for the loss of its parent.
     *
     * @param oldsib    sibling of the node with the extra black value
     *                  It is called "oldsib" because of the postcondition.
     * @param xb_br     Node field pointer indicating direction from the parent
     *                  of the extra-black node to the extra-black node (not
     *                  the sibling).
     *
     * @post the extra-black node has a new grandparent, its former sibling.
     * @post the extra-black node has a new uncle, its formerly red nephew.
     * @post the formerly red nephew is painted black so its blackheight does
     *       not decrease (despite the loss of its grandparent).
     * @post the former sibling and its parent swap colors.
     * @post the nil.parent pointer, which might have been temporarily allowed
     *      to point to a node, is (if so) restored to its usual zero value.
     */
    void case_of_the_far_red_nephew( Node* oldsib, Node* Node::* xb_br )
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        KJB(ASSERT( m_nil.is_black ));

        Node* Node::* sib_br = opposite_direction( xb_br );

        KJB(ASSERT( oldsib ));

        Node    *oldparent = oldsib -> parent,
                *old_far_nephew = oldsib ->* sib_br;
        KJB(ASSERT( oldparent && is_not_nil( oldparent ) ));
        KJB(ASSERT( oldsib == oldparent ->* sib_br ));
        KJB(ASSERT( oldsib -> is_black ));
        KJB(ASSERT( old_far_nephew -> is_red() ));

        rotate( to_parent_link( oldparent ), sib_br );
        old_far_nephew -> is_black = true;
        m_nil.parent = 00;
    }


    /**
     * @brief convert "only a near red nephew" case to "far red nephew" case.
     * @param xblack    points to node with "extra-black" status
     * @param oldsib    points to sibling node of *xblack, at first
     * @param xb_br     Node field pointer indicates direction from the parent
     *                  node of *xblack to *xblack.
     * @pre xblack has a non-nil sibling, oldsib
     * @pre oldsib has two non-nil children (nephews to xblack)
     * @pre both nephews are non nil, and they are different colors
     * @pre nephew nearer to xblack is red.
     * @post previously red nephew becomes future black sibling to xblack
     * @post previous sibling become future far red nephew to xblack
     * @post previous far black nephew becomes child to future far red nephew.
     *
     * The last step of this function is to call case_of_the_far_red_nephew().
     */
    void case_of_the_near_red_nephew(
        Node* xblack,
        Node* oldsib,
        Node* Node::* xb_br
    )
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        KJB(ASSERT( xblack -> parent == oldsib -> parent ));
        KJB(ASSERT( oldsib && is_not_nil( oldsib ) && oldsib -> is_black ));
        KJB(ASSERT( oldsib -> left && oldsib -> right ));
        KJB(ASSERT( m_nil.is_black ));

        Node    *old_red_nef = ( oldsib ->* xb_br ),
                *parent = xblack -> parent;

        // far nephew must be black too!  only the near nephew is red.
        KJB(ASSERT( ( oldsib ->* opposite_direction( xb_br ) ) -> is_black ));
        KJB(ASSERT( old_red_nef -> is_red() ));

        /* unzigzag xblack's parent,sib,near-nephew, make them swap colors.
         * oldsib becomes new far-nephew.  old near-nephew becomes new sib.
         *
         * Also, unzigzag likely corrupts xblack -> parent if *xblack is nil.
         * However, I think we have no need to care about that anymore.
         */
        unzigzag( parent, opposite_direction( xb_br ) );
        m_nil.parent = 00;
        oldsib -> is_black = false;

        Node *& new_blk_sib = old_red_nef;  // alias for clarity
        new_blk_sib -> is_black = true;

        // old_red_nef has become the new black sibling of xblack
        KJB(ASSERT( new_blk_sib == parent ->* opposite_direction( xb_br ) ));
        KJB(ASSERT( oldsib == new_blk_sib ->* opposite_direction( xb_br ) ));
        return case_of_the_far_red_nephew( new_blk_sib, xb_br );
    }


    /**
     * This case actually puts the extra-black pointer deeper in the tree,
     * and lengthens the trace by 1.  However, it can be proved that if you get
     * here, you only call resolve_double_black() one more time before you
     * return.
     * @param xblack    points to node (possibly nil) with extra-black status
     *                  If xblack points to the nil node, we suspend the usual
     *                  invariant "nil.parent==0" and require that
     *                  xblack->parent point to the specific parent of the node
     *                  with extra-black status.  In other words, we let nil
     *                  behave like a "real" node, temporarily.
     * @param xb_br     Node field pointer indicating the direction of descent
     *                  from the parent of *xblack to *xblack.
     * @param sib       points to sibling node of *xblack; it cannot be nil
     *                  because its parent would have an undefined blackheight.
     *
     * @pre As stated, xblack->parent must indicate the parent node of the node
     * with extra-black status, EVEN IF xblack indicates the nil node!  This is
     * contrary to the usual invariants of the tree.
     *
     * @post nil.parent==0 (restoring our usual invariants).
     */
    void case_of_the_red_sibling(
        Node* xblack,
        Node* Node::* xb_br
#ifdef DEBUGGING
        ,Node* sib
#endif
    )
    {
#ifdef DEBUGGING
        enter( __func__ );
        KJB(ASSERT( m_nil.is_black ));
        KJB(ASSERT( sib && is_not_nil( sib ) ));
        KJB(ASSERT( xblack -> parent == sib -> parent ));
        KJB(ASSERT( xblack -> is_black &&  sib -> is_red() ));
        KJB(ASSERT( xblack != root ));
#endif

        Node* xb_parent = xblack -> parent;

        Node* Node::* sib_br = opposite_direction( xb_br );
        rotate( to_parent_link( xb_parent ), sib_br );
        m_nil.parent = 00;

        // the following can be useful if xblack is nil, otherwise is benign:
        xblack -> parent = xb_parent;

        KJB(ASSERT( xb_parent -> is_red() ));
#ifdef DEBUGGING
        KJB(ASSERT( xb_parent == sib ->* xb_br ));
        KJB(ASSERT( sib -> is_black ));
#endif
        KJB(ASSERT( (xb_parent ->* sib_br) -> is_black ));
        KJB(ASSERT( (xb_parent ->* xb_br ) == xblack ));
        KJB(ASSERT( xblack -> is_black ));

        resolve_double_black( xblack );
    }


    /**
     * @brief remove any node you name from the tree
     *
     * @param target    Points to the node to be eliminated.
     *
     * This updates the subtree sums too.
     * During the course of this function, the invariant of "nil.parent==0" is
     * temporarily suspended, but restored at the end.
     *
     * The target named by the caller is either deleted, or its contents are
     * overwritten by the node with the next largest key in the tree.
     * This routine always executes a deletion, but it is therefore not always
     * done on the target.  Internally, 'tar2' points to the node to be
     * deleted.
     */
    void kill_a_node( Node* target )
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        KJB(ASSERT( 00 == m_nil.parent ));
        KJB(ASSERT( target && is_not_nil( target ) ));
        const Key_tp& query_key( target -> key );

        for ( Node* ppp = target; ppp != root; ppp = ppp -> parent )
        {
            ppp -> sum -= query_key;
        }
        root -> sum -= query_key;

        // Splice out target if possible, otherwise replace its contents
        Node *xblack, *tar2 = fully_auto_splice( target, & xblack );

        // *xblack is nil iff xblack -> parent equals tar2.
        KJB(ASSERT( 00 == tar2 || 00 == m_nil.parent || is_nil( xblack ) ));
        KJB(ASSERT( 00 == tar2 || m_nil.parent || is_not_nil( xblack ) ));

        if ( 00 == tar2 )
        {
            tar2 = del_child_not_target( target, & xblack );
        }

        KJB(ASSERT( tar2 ));
        KJB(ASSERT( is_nil( tar2 -> left ) || is_nil( tar2 -> right ) ));
        KJB(ASSERT( is_nil( tar2 -> left ) || xblack == tar2 -> left ));
        KJB(ASSERT( is_nil( tar2 -> right ) || xblack == tar2 -> right ));
        KJB(ASSERT( tar2 == root || xblack -> parent == tar2 -> parent ));
        KJB(ASSERT(     tar2 == root
                    ||  tar2 -> parent -> left == xblack
                    ||  tar2 -> parent -> right == xblack ));

        if ( tar2 -> is_black )
        {
            resolve_double_black( xblack );
        }
        m_nil.parent = 00;
        dispose( tar2 );
    }


    /// @brief common idiom for testing how the dictionary is represented now
    bool dictionary_is_small_linear_array() const
    {
        return size() <= TREE_THRESH;
    }


    /// @brief return the blackheight of the tree; BLACKHEIGHT_BAD if corrupt
    int blackheight_in_linear_time() const
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        if ( dictionary_is_small_linear_array() )
        {
            return 0;
        }
        return blackheight_in_linear_time( root );
    }

    /// @brief scan tree for red node with red child; if so, tree is corrupt
    bool red_node_has_red_child_in_linear_time() const
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        return red_node_has_red_child_in_linear_time( root );
    }

    /// @brief scan tree to verify the sums are good; if not, tree is corrupt
    bool sums_are_correct_in_linear_time() const
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif
        return sums_are_correct_in_linear_time( root );
    }


    /// @brief copy some other tree whose nil leaves are found in THAT tree.
    Node* recursive_copy(
        const Node* src,
        const Redblack_subtree_sum< SATELLITE_TYPE >& st
    )
    {
        KJB(ASSERT( src ));
        if ( st.is_nil( src ) ) // the other tree's nil node
        {
            return & m_nil;
        }

        Node* my_copy = new Node( *src );
        /* my_copy -> parent = 00; not actually necessary */
        my_copy -> link_left_child( recursive_copy( src -> left, st ) );
        my_copy -> link_right_child( recursive_copy( src -> right, st ) );
        m_nil.parent = 00;

        const Loc_tp l = src -> locator;
        location_list[ l ] = my_copy;

        return my_copy;
    }



#ifdef DEBUGGING
    /// @brief test the binary search tree invariant of the entire tree
    bool is_bst_in_linear_time() const
    {
        const Key_tp KMAX( std::numeric_limits< Key_tp >::max() );
        enter( __func__ );
        return is_bst_in_linear_time( root, -KMAX, +KMAX );
    }


    /// @brief test the binary search tree invariant of the given subtree
    bool is_bst_in_linear_time(
        const Node* ppp,
        const Key_tp& min,
        const Key_tp& max
    )   const
    {
        KJB(ASSERT( ppp ));
        if ( is_nil( ppp ) )
        {
            return true;
        }
        const Key_tp& kii = ppp -> key;

        /*
         * When we insert, we put equal keys in the right subtree, but when
         * we rotate, equal keys could end up in the left subtree too.
         * Thus, kii==min or kii==max is acceptable.
         */
        if ( kii < min || max < kii )   // equality is not a failure
        {
            return false;
        }

        const Node  *pl = ppp -> left,
                    *pr = ppp -> right;

        return      ( is_nil( pl ) || pl -> parent == ppp )
                &&  ( is_nil( pr ) || pr -> parent == ppp )
                &&  is_bst_in_linear_time( pr, kii, max )
                &&  is_bst_in_linear_time( pl, min, kii );
    }


#endif


    /// @brief insert a key into dictionary stored as a linear array
    Loc_tp linear_insert( const Key_tp& key, const Sat_tp& sat )
    {
        const int BLACK = 1;
        Loc_tp nuloc = location_list.obtain_avail_locator();
        root[ m_size ] = Node( key, sat, BLACK, & m_nil, & m_nil, nuloc );
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        KJB(ASSERT( 00 == location_list.at( nuloc ) ));
#endif
        location_list[ nuloc ] = root + m_size;
        ++m_size;
        return nuloc;
    }


    /// @brief insert key into dictionary stored as a tree
    Loc_tp tree_insert( const Key_tp& key, const Sat_tp& sat )
    {
        if ( TREE_THRESH == size() )
        {
            make_it_a_real_tree_now();
        }
        Node* n00b = new_node( key, sat, 0 ); // red
        insert_gp( &root, *n00b < *root, n00b );
        root -> is_black = true;
        ++m_size;
        KJB(ASSERT( 00 == m_nil.parent ));
        KJB(ASSERT( n00b ));
        return n00b -> locator;
    }



    /// @brief dictionary is a small linear array; try to find, remove a key.
    bool del_array_elt( size_t index )
    {
        KJB(ASSERT( index < size() ));

        Loc_tp loc = root[ index ].locator;
        location_list.release( loc );
        --m_size; // shrink the array size -- yes, a little bit premature

        /*
         * If the record we want to erase is anywhere except the last
         * array entry, then we clobber it with the last array entry.
         * The reason we already decremented m_size was to state this operation
         * a bit more cleanly.
         */
        if ( index < m_size )
        {
            // clobber the key and satellite data
            root[ index ] = root[ m_size ];
            // Clobbering record needs its location_list pointer updated.
            KJB(ASSERT( location_list[ root[index].locator ] == root+m_size ));
            location_list[ root[ index ].locator ] = root + index;
        }

        if ( 0 == m_size )
        {
            delete[] root;
        }

        return true;
    }


    /// @brief dictionary is a tree; try to find, remove a key.
    bool del_tree_node( Node* ppp )
    {
        KJB(ASSERT( ppp && location_list.is_good_here( ppp ) ));
        kill_a_node( ppp );
        --m_size;
        KJB(ASSERT( 00 == m_nil.parent ));

        // Test whether the dictionary has just now shrunk down to lin-arr size
        if ( TREE_THRESH == m_size )
        {
            condense_into_linear_array();
        }
        return true;
    }


    /// @brief search through tree for an extremal element (if any)
    Loc_tp loc_extremum(
        Node* Node::* branch,
        const Node* ( *pf )( const Node*, const Node* )
    )   const
    {
        if ( 0 == size() )
        {
            return 0;   // sentinel value for empty dictionaries
        }

        // handle the small linear array case
        if ( dictionary_is_small_linear_array() )
        {
            const Node* ppp = (*pf)( root, root + size() );
            return LOCATOR_BASE + ppp -> locator;
        }

        // common case: search the tree down its chosen branch
        KJB(ASSERT( root ));
        const Node* ppp;
        for ( ppp = root; ppp && is_not_nil( ppp ->* branch );
                                                        ppp = ppp ->* branch )
        {
            KJB(ASSERT( ppp ));
        }
        KJB(ASSERT( ppp && is_nil( ppp ->* branch ) ));
        return LOCATOR_BASE + ppp -> locator;
    }


    /// @brief implements loc_using_cumulative_key_sum for arrays
    Loc_tp array_seek_cukes( Key_tp cumulative_keysum ) const
    {
#if 0
        std::vector< Node > dicopy( root, root + size() );
        std::sort( dicopy.begin(), dicopy.end() ); // sort by keys
#else
        Node dicopy[ TREE_THRESH ];
        std::copy(root, root + size(), dicopy);
        std::sort( dicopy, dicopy + size() ); // sort by keys
        KJB(ASSERT(0 < size() && size() <= TREE_THRESH));
#endif
        Key_tp sum_so_far = 0;
        for ( size_t iii = 0; iii < size(); ++iii )
        {
            if ( cumulative_keysum <= ( sum_so_far += dicopy[ iii ].key ) )
            {
                return dicopy[ iii ].locator;
            }
        }
        KJB(ASSERT( sum_so_far < cumulative_keysum ));
        chatter( "sought a cumulative key sum larger than sum of all keys 1" );
        // ret loc to last record anyway -- maybe it's a floating point glitch.
        // We deliberately use at() here to throw in case that size is zero.
#if 0
        return dicopy.at( int(size()) - 1 ).locator;
#else
        return dicopy[ size() - 1 ].locator;
#endif
    }


    /// @brief recursively implements loc_using_cumulative_key_sum for trees
    Loc_tp tree_seek_cukes( const Node* ppp, Key_tp cumulative_keysum ) const
    {
        KJB(ASSERT( ppp && is_not_nil( ppp ) ));
        if ( are_both_children_nil( ppp ) )
        {
            if ( ppp -> key < cumulative_keysum )
            {
                chatter( "sought a cumulative key sum larger than "
                                                        "sum of all keys 2" );
            }
            return ppp -> locator;
        }
        const Key_tp lsum = is_not_nil( ppp -> left ) ? ppp -> left -> sum : 0;
        // Uncommon case:  go left.
        if ( is_not_nil( ppp -> left ) && cumulative_keysum <= lsum )
        {
            return tree_seek_cukes( ppp -> left, cumulative_keysum );
        }
        cumulative_keysum -= lsum;
        // Also uncommon case:  retrieve this node
        if ( cumulative_keysum <= ppp -> key )
        {
            return ppp -> locator;
        }
        // Degenerate case:  ck is too big but we cannot go right; we forgive.
        if ( is_nil( ppp -> right ) )
        {
            chatter( "sought a cumulative key sum larger than "
                                                        "sum of all keys 3" );
            return ppp -> locator;
        }
        cumulative_keysum -= ppp -> key;
        // Common case:  go right.  We have already shrunk c_k appropriately.
        return tree_seek_cukes( ppp -> right, cumulative_keysum );
    }


    /**
     * @brief copy a given tree
     * @pre this object must be empty
     * @post this object mimics the tree structure, including locators
     * @throws bad_alloc if memory allocation fails.
     *
     * If memory allocation fails, this object should be reset to an empty
     * state, but that has not been tested.
     * Locators in the old tree work in the new tree too.
     */
    void copy_tree_into_empty(
        const Redblack_subtree_sum< SATELLITE_TYPE >& tree
    )
    {
        KJB(ASSERT(0 == size()));
        KJB(ASSERT(location_list.empty()));

        if ( 0 == tree.size() ) return;
        root = 00; // must contain a defined value, if "catch" clause runs.

        try
        {
            location_list.replica_of(tree.location_list);
            m_size = tree.size();

            if ( tree.dictionary_is_small_linear_array() )
            {
                root = new Node[ TREE_THRESH ];
                std::copy(tree.root, tree.root + tree.size(), root);

                for ( size_t iii = 0; iii < tree.size(); ++iii )
                {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
                    KJB(ASSERT( root[iii].locator < location_list.size() ));
#endif
                    location_list[ root[iii].locator ] = root + iii;
                }
            }
            else
            {
                root = recursive_copy( tree.root, tree );
            }
        }
        catch (std::bad_alloc& e)
        {
            // If we cannot do the whole thing, retract any partial progress.
            clear();
            throw e;
        }
    }


    void set_error(const char* s) const
    {
#ifdef DEBUGGING
        kjb_c::set_error("Object is too small to have tree topology");
#endif
    }


    /**
     * @brief obtain pointer to node containing given locator, or return NULL
     *
     * This also calls set_error() with a message explaining the problem,
     * if a problem occurred.
     */
    const Node* look_up_loc_impl( Loc_tp query_loc ) const
    {
        if ( query_loc < LOCATOR_BASE )
        {
            set_error("Invalid locator");
            return 00;
        }

        query_loc -= LOCATOR_BASE; // this is why we pass in the loc by value

#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        if ( location_list.size() <= query_loc )
        {
            set_error("Bad, possibly expired, locator");
            return 00;
        }

        const Node* const ppp = location_list[ query_loc ];
        if ( 00 == ppp )
        {
            set_error("Expired locator");
            return 00;
        }
#else
        typename LocationList::LLCI i = location_list.find(query_loc);
        if (location_list.end() == i)
        {
            set_error("Expired locator");
            return 00;
        }
        const Node* const ppp = i -> second;
#endif
        KJB(ASSERT( ppp -> locator == query_loc ));
        return ppp;
    }

    /// alias for look_up_loc_impl
    const Node* look_up_locator( Loc_tp query_loc ) const
    {
        return look_up_loc_impl( query_loc );
    }

    /// non const version
    Node* look_up_locator( Loc_tp query_loc )
    {
        return const_cast<Node*>( look_up_loc_impl(query_loc) );
    }

public:

    /// @brief default ctor
    Redblack_subtree_sum()
    :   m_nil( 0, Sat_tp(), true, 00, 00, 00 ),
        m_size( 0 )
    {} // root is undefined, you got a problem with that???


    /**
     * @brief destroy the tree or the linear array.
     *
     * @post Naturally, this invalidates all iterators and locators.
     */
    void clear()
    {
        KJB(ASSERT( 00 == m_nil.parent ));
        if ( 0 == size() )
        {
            return;
        }

        if ( dictionary_is_small_linear_array() )
        {
            delete[] root;
        }
        else
        {
            recursive_destroy( root );
        }

        m_size = 0;
        location_list.clear();
    }


    /// @brief copy ctor (same locators will work in this one too)
    Redblack_subtree_sum( const Redblack_subtree_sum< SATELLITE_TYPE >& tree )
    :   m_nil( 0, Sat_tp(), true, 00, 00, 00 ),
        m_size( 0 )
    {
        copy_tree_into_empty( tree );
    }


    /**
     * @brief assignment operator (same locators will work in this one too)
     *
     * @post Naturally, this invalidates all previous iterators and locators.
     */
    Redblack_subtree_sum& operator=(
        const Redblack_subtree_sum< SATELLITE_TYPE >& tree
    )
    {
        if ( this != &tree )
        {
            clear();
            copy_tree_into_empty( tree );
        }
        return *this;
    }


    /// @brief dtor clears the structure
    ~Redblack_subtree_sum()
    {
        clear();
    }


    /**
     * @brief insert a new key+value pair in the dictionary
     *
     * @post This invalidates the iterators.
     */
    Loc_tp insert( const Key_tp& key, const Sat_tp& sat )
    {
        KJB(ASSERT( 00 == m_nil.parent ));
        if ( 0 == size() )
        {
            root = new Node[ TREE_THRESH ];
        }
        // Test whether we can do the insertion and STILL remain a linear array
        if ( size() < TREE_THRESH )
        {
            return LOCATOR_BASE + linear_insert( key, sat );
        }
        return LOCATOR_BASE + tree_insert( key, sat );
    }

    /// @brief the following is a hack to mimic StochasticPriorityQueue
    Loc_tp ins_max_key( const Sat_tp& sat )
    {
        return insert( std::numeric_limits< Key_tp >::max(), sat );
    }

#ifdef DEBUGGING
    /// @brief print the contents and structure of the dictionary
    void debug_print( std::ostream& os ) const
    {
        if ( 0 == size() )
        {
            os << "tree is empty\n";
        }
        else if ( dictionary_is_small_linear_array() )
        {
            os << "tiny tree is in an unsorted array, root=" << root << '\n';
            for ( size_t iii = 0; iii < size(); ++iii )
            {
                KJB(ASSERT( are_both_children_nil( root + iii ) ));
                recursive_db_print( root + iii, 0, os );
            }
        }
        else
        {
            os << "Big tree with nil at " << & m_nil
               << ", root=" << root << '\n';
            recursive_db_print( root, 0, os );
        }
        location_list.db_print_locators();
    }
#else
    /// @brief stub: remove print code when not checking aggressively
    void debug_print( std::ostream& ) const {}
#endif

    /// @brief scan dictionary in linear time to verify invariants are valid
    bool tree_valid_in_nlogn_time() const
    {
#ifdef DEBUGGING
        enter( __func__ );
        if ( m_nil.is_red() )
        {
            return fail( "nil is red" );
        }

        if ( m_nil.parent )
        {
            return fail( "nil points to a parent" );
        }


        if ( dictionary_is_small_linear_array() )
        {
            if (!location_list.locators_valid_in_nlogn_time(1, size(), root,0))
            {
                return fail( "locators are bad" );
            }
        }
        else
        {
            if (!location_list.locators_valid_in_nlogn_time(0,0, root, &m_nil))
            {
                return fail( "locators are bad" );
            }
            if  (       blackheight_in_linear_time() == BLACKHEIGHT_BAD
                    ||  root -> is_red()
                    ||  red_node_has_red_child_in_linear_time()
                    //  ||  ! sums_are_correct_in_linear_time()
                    ||  ! sums_are_close_enough_in_linear_time( root )
                    ||  ! is_bst_in_linear_time()
                )
            {
                return fail( "tree structure is bad" );
            }
        }
#endif
        return true;
    }

    /**
     * @brief search for a key in the dictionary; return first one.
     *
     * @param[in] key   key value in node to be searched for.
     * If multiple (key,value) pairs have been inserted in the dictionary using
     * the same key, it is undefined which one will be found.
     * Also remember the problems that occur when searching for exact
     * floating-point equality.
     * @param[out] sat_out  Optional output storage for satellite data.
     *                      If equal to NULL, satellite data is ignored.
     * @param[out] loc_out  Optional output storage for locator of this node
     *                      (biased).  If equal to NULL, locator is ignored.
     *
     * If the key is not found, the output locations are not written.
     *
     * @return true iff the exact key is found.
     */
    bool find_key( const Key_tp& key, Sat_tp* sat_out, Loc_tp* loc_out )
    {
        bool is_found;
        if ( dictionary_is_small_linear_array() )
        {
            is_found = linear_search( key, sat_out, loc_out, 00 );
        }
        else
        {
            is_found = tree_search( key, sat_out, root, loc_out );
        }

        // We bias the locator to make it a little bit more opaque.
        if ( is_found && loc_out )
        {
            *loc_out += LOCATOR_BASE;
        }

        return is_found;
    }


    /**
     * @brief erase a key+value pair from the dictionary
     *
     * @param[in] query_key Key for which to search.
     * If multiple key+value pairs have been inserted in the dictionary using
     * the same key, it is undefined which one will be found.
     *
     * @param[out] sat_out  Optional pointer to which, if not equal to NULL,
     * this will write the satellite data when and if the key value is found.
     *
     * @return true iff the key is found
     *
     * @post either this returns false, or one key+value pair is removed from
     * the dictionary having a key equal to that of query_key (xor).
     * As stated, it is undefined which key+value pair
     * is removed when there are multiple records sharing the same query_key
     * number, although of course the removed pair will be one of those
     * records.
     *
     * @post If the key is found, the iterators are invalidated, as is the
     * locator corresponding to the deleted node.
     */
    bool erase_key( const Key_tp& query_key, Sat_tp* sat_out )
    {
        KJB(ASSERT( 00 == m_nil.parent ));

        // handle the mini-array case
        if ( dictionary_is_small_linear_array() )
        {
            size_t index;
            if ( ! linear_search( query_key, sat_out, 00, & index ) )
            {
                return false; // not found
            }
            return del_array_elt( index );
        }

        // common case: dictionary is a tree
        Node* ppp = tree_search( query_key, sat_out, root, 00 );
        if ( 00 == ppp )
        {
            return false;
        }
        KJB(ASSERT( ppp -> key == query_key ));

        return del_tree_node( ppp );
    }


    /**
     * @brief access a record via its locator, a constant-time operation.
     *
     * @param[in] query_loc     Locator to look for in the tree.
     * @param[out] key_out      Optional pointer at which, if not eq. to NULL,
     *                          this will write the record's key value.
     * @param[out] sat_out      Optional pointer at which, if not eq. to NULL,
     *                          this will write the record's satellite field.
     * @return true iff success (i.e., the locator points to a valid record).
     * @warning Locators get recycled! Don't erase the same locator twice!
     */
    bool access_loc( Loc_tp query_loc, Key_tp* key_out, Sat_tp* sat_out ) const
    {
#if 0
        if ( query_loc < LOCATOR_BASE )
        {
            return false;
        }
        query_loc -= LOCATOR_BASE; // this is why we pass in the loc by value

#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        if ( location_list.size() <= query_loc )
        {
            return false;
        }

        const Node* ppp = location_list.at( query_loc );
        if ( 00 == ppp )
        {
            return false;
        }
#else
        typename LocationList::LLCI i = location_list.find(query_loc);
        if (location_list.end() == i) return false;
        const Node* const ppp = i -> second;
#endif
        KJB(ASSERT( ppp -> locator == query_loc ));
#else
        const Node* const ppp = look_up_locator( query_loc );
        if (00 == ppp) return false;
#endif

        if ( key_out )
        {
            *key_out = ppp -> key;
        }
        if ( sat_out )
        {
            *sat_out = ppp -> sat;
        }
        return true;
    }


    /**
     * @brief remove the record indicated by query_loc, or return false if bad
     * @param[in] query_loc     Locator of record to remove
     * @return true iff success (i.e., the locator points to a valid record).
     * @warning Locators get recycled! Don't erase the same locator twice!
     * @post This invalidates the query locator and all iterators.
     */
    bool erase_loc( Loc_tp query_loc )
    {
#if 0
        KJB(ASSERT( LOCATOR_BASE <= query_loc ));
        query_loc -= LOCATOR_BASE; // this is why we pass in the loc by value

#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        if ( location_list.size() <= query_loc )
        {
            return false;
        }
#else
        typename LocationList::LLCI i = location_list.find(query_loc);
        if (location_list.end() == i) return false;
#endif

        Node* ppp = location_list[ query_loc ];
        if ( 00 == ppp )
        {
            return false;
        }
        KJB(ASSERT( ppp -> locator == query_loc ));
#else
        Node* const ppp = look_up_locator( query_loc );
        if (00 == ppp) return false;
#endif

        // handle the linear array case
        if ( dictionary_is_small_linear_array() )
        {
            if ( ppp < root || root + size() <= ppp )
            {
                return false;
            }
            KJB(ASSERT( location_list.is_good_here( ppp ) ));
            return del_array_elt( ppp - root );
        }

        // common case: dictionary is a tree
        return del_tree_node( ppp );
    }



    /// @brief return the locator for the record with the minimum key, or 0
    Loc_tp loc_min() const
    {
        return loc_extremum( & Node::left, & std::min_element< const Node* > );
    }


    /// @brief return the locator for the record with the maximum key, or 0
    Loc_tp loc_max() const
    {
        return loc_extremum( & Node::right, & std::max_element< const Node* >);
    }


    /// @brief return the number of key+value pairs in the dictionary.
    size_t size() const
    {
        return m_size;
    }


    /// @brief return whether the size is zero
    bool is_empty() const
    {
        return 0 == m_size;
    }


    /// @brief change the key value for a node to a new value, O(log n) time
    bool rekey_loc( Loc_tp query_loc, const Key_tp& newkey )
    {
#ifdef DEBUGGING
        enter( __func__ );
#endif

#if 0
        if ( query_loc < LOCATOR_BASE )
        {
            return false;
        }

        query_loc -= LOCATOR_BASE; // this is why we pass in the loc by value

#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        if ( location_list.size() <= query_loc )
        {
            return false;
        }

        Node* const ppp = location_list[ query_loc ];
        if ( 00 == ppp )
        {
            return false;
        }
#else
        typename LocationList::LLCI i = location_list.find(query_loc);
        if (location_list.end() == i) return false;
        Node* const ppp = i -> second;
#endif
        KJB(ASSERT( ppp -> locator == query_loc ));
#else
        Node* const ppp = look_up_locator( query_loc );
        if (00 == ppp) return false;
        query_loc -= LOCATOR_BASE;
#endif

        // handle the linear array case:  unsorted, so just update key field
        if ( dictionary_is_small_linear_array() )
        {
            if ( ppp < root || root + size() <= ppp )
            {
                return false;
            }
            ppp -> key = newkey;
            return true;
        }

        // common case: dictionary is a tree
        // strategy:  insert an "impostor," a new node w/ new key, old sat
        Loc_tp newloc = insert( newkey, ppp -> sat ) - LOCATOR_BASE;
        Node* const qqq = location_list[ newloc ];
        KJB(ASSERT( qqq -> locator == newloc ));

        // now swap their locators, so impostor bears the query_loc locator
        std::swap( location_list[ query_loc ], location_list[ newloc ] );
        std::swap( ppp -> locator, qqq -> locator );
        KJB(ASSERT( location_list.is_good_here( ppp ) ));
        KJB(ASSERT( ppp == location_list[ newloc ] ));

        // now we can "disappear" the old node and no one will even notice.
        bool ok = del_tree_node( ppp );
        // the perfect crime: the old locator continues to work
        KJB(ASSERT( newkey == location_list[ query_loc ] -> key ));

        /*
         * Warning:  ppp could now be dangling, or qqq could now be dangling!
         * Neither is safe to use anymore!
         *
         * If *ppp had one child or was a leaf, then ppp is now dangling.
         * But if *ppp had two children, some other node was deleted and its
         * fields moved into *ppp, and the location list was also twiddled to
         * make *ppp look like the victim -- which is all FINE.  However, it is
         * possible (as I learned the hard way) that *qqq was this victim!
         * In which case, qqq is dangling.
         *
         * Conclusion:  do NOT assert qqq == location_list[ query_loc ].
         */

        return ok;
    }


    /// @brief return subtree sum at root node (i.e., the sum of all keys).
    Key_tp root_sum() const
    {
        if ( 0 == size() )
        {
            return 0;
        }
        if ( dictionary_is_small_linear_array() )
        {
            return std::accumulate( root, root+size(), Key_tp( 0 ), AddKey() );
        }
        return root -> sum;
    }

    /**
     * @brief fetch a node based on inorder cumulative sum of tree keys
     * @param cumulative_keysum value we want to find or minimally exceed.
     * @pre if tree contains nonpositive keys, behavior is undefined.
     * @return See exposition below:  returns locator of a node such that the
     *          sum of its key and the keys of all nodes less than it is at
     *          least cumulative_keysum, in a minimal sense.
     *
     * This method is most useful when keys are all positive.  Keys do not need
     * to be distinct.  If keys are nonpositive, screwy things will probably
     * occur.
     *
     * Let n denote a node and n.k denote its key.  Neither n or k will
     * be used to represent integers (but i, j will).
     *
     * Let n1,n2,n3,... be an inorder listing of nodes, such that
     * n1.k <= n2.k <= n3.k <= ...
     *
     * Now consider the cumulative sum of keys starting with n1:
     * s_0 = 0;  s_1 = s_0 + n1.k;  s_2 = s_1 + n2.k; ...
     * As a special case, let S equal the sum of all nodes' keys.
     *
     * If it exists, this method finds a value j such that
     * s_{j-1} < cumulative_keysum <= s_j
     * and if such a value exists, this returns the locator of node nj.
     *
     * Degenerate cases:
     * - If cumulative_keysum is nonpositive, this returns the locator of n1.
     * - If S <= cumulative_keysum this method returns the locator of the last
     *   node in the inorder listing.
     */
    Loc_tp loc_using_cumulative_key_sum( Key_tp cumulative_keysum ) const
    {
        if ( 0 == size() )
        {
            return 0;   // obviously bad because good locators are positive
        }
        if ( dictionary_is_small_linear_array() )
        {
            return LOCATOR_BASE + array_seek_cukes( cumulative_keysum );
        }
        return LOCATOR_BASE + tree_seek_cukes( root, cumulative_keysum );
    }

    /// @brief hack proxy to make this and a stochastic pri queue work alike
    Loc_tp Dijkstra_extraction() const
    {
        return loc_min();
    }

    /**
     * @brief get left/right* path from root to given node, in red-black tree
     *
     * @param[in] loc   location of target node to trace
     * @param[out] path pointer to output array of left vs. right links
     *                  from root to the target node.  See description.
     *                  Required -- return value is ERROR if equal to null.
     * @return NO_ERROR if successful; ERROR if object is too small to be a
     *         tree, or if 'path' equals null.
     *
     * Sometimes you might be interested in the red-black tree as an
     * interesting graph structure.  You might want to know the shape of the
     * path from root (if any) to a node.  This structure stores very small
     * collections of nodes in an array, so if size() is very small, there is
     * no tree topology to reveal.  If the return value is NO_ERROR then
     * path->size() tells you the depth of the node, and path->at(k) tells you
     * whether the direction of the kth link from root to node is a left link.
     * So node->at(0) is true iff the node is in the left subtree of the root,
     * node->at(1) is true iff the node is in the left subtree of the root's
     * child, and so on.
     */
    int get_topology_to_node(Loc_tp loc, std::vector<bool>* path) const
    {
        KJB(NRE(path));
        if ( dictionary_is_small_linear_array() )
        {
            set_error("Object is too small to have tree topology");
            return kjb_c::ERROR;
        }

        const Node* ppp = look_up_locator( loc );
        KJB(NRE( ppp ));
        KJB(ASSERT( ! is_nil( ppp ) ));

        std::vector<bool> toparents;
        while (ppp != root)
        {
            const Node* const parent = ppp -> parent;
            KJB(NRE( parent ));
            KJB(ASSERT( ppp == parent -> right || ppp == parent -> left ));
            toparents.push_back( ppp == parent -> left );
            ppp = parent;
            // Tree cannot be very deep; height of 60 would be very deep.
            KJB(ASSERT( toparents.size() < 60 ));
        }
        path -> resize( toparents.size() );
        std::copy( toparents.rbegin(), toparents.rend(), path -> begin() );

        return kjb_c::NO_ERROR;
    }

    /*
     * ==============================
     *           ITERATOR
     * ==============================
     *
     * The iterator enumerates all the nodes in the tree, using the
     * location_list vector.  That vector, as you know, contains pointers
     * to tree nodes, but also unused cells that contain NULL values.
     * Thus, this iterator has to skip over the NULLs.
     */
    friend class const_iterator;

    /// @brief iterator class for a tree -- lets you access node locators.
    class const_iterator
    :   public std::iterator< std::bidirectional_iterator_tag, Loc_tp >
    {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        size_t m_index;

        const Redblack_subtree_sum< Sat_tp > *m_tree;

        /**
         * @post m_index increases by the minimum value (possibly zero) to
         *       satisfy either of two conditions:  (a) equality with
         *       location_list.size(), the end value, or (b) pointing to a
         *       non-nil entry in location_list.
         *
         * Note that if m_index already points to a non-nil entry, nothing
         * happens.  If the tree is empty, condition (a) is what happens.
         */
        void advance_()
        {
            // skip index forward, over null pointers in location list
            while (   m_index < m_tree -> location_list.size()
                  &&  00 == m_tree -> location_list[m_index]
                  )
            {
                ++m_index;
            }
        }

        /**
         * @post m_index decreases by the minimum amount (possibly zero) to
         *       satisfy the same two conditions described for advance_().
         */
        void retreat_()
        {
            // skip index backward, over null pointers in location list
            while ( 0 < m_index && 00 == m_tree -> location_list[m_index] )
            {
                --m_index;
            }
            if (0 == m_index) advance_();
        }
#else
        /*
        typedef typename Redblack_subtree_sum< Sat_tp >::LLCI RBLLI;
        RBLLI m_it;
        */
        typename LocationList::LLCI m_it;
#endif

    public:
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        const_iterator(size_t ix, const Redblack_subtree_sum< Sat_tp > *t)
        :   m_index(ix),
            m_tree(t)
        {
            NTX(t);
            advance_();
        }
#else
        //const_iterator(RBLLI i) : m_it(i) {}
        const_iterator(typename LocationList::LLCI i) : m_it(i) {}
#endif

        Loc_tp operator*() const
        {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            const Node* n = m_tree -> location_list[m_index];
            KJB(ASSERT(n && n -> locator == m_index));
            return LOCATOR_BASE + m_index;
#else
            KJB(ASSERT(m_it -> second));
            KJB(ASSERT(m_it -> second -> locator == m_it -> first));
            return LOCATOR_BASE + m_it -> first;
#endif
        }

        bool operator==(const const_iterator& i) const
        {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            return m_index == i.m_index && m_tree == i.m_tree;
#else
            return m_it == i.m_it;
#endif
        }

        bool operator!=(const const_iterator& i) const
        {
            return ! operator==(i);
        }

        const_iterator& operator++()
        {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            ++m_index;
            advance_();
#else
            ++m_it;
#endif
            return *this;
        }
        const_iterator operator++(int)
        {
            const_iterator temp(*this);
            operator++();
            return temp;
        }
        const_iterator& operator--()
        {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
            --m_index;
            retreat_();
#else
            --m_it;
#endif
            return *this;
        }
        const_iterator operator--(int)
        {
            const_iterator temp(*this);
            operator--();
            return temp;
        }
    };

    const_iterator begin() const
    {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        return const_iterator(0, this);
#else
        return const_iterator(location_list.begin());
#endif
    }

    const_iterator end() const
    {
#if REDBLACK_H_2014_NOV_13_LOCATION_LIST_IMPL_USES_ARRAY
        return const_iterator(location_list.size(), this);
#else
        return const_iterator(location_list.end());
#endif
    }

#if 0
    /*
     * DOES NOT WORK -- as designed, the tree not entirely "pimpl" since it
     * has a member m_nil.  The leaf nodes all point to it.  If we swap the
     * representations we would have to change all the pointers formerly to
     * m_nil to the new member.  Otherwise, after the swap, ownership of m_nil
     * and ownership of the nodes pointing to it will differ, which is
     * intolerable.
     */
    void swap(Redblack_subtree_sum< SATELLITE_TYPE >& other)
    {
        std::swap(m_nil, other.m_nil);
        std::swap(root, other.root);
        std::swap(m_size, other.m_size);
        location_list.swap(other.location_list);
        free_locator_list.swap(other.free_locator_list);
    }
#endif
};


}
}


#if 0
/// DOES NOT WORK -- see comment attached to swap member function.
namespace std
{
    /// @brief swap the contents of two trees of the same type
    template <typename S>
    inline void swap(
        kjb::qd::Redblack_subtree_sum< S >& rb1,
        kjb::qd::Redblack_subtree_sum< S >& rb2
    )
    {
        rb1.swap(rb2);
    }
}
#endif

#endif /* REDBLACK_H_PREDOEHL_12_DEC_2011_VISION */
