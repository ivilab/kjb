#ifndef PUNCTUATION_EVENT_H_
#define PUNCTUATION_EVENT_H_

/*!
 * @file Punctuation_event.h
 *
 * @author Colin Dawson 
 * $Id: Punctuation_event.h 17175 2014-07-29 19:11:35Z cdawson $ 
 */

#include "semantics/Syntactic_event.h"
#include <iosfwd>

namespace semantics
{

class Punctuation_event : public Syntactic_event
{
public:

    /*------------------------------------------------------------
     * CONSTRUCTORS AND DESTRUCTOR
     *------------------------------------------------------------*/
    
    /*! @brief this class doesn't do anything
     */
    Punctuation_event(int id) : Syntactic_event(id, false) {}

    /*------------------------------------------------------------
     * ACCESSORS
     *------------------------------------------------------------*/

    /*! @brief override function from Syntactic_event class
     */
    const Token_map::Key_type& data_as_word(
	const Token_map::Key_type&
	) const
    {
	return dummy;
    }
    
    /*! @brief override function from Syntactic_event class
     */
    const Token_map::Key_type& data_as_nonterminal(
	const Token_map::Key_type&
	) const
    {
	return dummy;
    }

    /*------------------------------------------------------------
     * MANIPULATORS
     *------------------------------------------------------------*/
    
    /*! @brief dummy function from virtual base version
     */
    void update_semantics(const Semantic_data_base::Hash_pair&){}

    /*! @brief resample CRP table assignments for all component event views (no-op)
     */
    void resample_table_assignments() {}

    /*------------------------------------------------------------
     * COMPUTATION
     *------------------------------------------------------------*/
    
    /*! @brief compute smoothed probability of this event
     */
    double log_probability(const bool&) const {return 0.0;};

    /*! @brief get a deep copy of this
     */
    Event_ptr get_a_copy(bool) const {return Event_ptr(new Punctuation_event(0));}

    /*------------------------------------------------------------
     * DISPLAY
     *------------------------------------------------------------*/
    
    /*! @brief override print function from Syntactic_event class
     */
    void print(std::ostream& os) const {os << dummy;}

    /*! @brief override print_semantics to do nothing
     */
    void print_semantics(std::ostream&) const {}

    /*! @brief print view counts (does nothing, in this case)
     */
    void print_view_counts(std::ostream&) const {}

    /*! @brief print with links
     */
    void print_with_links(std::ostream& os) const
    {
	os << data_as_nonterminal("");
    }

    /*! @brief override Syntactic_event::print_constituency_tree_with_head
     */
    void print_constituency_tree_with_head(std::ostream& os) const
    {
	print_child_constituency_trees(os);
    }

private:
    const Key_slots::Map& var_map() const
    {
	return PCC2_traits::variable_map;
    }

    void update_event_views(){}
    static const Token_map::Key_type dummy;
};

};

#endif
