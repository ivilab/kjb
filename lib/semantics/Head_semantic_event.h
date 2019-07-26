#ifndef HEAD_SEMANTIC_EVENT_H_
#define HEAD_SEMANTIC_EVENT_H_

/*!
 * @file Head_semantic_event.h
 *
 * @author Colin Dawson 
 * $Id: Head_semantic_event.h 17119 2014-07-17 00:46:17Z cdawson $ 
 */

#include "semantics/Semantic_step_event.h"
#include "semantics/Event_traits.h"
#include <boost/make_shared.hpp>

namespace semantics
{

    class Head_semantic_event : public Semantic_step_event
    {
    public:
	boost::shared_ptr<Hsem_view> hsem_view;

        /*------------------------------------------------------------
         * CONSTRUCTORS/DESTRUCTOR
         *------------------------------------------------------------*/
        
	/*! @brief constructor from individual data elements
	 */
	Head_semantic_event(
	    const Step_code_t&   step_code,
	    const size_t&        type_code,
	    const Node_data&     parent_data,
	    const Sem_hash_pair& parent_sem_data,
	    const bool           learn = true
	    );

	/*! @brief construct from data vector directly
	 */
	Head_semantic_event(const Data_type& data, bool learn = false)
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

	/*! @brief update semantic context features
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

	/*! @brief release counts for associated views (treat as unobserved)
	 */
	void release_view_counts()
	{
	    if(learn_ == true)
	    {
		hsem_view->unlearn();
		learn_ = false;
	    }
	}

	/*! @brief release counts for associated views (treat as unobserved)
	 */
	void reacquire_view_counts()
	{
	    if(learn_ == false)
	    {
		hsem_view->learn();
		learn_ = true;
	    }
	}

        /*! @brief resample CRP table assignments for all component event views
         */
        void resample_table_assignments()
        {
            hsem_view -> resample_table_assignment();
        }


	/*! @brief produce a deep copy of this event and return a smart pointer
	 */
	Event_ptr get_a_copy(bool learn = false) const
	{
	    return boost::make_shared<Head_semantic_event>(data_, learn);
	}

        /*------------------------------------------------------------
         * DISPLAY
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
	    return Hsem_traits::variable_map;
	}

	void update_event_views()
	{
	    hsem_view =
		boost::make_shared<Hsem_view>(
		    data_.begin(), data_.end(), learn_
		    );
	}
    };
};
	  
#endif
