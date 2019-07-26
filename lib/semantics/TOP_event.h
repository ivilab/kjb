#ifndef TOP_EVENT_H_
#define TOP_EVENT_H_

/*!
 * @file TOP_event.h
 *
 * @author Colin Dawson 
 * $Id: TOP_event.h 17119 2014-07-17 00:46:17Z cdawson $ 
 */

#include "semantics/Syntactic_event.h"
#include "semantics/Lexicon_db.h"
#include "semantics/Nonterminal_db.h"

namespace semantics
{

class TOP_event : public Syntactic_event
{
public:

    /*------------------------------------------------------------
     * CONSTRUCTORS AND DESTRUCTOR
     *------------------------------------------------------------*/
    
    /*! @brief standard constructor, one argument per piece of data
     */
    TOP_event(int id)
    : Syntactic_event(id, false)
    {}

    /*------------------------------------------------------------
     * ACCESSORS
     *------------------------------------------------------------*/
    
    /*! @brief override function from Syntactic_event class
     */
    const Token_map::Key_type& data_as_word(
	const Token_map::Key_type&
	) const
    {
	return Nonterminal_db::root_key();
    }
    
    /*! @brief override function from Syntactic_event class
     */
    const Token_map::Key_type& data_as_nonterminal(
	const Token_map::Key_type&
	) const
    {
	return Nonterminal_db::root_key();
    }

    /*------------------------------------------------------------
     * COMPUTATION
     *------------------------------------------------------------*/
    
    /*! @brief compute smoothed probability of this event
     */
    double log_probability(const bool&) const {return 0.0;};

    /*! @brief get log probability of entire subtree
     */
    double subtree_log_probability() const
    {
	return Syntactic_event::subtree_log_probability();
    }
    
    /*! @brief get a deep copy of this
     */
    Event_ptr get_a_copy(bool) const {return Event_ptr(new TOP_event(0));}

    /*------------------------------------------------------------
     * MANIPULATORS
     *------------------------------------------------------------*/

    /*! @brief update semantic feature associated with this event (no-op)
     */
    void update_semantics(const Sem_hash_pair&) {}

    /*! @brief resample CRP table assignments for all component event views (no-op)
     */
    void resample_table_assignments() {}

    /*------------------------------------------------------------
     * DSIPLAY
     *------------------------------------------------------------*/
    
    /*! @brief override print function from Syntactic_event class
     */
    void print(std::ostream& os) const {os << Nonterminal_db::root_key();}

    /*! @brief override print_semantics function to do nothing
     */
    void print_semantics(std::ostream&) const {}

    /*! @brief override Syntactic_event::print_subtree
     */
    void print_subtree(std::ostream& os, int indent_level) const
    {
	print_child_trees(os, indent_level);
    }

    /*! @brief override Syntactic_event::print_constituency_tree_with_head
     */
    void print_constituency_tree_with_head(std::ostream& os) const
    {
	os << "(" << Nonterminal_db::root_key() << " ";
	print_child_constituency_trees(os);
	os << head_pos_-1 << ")";
    }
    
    /*! @brief print view counts (does nothing, in this case)
     */
    void print_view_counts(std::ostream&) const {}

    /*! @brief print with links
     */
    void print_with_links(std::ostream&) const {}
private:
    const Key_slots::Map& var_map() const
    {
	return S2_traits::variable_map;
    }

    void update_event_views(){}
};

};

#endif
