#ifndef SYNTACTIC_EVENT_H_
#define SYNTACTIC_EVENT_H_

/*!
 * @file Syntactic_event.h
 *
 * @author Colin Dawson 
 * $Id: Syntactic_event.h 17119 2014-07-17 00:46:17Z cdawson $ 
 */

#include "semantics/Tree_event.h"
#include "semantics/Event_traits.h"
#include <deque>
#include <iosfwd>

namespace semantics
{

    
class Syntactic_event : public Tree_event
{
public:
    /*------------------------------------------------------------
     * TYPEDEFS
     *------------------------------------------------------------*/
    
    typedef boost::shared_ptr<Syntactic_event> Event_ptr;
    typedef Event_ptr Self_ptr; //temporary redundant typedef
    typedef std::deque<Self_ptr> Event_ptr_container;

    /*------------------------------------------------------------
     * STATIC MEMBERS
     *------------------------------------------------------------*/
    
    static bool VERBOSE;
protected:

    /*------------------------------------------------------------
     * CONSTRUCTORS/DESTRUCTOR
     *------------------------------------------------------------*/

    /*! @brief construct an empty event with data to be filled at derived level
     */
    Syntactic_event(int id, bool learn);

    /*! @brief construct an event with data prespecified
     */
    Syntactic_event(const Data_type& data, int id, bool learn);

    /*! @brief virtual destructor
     */
    virtual ~Syntactic_event() {};

public:
    /*------------------------------------------------------------
     * ACCESSORS
     *------------------------------------------------------------*/
    
    /*! @brief return unique id
     */
    int id() const {return id_;};


    /*! @brief return head position
     */
    size_t head_pos() const {return head_pos_;}
    
    /*------------------------------------------------------------
     * MANIPULATORS
     *------------------------------------------------------------*/
    
    /*! @brief add the first child
     */
    void add_head_child(const Event_ptr head)
    {
	assert(children_.empty());
	children_.push_back(head);
	head_pos_ = 0;
    }

    /*! @brief add a pointer to a new child which is not the head
     * @param child pointer to the child to be "adopted"
     * @param on_left boolean determining whether child is left of head
     */
    void add_dependency_child(const Event_ptr child, bool on_left = false)
    {
	if(on_left)
	{
	    children_.push_front(child);
	    ++head_pos_;
	}
	else
	{
	    children_.push_back(child);
	}
    }

    /*! @brief return a pointer to a deep copy of this instance
     */
    virtual Event_ptr get_a_copy(bool learn = false) const = 0;

    /*! @brief replace semantic data with new data and update views
     */
    virtual void update_semantics(const Sem_hash_pair&);

    /*! @brief update event views
     */
    virtual void update_event_views() {};

    /*------------------------------------------------------------
     * CALCULATION
     *------------------------------------------------------------*/
    
    /*! @brief get log probability of entire subtree
     */
    virtual double subtree_log_probability() const;


    /*------------------------------------------------------------
     * DISPLAY FUNCTIONS
     *------------------------------------------------------------*/
    
    /*! @brief pure virtual dummy --- instantiated versions display count data
     */
    virtual void print_view_counts(std::ostream&) const = 0;

    /*! @brief show counts at each node, recursively descending
     */
    virtual void print_subtree_view_counts(
	std::ostream&,
	int indent_level = 0
	) const;

    /*! @brief display this node's data in human readable form
     */
    virtual void print(std::ostream& os) const;

    /*! @brief display the semantic node associated with this node
     */
    virtual void print_semantics(std::ostream& os) const;

    /*! @brief virtual function, does nothing at base level
     */
    virtual void print_with_links(std::ostream&) const {}

    /*! @brief recursively print full subtree
     */
    virtual void print_subtree(std::ostream& os, int indent_level = 0) const;

    /*! @brief recursively print subtree as constituency tree with head info
     */
    virtual void print_constituency_tree_with_head(std::ostream& os) const;

    /*! @brief recursively print subtree as constituency tree with head info
     */
    void print_child_constituency_trees(std::ostream& os) const;

    /*------------------------------------------------------------
     * FRIEND FUNCTIONS
     *------------------------------------------------------------*/
    
    /*! @brief display this node in tree context
     */
    friend
    std::ostream& operator<<(std::ostream& os, const Syntactic_event& e)
    {
	e.print_with_links(os);
	return os;
    }
    friend
    std::ostream& operator<<(std::ostream& os, const Syntactic_event* e)
    {
	e -> print_with_links(os);
	return os;
    }
    friend
    std::ostream& operator<<(std::ostream& os, const Event_ptr e)
    {
	e -> print_with_links(os);
	return os;
    }

protected:
    /*! @brief return reference to the map between variable names and positions
     */
    virtual const Key_slots::Map& var_map() const = 0;

    /*! @brief print children subtrees in sequence
     */
    void print_child_trees(std::ostream& os, int indent_level) const;
    
    int id_;
    size_t head_pos_;
    Event_ptr_container children_;
};

};

#endif
