#ifndef ROOT_EVENT_H_
#define ROOT_EVENT_H_

/*!
 * @file Root_event.h
 *
 * @author Colin Dawson 
 * $Id: Root_event.h 17119 2014-07-17 00:46:17Z cdawson $ 
 */


#include "semantics/Syntactic_event.h"
#include "semantics/Event_view.h"
#include "semantics/Event_traits.h"
#include <boost/shared_ptr.hpp>
#ifdef USE_SEMANTICS
#include "semantics/Elaboration_tree.h"
#endif


namespace semantics
{

class Root_event : public Syntactic_event
{
public:
    boost::shared_ptr<S1_view> s1_view;
    boost::shared_ptr<S2_view> s2_view;

    /*------------------------------------------------------------
     * CONSTRUCTORS AND DESTRUCTOR
     *------------------------------------------------------------*/
    
    /*! @brief constructor
     */
#ifdef USE_SEMANTICS
    Root_event(
	const Node_data&                      node_data,
	const Semantic_data_base::Hash_pair&  sem_data,
	int                                   id,
	bool                                  learn = true);
#endif
    Root_event(
	const Node_data&                         node_data,
	int                                      id,
	bool                                     learn = true);

    /*! @brief data constructor
     */
    Root_event(const Data_type& data, bool learn = false)
    : Syntactic_event(data, 0, learn)
    {
	update_event_views();
    }
    
    /*! @brief default destructor
     */
    ~Root_event(){}

    /*------------------------------------------------------------
     * COMPUTATION
     *------------------------------------------------------------*/
    
    /*! @brief compute smoothed probability of this event
     *  @param collins uses Collins smoothing if TRUE, CRP otherwise
     */
    double log_probability(const bool& collins = false) const;

    /*------------------------------------------------------------
     * DISPLAY
     *------------------------------------------------------------*/

    /*! @brief print bare node information
     */
    void print(std::ostream& os) const;
    
    /*! @brief print node in context
     */
    void print_with_links(std::ostream& os) const;

    /*! @brief print counts of various event "views"
     */
    void print_view_counts(std::ostream& os) const;

    /*------------------------------------------------------------
     * MANIPULATORS
     *------------------------------------------------------------*/

    /*! @brief release counts for associated views (i.e., treat as unobserved)
     */
    void release_view_counts()
    {
	if(learn_ == true)
	{
	    s1_view -> unlearn();
	    s2_view -> unlearn();
	    learn_ = false;
	}
    }

    /*! @brief reacquire counts for associated views (i.e., treat as observed)
     */
    void reacquire_view_counts()
    {
	if(learn_ == false)
	{
	    s1_view -> learn();
	    s2_view -> learn();
	    learn_ = true;
	}
    }

    /*! @brief resample CRP table assignments for all component event views
     */
    void resample_table_assignments()
    {
        s1_view -> resample_table_assignment();
        s2_view -> resample_table_assignment();
    }
    
    /*! @brief return a smart pointer to a deep copy of this instance
     */
    Event_ptr get_a_copy(bool learn = false) const
    {
	return Event_ptr(new Root_event(data_, learn));
    }

private:
    const Key_slots::Map& var_map() const
    {
	return S2_traits::variable_map;
    }
	
    void update_event_views()
    {
	s1_view =
	    boost::make_shared<S1_view>(data_.begin() + 1, data_.end(), learn_);
	s2_view =
	    boost::make_shared<S2_view>(data_.begin(), data_.end(), learn_);
    }

};

};

#endif
