#ifndef CATEGORICAL_EVENT_H_
#define CATEGORICAL_EVENT_H_

/*!
 * @file Categorical_event.h
 *
 * @author Colin Dawson 
 * $Id: Categorical_event.h 17097 2014-07-05 00:13:22Z cdawson $ 
 */

#include "l_cpp/l_exception.h"
#include <boost/functional/hash.hpp>
#include <vector>
#include <iostream>

namespace semantics
{
    
/*! @class Categorical_event_base
 *  @brief abstract base class for look-up keys
 */
class Categorical_event_base;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*! @class Categorical_event
 *  @brief look-up key class for contingency table
 */
template<size_t N> class Categorical_event;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*! @brief marginalizes look-up key by <steps> from back (generalizes context)
 */ 
template<size_t N>
Categorical_event<N-1> smooth(
    const Categorical_event<N>& event,
    const size_t                steps = 1);

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*! @brief returns RHS of conditioning expression
 */ 
template<size_t N>
Categorical_event<N> get_context(
    const Categorical_event<N>& event,
    const size_t                steps = 1);

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*! @brief display key in human readable form
 */
std::ostream& operator<<(std::ostream&, const Categorical_event_base&);


/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

class Categorical_event_base
{
public:
    typedef Categorical_event_base Self_type;
    typedef size_t Val_type;
    typedef std::vector<Val_type> Data_type;
    typedef size_t Hash_type;
    virtual std::ostream& print_to(std::ostream& os) const {return os;}
    virtual bool operator==(const Self_type&) const {return false;}
};

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<size_t N>
class Categorical_event : public Categorical_event_base
{
public:

/*------------------------------------------------------------
 * TYPEDEFS
 *------------------------------------------------------------*/

    typedef Categorical_event<N> Self_type;

/*------------------------------------------------------------
 * CONSTRUCTORS & DESTRUCTOR
 *------------------------------------------------------------*/
    
    /*! @brief default ctor
     */

    Categorical_event() : size_(0), data_(), hash_val_(0) {}
    
    /*! @brief construct look-up key from a standard container,
     *  @param data standard container containing lookup key data
     *  @param size intended size of lookup key (for consistency checking)
     */
    Categorical_event(Data_type data, size_t size);
    
    /*! @brief alternative ctor using iterators
     *  @param first iterator to beginning of data range
     *  @param last iterator to end of data range
     *  @param size asserted size, for consistency check
     */
    Categorical_event(
	Data_type::const_iterator first,
	Data_type::const_iterator last,
	size_t                    size
	);

    /*! @brief copy constructor
     */
    Categorical_event(const Self_type& other)
    : Categorical_event_base(),
      size_(other.size_),
      data_(other.data_),
      hash_val_(other.hash_val_)
    {}

/*------------------------------------------------------------
 * ACCESSORS
 *------------------------------------------------------------*/
    
    /*! @brief get length of data key
     */
    const size_t& size() const {return size_;}

    /*! @brief get first data entry
     */
    const size_t& front() const {return data_.front();}

    /*! @brief get last data entry
     */
    const size_t& back() const {return data_.back();}

/*------------------------------------------------------------
 * OPERATORS
 *------------------------------------------------------------*/
    
    /*! @brief two keys are equal iff their hash values are equal
     */
    bool operator==(const Self_type& other) const;

/*------------------------------------------------------------
 * DISPLAY FUNCTIONS
 *------------------------------------------------------------*/
    
    /*! @brief print key data to ostream os
     */
    std::ostream& print_to(std::ostream& os) const;

/*------------------------------------------------------------
 * FRIEND FUNCTIONS
 *------------------------------------------------------------*/
    
    /*! @brief get smoothed key (context simplified by one level)
     */
    friend Categorical_event<N-1> smooth<N>(
	const Self_type& event,
	const size_t     steps);

    /*! @brief get RHS of conditioning context
     */
    friend Categorical_event<N> get_context<N>(
	const Self_type& event,
	const size_t     steps);

    /*! @brief return underlying hash value
     */
    friend size_t hash_value(Self_type const& e) {return e.hash_val_;}
    
private:
    size_t size_; /*!< size of data key */
    Data_type data_; /*!< implementation of data key */
    Hash_type hash_val_; /*!< underlying hash value */
    
    /*! @brief verify size is as claimed, otherwise throw
     */
    void check_size() const
    {
	if(data_.size() != size_)
	{
	    std::cerr << "Data has size: " << data_.size() << ","
		      << "Size required is " << size_ << std::endl;
	    KJB_THROW_2(
		kjb::Illegal_argument,
		"Categorical_event constructed with incorrect length.");
	}
    }
};

/*------------------------------------------------------------
 * TEMPLATE MEMBER FUNCTION DEFINITIONS
 *------------------------------------------------------------*/

template<size_t N> inline
Categorical_event<N>::Categorical_event(Data_type data, size_t size)
    : size_(size),
      data_(data),
      hash_val_(boost::hash_range(data.begin(), data.end()))
{
    try
    {
	check_size();
    }
    catch(kjb::Illegal_argument& e)
    {
	std::cout << e.get_msg() << std::endl;
    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<size_t N> inline
Categorical_event<N>::Categorical_event(
	Data_type::const_iterator first,
	Data_type::const_iterator last,
	size_t                    size
	) : size_(size),
	    data_(first, last),
	    hash_val_(boost::hash_range(first, last))
{
    try
    {
	check_size();
    }
    catch(kjb::Illegal_argument& e)
    {
	std::cout << e.get_msg() << std::endl;
    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<size_t N> inline
bool Categorical_event<N>::operator==(const Categorical_event<N>& other) const
{
    return hash_val_ == other.hash_val_;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<size_t N>
std::ostream& Categorical_event<N>::print_to(std::ostream& os) const
{
    if(data_.size() == 0)
    {
	os << "NULL";
    } else {
	os << "(";
	for(Data_type::const_iterator it = data_.begin();
	    it != data_.end(); it++)
	{
	    os << *it << ",";
	}
	os << ")";
    }
    return os;
}


/*------------------------------------------------------------
 * TEMPLATE FREE FUNCTION DEFINITIONS
 *------------------------------------------------------------*/

template<size_t N>
Categorical_event<N-1> smooth(
    const Categorical_event<N>& event,
    const size_t steps
    )
{
    if(steps > event.size())
    {
	KJB_THROW_2(
	    kjb::Illegal_argument,
	    "steps argument of smooth() too large for event");
    }
    return Categorical_event<N-1>(
	event.data_.begin(), event.data_.end() - steps,
	event.size() - steps);
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<size_t N>
Categorical_event<N> get_context(
    const Categorical_event<N>& event,
    const size_t steps)
{
    return Categorical_event<N>(
	event.data_.begin() + steps, event.data_.end(),
	event.size() - steps);
}

}; //namespace semantics

#endif
