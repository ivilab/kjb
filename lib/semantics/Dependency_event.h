#ifndef DEPENDENCY_EVENT_H_
#define DEPENDENCY_EVENT_H_

/*!
 * @file Dependency_event.h
 *
 * @author Colin Dawson 
 * $Id: Dependency_event.h 17119 2014-07-17 00:46:17Z cdawson $ 
 */

#include "semantics/Event_view.h"
#include "semantics/Event_traits.h"
#include "semantics/Syntactic_event.h"
#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>
#include <string>
#include <vector>
#ifdef USE_SEMANTICS
#include "semantics/Elaboration_tree.h"
#endif

namespace semantics
{
    
class Dependency_event : public Syntactic_event
{
public:
    boost::shared_ptr<D1_view> d1_view;
    boost::shared_ptr<D2_view> d2_view;
    boost::shared_ptr<PCC1_view> punc1_view;
    boost::shared_ptr<PCC2_view> punc2_view;
    boost::shared_ptr<PCC1_view> coord1_view;
    boost::shared_ptr<PCC2_view> coord2_view;
    
    /// Constructors and destructors

    /*! @brief "Bottom-up" constructor, specifying parent and head info directly
     */
    Dependency_event(
	const Node_data&                     node_data,
	const Node_data&                     parent_data,
	const Node_data&                     head_data,
#ifdef USE_SEMANTICS
	const Elaboration_tree::Hash_pair&   sem_data,
	const Elaboration_tree::Hash_pair&   parent_sem_data,
	const Elaboration_tree::Hash_pair&   head_sem_data,
#endif
	const Distance_type&                 distance,
	bool                                 punc_flag,
	const Node_data&                     punc_data,
	bool                                 coord_flag,
	const Node_data&                     coord_data,
	int                                  id,
	bool                                 learn = true
	);

    Dependency_event(
	const Data_type& data,
	const Data_type& punc_data,
	const Data_type& coord_data,
	bool             learn = false
	): Syntactic_event(data, 0, learn),
	   pcc_data_(punc_data),
	   coord_data_(coord_data)
    {
	update_event_views();
    }

    /// Calculation
    
    /*! @brief compute smoothed probability of this event
     */
    double log_probability(const bool& collins = false) const;

    /// Manipulation

    /*! @brief replace semantic context with new data and update views
     */
    void update_semantic_context(
	const Semantic_data_base::Hash_pair& parent_semantics,
	const Semantic_data_base::Hash_pair& head_semantics
	)
    {
	boost::tie(data_in_slot("PSEM"), data_in_slot("PSEMARGS")) =
	    parent_semantics;
	boost::tie(data_in_slot("HSEM"), data_in_slot("HSEMARGS")) =
	    head_semantics;
	update_event_views();
    }

    /*! @brief release counts for associated views (i.e., treat as unobserved)
     */
    void release_view_counts()
    {
	if(learn_ == true)
	{
	    d1_view->unlearn();
	    if(d2_view != NULL) d2_view->unlearn();
	    learn_ = false;
	}
    }

    /*! @brief reacquire counts for associated events (i.e., treat as observed)
     */
    void reacquire_view_counts()
    {
	if(learn_ == false)
	{
	    d1_view->learn();
	    if(d2_view != NULL) d2_view->learn();
	    learn_ = true;
	}
    }

    /*! @brief resample CRP table assignments for all component event views
     */
    void resample_table_assignments()
    {
        d1_view -> resample_table_assignment();
        d2_view -> resample_table_assignment();
    }


    Event_ptr get_a_copy(bool learn = false) const
    {
	return Event_ptr(
	    new Dependency_event(data_, pcc_data_, coord_data_, learn));
    }

    /// Display functions

    /*! @brief display immediate path to this node
     */
    void print_with_links(std::ostream& os) const;

    /*! @brief display count information associated with this node
     */
    void print_view_counts(std::ostream& os) const;
    
private:
    const Key_slots::Map& var_map() const
    {
	return D2_traits::variable_map;
    }

    void update_event_views();

    Data_type pcc_data_;
    Data_type coord_data_;
};

};

#endif
