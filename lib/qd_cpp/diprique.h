/**
 * @file
 * @brief interface for priority queues used by Dijkstra's algorithm
 * @author Andrew Predoehl
 */
/*
 * $Id: diprique.h 17555 2014-09-18 07:36:52Z predoehl $
 */

#ifndef DIPRIQUEUE_H_INCLUDED_PREDOEHL_UOFARIZONAVISION
#define DIPRIQUEUE_H_INCLUDED_PREDOEHL_UOFARIZONAVISION 1

namespace kjb
{
namespace qd
{

/**
 * @brief pure virtual interface for priority queue in Dijkstra's algorithm
 *
 * This class supports the interface requirements of a priority queue used by
 * Dijkstra's algorithm.  We can mildly tweak the behavior of the algorithm by
 * tweaking the behavior of the queue, but we won't say more about such tweaks
 * except to point to two classes:
 * - @ref Redblack_subtree_sum, a standard priority queue
 * - @ref StochasticPriorityQueue, a nonstandard, stochastic queue
 *
 * Obviously if you put a nonstandard priority queue, then you don't really
 * have Dijkstra's algorithm anymore, but I don't want to fuss about details.
 * I commend the term "quasi-Dijkstra" to describe such an algorithm.
 *
 * Like any container, SATELLITE_TYPE must be copyable.  It does not need to be
 * ordered.
 *
 * @section dpq_pri Priority
 *
 * Dijkstra's algorithm requires nodes (graph vertices) in the queue to have a
 * key, which in graph terms is called "distance" but also could be called
 * "energy," since it could represent the energy required to go from the source
 * to the given node.  Each iteration of a standard implementation extracts a
 * node keyed with minimum energy from the queue.  A tweaked queue might do
 * something else; this abstract interface takes no position.  Any
 * implementation of this interface should make the desired behavior available
 * in the form of the Dijkstra_extraction() method.
 *
 * When a node is inserted, it is keyed with an energy value that must be
 * nonnegative.  We also want to allow for an "infinity" key value.  The latter
 * operation is ins_max_key( node ), and the former is insert( energy, node ).
 * Each of those operations returns a "locator."
 *
 * @section dpq_loc Locators
 *
 * As you surely know, Dijkstra's algorithm requires one to alter (aka relabel,
 * aka rekey) the energies of nodes still in the queue.  This is implemented
 * via the Locator pattern:  a locator is an opaque value that serves like a
 * pointer to a node.  Each insert operation returns a locator, which the
 * caller can later use to revise the node's energy, using the rekey_loc()
 * method.  So you will probably want to store all those locators in an array.
 *
 * Given a locator, you can retrieve the node representation and its energy
 * using the access_loc() method.  You can erase the node using erase_loc().
 * To rekey a node use the rekey_loc() method.  Each of those methods returns
 * a boolean indicating whether the locator appeared to be valid (true means
 * valid, i.e., success).  The rekey_loc() changes the node's energy value but
 * it does not, of course, change the node's locator.
 *
 * Be careful with locators.  If you erase a node, but if you store its old
 * locator, that could be called a "dangling locator," and if you do a later
 * insertion, the old locator value might be recycled.
 */
template< typename SATELLITE_TYPE >
class DijkstraPriorityQueue {
public:
    typedef SATELLITE_TYPE  Sat_tp; ///< type of satellite data
    typedef float           Key_tp; ///< type that we use for keys
    typedef size_t          Loc_tp; ///< type that we use for locators

    /// @brief obligatory virtual destructor
    virtual ~DijkstraPriorityQueue() {}

    /// @brief we must be able to clear the queue and reuse it
    virtual void clear() = 0;

    /// @brief we must be able to insert a record associated with a key value
    virtual Loc_tp insert( const Key_tp&, const Sat_tp& ) = 0;

    /// @brief we must be able to insert a record with key value of "infinity"
    virtual Loc_tp ins_max_key( const Sat_tp& ) = 0;

    /// @brief we want to be able to access that record via its locator value
    virtual bool access_loc( Loc_tp, Key_tp*, Sat_tp* ) const = 0;

    /// @brief we want to erase a record via its locator value
    virtual bool erase_loc( Loc_tp ) = 0;

    /// @brief get the locator of the record with min (or near min) key
    virtual Loc_tp Dijkstra_extraction() const = 0;

    /// @brief get the number of elements in the queue
    virtual size_t size() const = 0;

    /// @brief predicate tests whether the queue is void of elements
    virtual bool is_empty() const = 0;

    /// @brief we must be able to change (reduce) the key value for a record
    virtual bool rekey_loc( Loc_tp, const Key_tp& ) = 0;
};

}
}

#endif /* DIPRIQUEUE_H_INCLUDED_PREDOEHL_UOFARIZONAVISION */
