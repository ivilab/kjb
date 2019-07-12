#ifndef EVENT_VIEW_H_
#define EVENT_VIEW_H_

/*!
 * @file Event_view.h
 *
 * @author Colin Dawson 
 * $Id: Event_view.h 17119 2014-07-17 00:46:17Z cdawson $ 
 */

#include "semantics/Marginal_cell.h"
#include "l_cpp/l_exception.h"
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/array.hpp>


namespace semantics
{

/*------------------------------------------------------------
 * FORWARD DECLARATIONS
 *------------------------------------------------------------*/

    template<typename T> struct View_traits;

/*! @class Event_view
 *  @brief Template class to represent different "views" of syntactic events
 */
template<class T, class Traits = View_traits<T> >
class Event_view;

/*------------------------------------------------------------
 * CLASS DEFINITIONS
 *------------------------------------------------------------*/
    
template<class T, class Traits>
class Event_view
{
public:

/*------------------------------------------------------------
 * TYPEDEFS
 *------------------------------------------------------------*/
    
    typedef Event_view<T, Traits> Self;
    typedef T Event;
    typedef Traits Event_traits;
    typedef Marginal_cell<Self, Traits::context_levels> M_cell;
    typedef typename M_cell::Key_type Key_type;
    typedef typename Key_type::Data_type Data_type;
    typedef typename M_cell::Map Map;

/*------------------------------------------------------------
 * PUBLIC STATIC MEMBERS
 *------------------------------------------------------------*/
    
    static Map& map;
    static const size_t& smoothing_levels;
    static bool VERBOSE;

/*------------------------------------------------------------
 * CONSTRUCTORS & DESTRUCTOR
 *------------------------------------------------------------*/
    
    Event_view(const bool learn = true) : cell_ptr_(), learn_(learn) {}

    /*! @brief Constructs an event from two container iterators
     */
    Event_view(
	typename Data_type::iterator first,
	typename Data_type::iterator last,
	bool                         learn = true
	);

    /*! @brief Copy constructor
     */
    Event_view(const Self& other)
    {
	values_ = other.values_;
	learn_ = other.learn_;
	initialize();
    }

    /*! @brief Destructor
     */
    virtual ~Event_view() {cleanup();}

/*------------------------------------------------------------
 * OPERATORS
 *------------------------------------------------------------*/

    /*! @brief Assignment operator
     */
    Self& operator=(const Self& source);

/*------------------------------------------------------------
 * CALCULATION
 *------------------------------------------------------------*/
    
    /*! @brief get counts of event at various smoothing levels
     */
    std::vector<int> get_numerators() const;
    
    /*! @brief get counts of context at various smoothing levels
     */
    std::vector<int> get_denominators() const;

    /*! @brief get counts of how many different events occur at this context
     */
    std::vector<int> get_diversities() const;

    /*! @brief get smoothed conditional probability via Collins' method
     */
    double smoothed_probability(const bool& collins = false) const
    {
        if(collins == true)
        {
            return cell_ptr_ -> smoothed_probability();
        } else {
            return cell_ptr_ -> predictive_probability();
        }
    };

    /*! @brief get predictive prob according to HCRP model
     */
    double predictive_probability() const
    {
	return cell_ptr_ -> predictive_probability();
    };
    
/*------------------------------------------------------------
 * STATIC FUNCTIONS
 *------------------------------------------------------------*/

    /*! @brief unconditional prior probability of a particular outcome
     *  @param val the code associated with the outcome whose probability is
     *         being computed
     *  @param type contextual information; used for only some types
     */
    static const double& prior_prob(const size_t& val, const size_t& type)
    {
	return Traits::prior_prob(val, type);
    }

    /*! @brief hard-coded diversity-weight parameter for this event type
     *  @result the value of the 'u' diversity parameter in Collins (2003)
     */
    static const double& diversity_weight(){return Traits::diversity_weight();}

    /*! @brief show output and context variables
     */
    void display_conditioning_expression(std::ostream& os) const;

/*------------------------------------------------------------
 * MANIPULATORS
 *------------------------------------------------------------*/

    /*! @brief remove counts from database (treat as unobserved)
     */
    void unlearn() {if(learn_ == true) {cleanup(); learn_ = false;}}

    /*! @brief add counts to database (treat as observed)
     */
    void learn() {if(learn_ == false) {learn_ = true; initialize();}}

    /*! @brief resample the CRP table assignment for this event
     */
    void resample_table_assignment()
    {
        if(learn_ == true)
        {
            cell_ptr_->decrement(table_code_);
            table_code_ = cell_ptr_->sample_table_assignment();
            cell_ptr_->increment(table_code_);
        }
    }
	
private:
    Key_type values_; /*!< Data container */
    size_t table_code_;
    typename M_cell::Base_ptr cell_ptr_; /*!< Pointer to event counting object */
    bool learn_; /*!< Flag indicating whether this event is part of training */
    void initialize(); /*!< Initialization function called at every construction */
    void cleanup(); /*!< Cleanup function called before destruction */
};


/// Implementation of member functions

template<class T, class Traits>
Event_view<T, Traits>::Event_view(
    typename Data_type::iterator first,
    typename Data_type::iterator last,
    const bool                   learn
    )
    : values_(first, last, Traits::size), table_code_(0), learn_(learn)
{
    initialize();
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
Event_view<T, Traits>& Event_view<T, Traits>::operator=(const Self& source)
{
    values_ = source.values_;
    table_code_ = source.table_code_;
    learn_ = source.learn_;
    initialize();
    return *this;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
std::vector<int> Event_view<T, Traits>::get_numerators() const
{
    using namespace boost;
    std::vector<int> result(smoothing_levels, 0);
    typename M_cell::Base_ptr p = cell_ptr_;
    for(size_t j = 0;
	p != NULL && j < smoothing_levels;
	j++
	)
    {
	if(p != NULL && p -> context() != NULL)
	{
	    result[j] = p -> count();
	    if(VERBOSE)
	    {
		std::cerr << "Level " << smoothing_levels - j << " key:"
			  << p -> key() << std::endl;
	    }
	    p = p -> margin();
	} 
    }
    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
std::vector<int> Event_view<T, Traits>::get_denominators() const
{
    using namespace boost;
    std::vector<int> result(smoothing_levels, 0);
    typename M_cell::Base_ptr p = cell_ptr_;
    for(size_t j = 0;
	j < smoothing_levels;
	j++)
    {
	if(p != NULL && p -> context() != NULL)
	{
	    result[j] = p -> context() -> count();
	    if(VERBOSE)
	    {
		std::cerr << "Level " << smoothing_levels - j << " key:"
			  << p -> context() -> key() << std::endl;
	    }
	    p = p -> margin();
	} 
    
    }
    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
std::vector<int> Event_view<T, Traits>::get_diversities() const
{
    using namespace boost;
    std::vector<int> result(smoothing_levels, 0);
    typename M_cell::Base_ptr p = cell_ptr_;
    for(size_t j = 0;
	j < smoothing_levels;
	j++
	)
    {
	if(p != NULL && p -> context() != NULL)
	{
	    result[j] = p -> context() -> diversity();
	    p = p -> margin();
	}
    }
    return result;
}


/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits> inline
void Event_view<T, Traits>::initialize()
{
    cell_ptr_ =
	M_cell::add_event(
	    values_,
	    Traits::step_sizes.rbegin(),
	    Traits::size,
	    Traits::out_size
	    );
    assert(cell_ptr_ != NULL);
    if(learn_ == true)
    {
         table_code_ = cell_ptr_->sample_table_assignment();
         cell_ptr_->increment(table_code_);
         assert(cell_ptr_->count() > 0);
    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits> inline
void Event_view<T, Traits>::cleanup()
{
    if(learn_ == true)
    {
        assert(cell_ptr_->count() > 0);
        cell_ptr_->decrement(table_code_);
    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
void Event_view<T, Traits>::display_conditioning_expression(
    std::ostream& os
    ) const
{
    using namespace std;
    typename Traits::Var_list::const_iterator marker;
    typename Traits::Var_list::const_iterator given =
	Traits::variable_names.begin() + Traits::out_size;
    for(marker = Traits::variable_names.begin(); marker != given; marker++)
    {
	os << *marker << " ";
    }
    os << "| ";
    for(typename Traits::Step_sizes::const_iterator it =
	    Traits::step_sizes.begin();
	it != Traits::step_sizes.end();
	it++)
    {
	int j = *it;
	while(j > 0)
	{
	    os << *marker << " ";
	    marker++;
	    j--;
	}
	os << ". ";
    }
    os << endl;
}

/*------------------------------------------------------------
 * STATIC INITIALIZATION
 *------------------------------------------------------------*/
    
template<class T, class Traits>
const size_t& Event_view<T, Traits>::smoothing_levels = Traits::context_levels;
    
template<class T, class Traits>
bool Event_view<T,Traits>::VERBOSE = false;

template<class T, class Traits>
typename Event_view<T,Traits>::Map&
Event_view<T,Traits>::map =
    Marginal_cell<Event_view<T,Traits>, Traits::context_levels>::map;

};

#endif
