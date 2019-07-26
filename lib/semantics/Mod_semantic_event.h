#ifndef MOD_SEMANTIC_EVENT_H_
#define MOD_SEMANTIC_EVENT_H_

/*!
 * @file Mod_semantic_event.h
 *
 * @author Colin Dawson 
 * $Id: Mod_semantic_event.h 17119 2014-07-17 00:46:17Z cdawson $ 
 */

#include "semantics/Semantic_step_event.h"
#include "semantics/Event_traits.h"
#include <boost/make_shared.hpp>

namespace semantics
{

    class Mod_semantic_event : public Semantic_step_event
    {
    public:
	boost::shared_ptr<Msem_view> msem_view;

        /*------------------------------------------------------------
         * CONSTRUCTORS/DESTRUCTOR
         *------------------------------------------------------------*/
        
	/*! @brief constructor from individual data elements
	 */
	Mod_semantic_event(
	    const Step_code_t&   step_code,
	    const size_t&        type_code,
	    const Node_data&     parent_data,
	    const Node_data&     head_data,
	    const Sem_hash_pair& parent_sem_data,
	    const Sem_hash_pair& head_sem_data,
	    const Distance_type& dist,
	    bool                 learn = true
	    );

	/*! @brief construct from data vector directly
	 */
	Mod_semantic_event(const Data_type& data, bool learn = false)
	: Semantic_step_event(data, learn)
	{
	    update_event_views();
	}

	/*------------------------------------------------------------
         * COMPUTATION
         *------------------------------------------------------------*/

        /*! @brief compute smoothed probability of this event
	 */
	double log_probability(const bool& collins = false) const;

        /*------------------------------------------------------------
         * MANIPULATORS
         *------------------------------------------------------------*/

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

	/*! @brief release counts for associated views (treat as unobserved)
	 */
	void release_view_counts()
	{
	    if(learn_ == true)
	    {
		msem_view->unlearn();
		learn_ = false;
	    }
	}

	/*! @brief release counts for associated views (treat as unobserved)
	 */
	void reacquire_view_counts()
	{
	    if(learn_ == false)
	    {
		msem_view->learn();
		learn_ = true;
	    }
	}

        /*! @brief resample CRP table assignments for all component event views
         */
        void resample_table_assignments()
        {
            msem_view -> resample_table_assignment();
        }

	/*! @brief produce a deep copy of this event and return a smart pointer
	 */
	Event_ptr get_a_copy(bool learn = false) const
	{
	    return boost::make_shared<Mod_semantic_event>(data_, learn);
	}
	
        /*------------------------------------------------------------
         * DISPLAY FUNCTIONS
         *------------------------------------------------------------*/
        
	/*! @brief display the immediate path to this node
	 */
	void print_with_links(std::ostream& os) const;

	/*! @brief display the count data associated with this node
	 */
	void print_view_counts(std::ostream& os) const;

    private:
	const Key_slots::Map& var_map() const
	{
	    return Msem_traits::variable_map;
	}

	void update_event_views()
	{
	    msem_view =
		boost::make_shared<Msem_view>(
		    data_.begin(), data_.end(), learn_
		    );
	}
    };
};
	  
#endif
