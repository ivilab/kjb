#ifndef UNARY_EVENT_H_
#define UNARY_EVENT_H_

/*!
 * @file Unary_event.h
 *
 * @author Colin Dawson 
 * $Id: Unary_event.h 17119 2014-07-17 00:46:17Z cdawson $ 
 */

#include "semantics/Event_traits.h"
#include "semantics/Syntactic_event.h"
#include <boost/shared_ptr.hpp>
#ifdef USE_SEMANTICS
#include "semantics/Elaboration_tree.h"
#endif

namespace semantics
{

class Unary_event : public Syntactic_event
{
public:
    boost::shared_ptr<U_view> u_view;

    /// Constructors and destructors
    
    /*! @brief constructor from individual data elements
     */
    Unary_event(
	const Node_data&                   node_data,
#ifdef USE_SEMANTICS
	const Elaboration_tree::Hash_pair& sem_data,
	const Elaboration_tree::Hash_pair& psem_data,
#endif
	const Label_type&                  parent_label,
	int                                id,
	bool                               learn = true
	);

    /*! @brief construct from data vector directly
     */
    Unary_event(const Data_type& data, bool learn = false)
    : Syntactic_event(data, 0, learn)
    {
	update_event_views();
    }
    
    /// Computation

    /*! @brief compute smoothed probability of this event
     *  @param collins use Collins smoothing if TRUE, CRP otherwise
     */
    double log_probability(const bool& collins = false) const;

    /// Manipulators

    /*! @brief replace data about parent semantics and update views
     */
    void update_semantic_context(
    	const Sem_hash_pair& parent_semantics,
    	const Sem_hash_pair&
    	)
    {
    	boost::tie(data_in_slot("PSEM"), data_in_slot("PSEMARGS")) =
    	    parent_semantics;
    	update_event_views();
    }

    /*! @brief release counts for associated views (i.e., treat as unobserved)
     */
    void release_view_counts()
    {
	if(learn_ == true)
	{
	    u_view->unlearn();
	    learn_ = false;
	}
    }

    /*! @brief reacquire counts for associated events (i.e., treat as observed)
     */
    void reacquire_view_counts()
    {
	if(learn_ == false)
	{
	    u_view->learn();
	    learn_ = true;
	}
    }

    /*! @brief resample CRP table assignments for all component event views
     */
    void resample_table_assignments()
    {
        u_view -> resample_table_assignment();
    }
    
    /*! @brief produce a deep copy of this event and return a smart pointer
     */
    Event_ptr get_a_copy(bool learn = false) const
    {
	return Event_ptr(new Unary_event(data_, learn));
    }

    /// Display functions

    /*! @brief display this node in human readable form
     */
    void print(std::ostream& os) const;

    /*! @brief display the immediate path to this node
     */
    void print_with_links(std::ostream& os) const;

    /*! @brief display the count data associated with this node
     */
    void print_view_counts(std::ostream& os) const;

private:
    const Key_slots::Map& var_map() const
    {
	return U_traits::variable_map;
    }

    void update_event_views()
    {
	u_view = boost::make_shared<U_view>(data_.begin(), data_.end(), learn_);
    }
};

};


#endif
