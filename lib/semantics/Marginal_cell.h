#ifndef MARGINAL_CELL_H_
#define MARGINAL_CELL_H_

/*!
 * @file Marginal_cell.h
 *
 * @author Colin Dawson 
 * $Id: Marginal_cell.h 17175 2014-07-29 19:11:35Z cdawson $ 
 */

#include <semantics/Categorical_event.h>
#include <semantics/Cell_base_classes.h>
#include <semantics/Cell_traits.h>
#include <m_cpp/m_vector.h>
#include <prob_cpp/prob_distribution.h>
#include <prob_cpp/prob_sample.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/unordered_map.hpp>
#include <cmath>
#include <iostream>
#include <list>

/*------------------------------------------------------------
 * FORWARD DECLARATIONS
 *------------------------------------------------------------*/

namespace semantics
{

template<typename T> class Prior_cell;

/*! @class Marginal_cell
 *  @brief concrete class for contingency table cell
 */
template<typename T, size_t N> class Marginal_cell;

/*! @class Context_cell
 *  @brief class containing conditioning context for marginal cells
 */
template<typename T, size_t N> class Context_cell;


/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T>
class Prior_cell : public Marginal_cell_base
{
public:
    typedef Prior_cell<T> Self;
    typedef boost::shared_ptr<Prior_cell<T> > Self_ptr;
    typedef Marginal_cell_base::Self_ptr Base_ptr;
    typedef Context_cell_base::Self_ptr Context_base_ptr;
    typedef Categorical_event<0> Key_type;
    typedef boost::unordered_map<Key_type, Self_ptr> Map;
    static Map map;
public:
    Prior_cell(const Key_type& key) : key_(key) {}

    const Key_type& key() const {return key_;}
    
    void increment(const size_t&){}
    
    void decrement(const size_t&){}

    size_t sample_table_assignment() {return 0;}

    Base_ptr margin() const {return Base_ptr();}
    
    Context_base_ptr context() const {return Context_base_ptr();}
    
    double backoff_probability() const
    {
	return T::prior_prob(key_.front(), key_.back());
    }

    double smoothed_probability()
    {
        return backoff_probability();
    }

    double predictive_probability() const
    {
        return backoff_probability();
    }
    
    static Self_ptr add_event(
	const Key_type&                            key,
	Cell::Step_sizes::const_reverse_iterator,
	const size_t&,
	const size_t&
	)
    {
	Self_ptr result;
	typename Map::iterator it = map.find(key);
	if(it == map.end()) result = boost::make_shared<Self>(key);
	else result = it->second;
	return result;
    }
private:
    Key_type key_;
};

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
std::ostream& operator<<(
    std::ostream&,
    const typename Marginal_cell<T,N>::Map&
    );

template<class T>
std::ostream& operator<<(
    std::ostream&,
    const typename Marginal_cell<T,1>::Map&
    );

template<class T, size_t N>
std::ostream& operator<<(
    std::ostream&,
    const typename Context_cell<T,N>::Map&
    );

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
class Marginal_cell : public Marginal_cell_base
{
    // T is the event type
    // N is the smoothing level (higher values are less smoothed)
public:
    typedef Marginal_cell<T,N> Self;
    typedef boost::shared_ptr<Self> Self_ptr;
    typedef Cell_traits<T,N> Traits;
    typedef typename Marginal_cell_base::Self_ptr Base_ptr;
    typedef typename Context_cell_base::Self_ptr Context_base_ptr;
    typedef Categorical_event<N> Key_type;
    typedef boost::unordered_map<Key_type, Self_ptr> Map;
    typedef Context_cell<T,N> Context_type;
    typedef boost::shared_ptr<Context_type> Context_ptr;
    typedef typename Traits::Margin_type Margin_type;
    typedef boost::shared_ptr<Margin_type> Margin_ptr;
    // Associative container from keys to cells
    static Map map;
    static bool& VERBOSE;
public:
    /*------------------------------------------------------------
     * CONSTRUCTORS/DESTRUCTOR
     *------------------------------------------------------------*/
    
    /*! @brief ctor to create a new cell out of a key
     *  @param key container holding values of each categorical variable
     */
    Marginal_cell(const Key_type& key);

    /*! @brief copy ctor
     */
    Marginal_cell(const Marginal_cell& other);

    /*! @brief destructor
     */
    virtual ~Marginal_cell();

    /*------------------------------------------------------------
     * ACCESSORS
     *------------------------------------------------------------*/

    /*! @brief return data
     */
    const Key_type& key() const {return key_;}

    /*! @brief get smoothed cell
     */
    Base_ptr margin() const {return margin_;}

    /*! @brief get conditioning context
     */
    Context_base_ptr context() const {return context_;}

    /*------------------------------------------------------------
     * MANIPULATORS
     *------------------------------------------------------------*/

    /*! @brief increment this cell and its relevant margins and contexts
     */
    void increment(const size_t& table_code);
    
    /*! @brief decrement this cell and its relevant margins and contexts
     */
    void decrement(const size_t& table_code);

    /*! @brief increment the number of distinct tables with this value
     */
    void increment_table_count();
    
    /*! @brief decrement the number of distinct tables with this value
     */
    void decrement_table_count();
    /*------------------------------------------------------------
     * COMPUTATION
     *------------------------------------------------------------*/

    /*! @brief stochastically return a new table assignment for an observation
     *  @result a new local index value
     *
     * Uses the Hierarchical CRP mechanism to sample a new table assignment.
     * Includes bookkeeping to recycle table indices
     */
    size_t sample_table_assignment()
    {
        assert(!unused_table_codes_.empty());
        kjb::Categorical_distribution<size_t> table_weights(per_table_counts_);
        /// Add an extra weight for the unused component.  Note that, rather than
        /// compute (n_{table} / n_{context}) * (n_{context} / n_{context} + alpha)
        /// for the existing tables, and (alpha / (n_{context}+alpha)) * q(outcome),
        /// we simplify matters by rescaling by n_{context} + alpha, leaving
        /// proportional weights, n_{table} * 1 for existing tables, and alpha * q
        /// for the new table.
        table_weights.insert(
            unused_table_codes_.front(),
            context_->alpha() * backoff_probability()
            );
        size_t result = kjb::sample(table_weights);

        if(result == unused_table_codes_.front())
        {
            /// if we've selected the empty table, add it to the map.
            per_table_counts_.insert(
                std::pair<size_t, size_t>(unused_table_codes_.front(), 0)
                );
            /// now it is no longer empty
            unused_table_codes_.pop_front();
            used_table_codes_.push_back(result);
            if(unused_table_codes_.empty())
            {
                ///  We always recycle old codes
                ///  when possible, so if we need to generate a new one, it
                ///  must be that the existing ones form a contiguous range
                ///  starting from zero, so, size() will be the next available.
                unused_table_codes_.push_back(used_table_codes_.size());
            }
        }
        return result;
    }

    /*! @brief prob of this outcome conditioned on starting a new table
     *
     *  corresponds to q(x,H) in the writeup
     */
    double backoff_probability() const
    {
        return table_count_ / (context_->effective_tables()) +
            context_->table_backoff_weight() * margin_->backoff_probability();
    }

    /*! @brief predictive probability of this outcome in this context
     *
     * corresponds to p(x|beta(H)) in the writeup, but is not normalized.
     * normalization must occur later when comparing
     */
    double predictive_probability() const
    {
        return count_ / (context_->effective_observations()) +
            context_->observation_backoff_weight() * margin_->backoff_probability();
    }

    /*! @brief predictive probability under original Collins algorithm
     */
    double smoothed_probability()
    {
	update_probabilities();
	return smoothed_probability_;
    }

    /*------------------------------------------------------------
     * STATIC AND FRIEND FUNCTIONS
     *------------------------------------------------------------*/
    
    /*! @brief creates a new instance and sets its margin and context
     */
    static Self_ptr add_event(
	const Key_type&                           key,
	Cell::Step_sizes::const_reverse_iterator  first,
	const size_t&                             size,
	const size_t&                             out_size
	);

    /*! @brief creates a new context cell and returns a pointer to it
     */
    static Context_ptr add_context_event(
	const typename Context_type::Key_type& key);

    /*! @brief output a contingency table in human readable form to an ostream
     */
    friend std::ostream& operator<< <T,N>(std::ostream& os, const Map& map);
private:
    /*! @brief update smoothed conditional probability
     */
    void update_probabilities();
    
    const Key_type key_;
    Context_ptr context_;
    Margin_ptr margin_;
    std::list<size_t> used_table_codes_;
    std::list<size_t> unused_table_codes_;
    double smoothed_probability_;
};

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
class Context_cell : public Context_cell_base
{
public:
    // T is the event type
    // N is the smoothing level (higher values are less smoothed)

    /*------------------------------------------------------------
     * TYPEDEFS
     *------------------------------------------------------------*/
    
    typedef boost::shared_ptr<Context_cell<T, N> > Self_ptr;
    typedef Cell_traits<T,N> Traits;
    typedef Categorical_event<N> Key_type;
    typedef boost::unordered_map<Key_type, Self_ptr> Map;

    /*------------------------------------------------------------
     * STATIC MEMBERS
     *------------------------------------------------------------*/
    static Map map;
    static bool& VERBOSE;
    
public:
    /*------------------------------------------------------------
     * CONSTRUCTORS/DESTRUCTOR
     *------------------------------------------------------------*/
    
    /*! @brief Constructs a cell which is purely context information
     *  @param key container holding values of each categorical variable
     */
    Context_cell(const Key_type& key);

    /*! @brief copy ctor
     */
    Context_cell(const Context_cell& other);
    
    /*! @brief destructor
     */
    virtual ~Context_cell();

    /*------------------------------------------------------------
     * ACCESSORS
     *------------------------------------------------------------*/

    /*! @brief return data
     */
    const Key_type& key() const {return key_;}

    /*------------------------------------------------------------
     * STATIC AND FRIEND FUNCTIONS
     *------------------------------------------------------------*/

    /*! @brief resample all CRP alpha values at this level only
     */
    static void resample_all_alphas();
    
    /*! @brief output a contingency table in human readable form to an ostream
     */
    friend std::ostream& operator<< <T,N>(
	std::ostream& os,
	const Map&    map
	);
private:
    const Key_type key_;
};


/*------------------------------------------------------------
 * MEMBER IMPLEMENTATION (Context_cell)
 *------------------------------------------------------------*/

template<class T, size_t N>
Context_cell<T,N>::Context_cell(const Key_type&  key)
: Context_cell_base(), key_(key)
{
    if(VERBOSE)
    {
	std::cerr << "Constructed context event at level " << N
		  << " with key " << key_ << std::endl;
    }
}


/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
Context_cell<T,N>::Context_cell(
    const Context_cell& other
    ) : Cell(other),
	key_(other.key_)
{
    /// DEBUG
    std::cerr << "COPYING Context_cell<T,N>" << std::endl;
    /// ~DEBUG
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
Context_cell<T,N>::~Context_cell()
{}

/*------------------------------------------------------------
 * STATIC MEMBER IMPLEMENTATION (Context_cell)
 *------------------------------------------------------------*/

template<class T, size_t N>
void Context_cell<T,N>::resample_all_alphas()
{
    for(typename Map::iterator it = map.begin();
        it != map.end(); ++it)
    {
        it->second->resample_alpha();
    }
}

//// This is a horrendous hack to simulate partial specialization of
//// a template function.  Surely there's a better way.
    
template<size_t M>
class Dummy_template_class
{};
    
template<size_t M>
static const Dummy_template_class<M>& create_dummy_template_class()
{
    static Dummy_template_class<M> d = Dummy_template_class<M>();
    return d;
}
    
template<class T>
class Alpha_sampler
{
public:
    
    template<size_t M>
    static void recursively_resample_all_alphas(
        const Dummy_template_class<M>&
        )
    {
        Context_cell<T,M>::resample_all_alphas();
        recursively_resample_all_alphas(create_dummy_template_class<M-1>());
    }

    static void recursively_resample_all_alphas(
        const Dummy_template_class<1>&
        )
    {
        Context_cell<T,1>::resample_all_alphas();
    }

};

//// End horrendous hack    
    
/*------------------------------------------------------------
 * MEMBER IMPLEMENTATION (Marginal_cell)
 *------------------------------------------------------------*/

template<class T, size_t N>
Marginal_cell<T,N>::Marginal_cell(const Key_type& key)
: Marginal_cell_base(),
  key_(key),
  context_(),
  margin_(),
  used_table_codes_(),
  unused_table_codes_(1,0),
  smoothed_probability_(1.0)
{}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
Marginal_cell<T,N>::Marginal_cell(
    const Marginal_cell& other
    ) : Cell(other),
	key_(other.key_),
	margin_(other.margin_),
	context_(other.context_)
{}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
Marginal_cell<T,N>::~Marginal_cell()
{}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
void Marginal_cell<T,N>::increment(const size_t& table_code)
{
    margin_->increment(table_code);
    if(count() == 0) context_->increment_diversity();
    context_->increment();
    if(per_table_counts_[table_code] == 0) {increment_table_count();}
    ++per_table_counts_[table_code];
    Cell::increment();
    if(VERBOSE)
    {
	std::cerr << "Incremented marginal cell"
		  << "at level " << N << " with key " << key_
		  << " to " << count()
		  << std::endl;
    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
void Marginal_cell<T,N>::decrement(const size_t& table_code)
{
    assert(per_table_counts_[table_code] > 0);
    /// Decrement the overall and table-specific count
    Cell::decrement();
    --per_table_counts_[table_code];
    /// If so, recycle the table code and decrement number of tables
    if(per_table_counts_[table_code] == 0)
    {
        unused_table_codes_.push_front(table_code);
        used_table_codes_.remove(table_code);
        decrement_table_count();
        per_table_counts_.erase(table_code);
    }
    context_->decrement();
    /// If there are no observations with this outcome, decrement diversity
    if(count() == 0) context_->decrement_diversity();
    margin_->decrement(table_code);
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
void Marginal_cell<T,N>::increment_table_count()
{
    margin_->increment_table_count();
    context_->increment_table_count();
    Cell::increment_table_count();
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
void Marginal_cell<T,N>::decrement_table_count()
{
    Cell::decrement_table_count();
    context_->decrement_table_count();
    margin_->decrement_table_count();
}
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
void Marginal_cell<T,N>::update_probabilities()
{
    const int f = context_ -> count();
    const int u = context_ -> diversity();
    const double next_prob = margin_ -> smoothed_probability();
    if(f == 0 && u == 0)
    {
	smoothed_probability_ = next_prob;
    } else {
	assert(f > 0 && u > 0); // can't have positive f without positive u;
	const double bo = T::diversity_weight() * u;
	assert(bo > 0);
	smoothed_probability_ = log(bo * exp(next_prob) + count()) - log(f + bo);
    }
}

/*------------------------------------------------------------
 * FREE FUNCTIONS
 *------------------------------------------------------------*/

template<typename T, size_t N>
void set_contingency_table_verbosity(bool value) 
{
    Marginal_cell<T,N>::VERBOSE = value;
    Context_cell<T,N>::VERBOSE = value;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<typename T, size_t N>
typename Marginal_cell<T,N>::Self_ptr Marginal_cell<T,N>::add_event(
    const Key_type&                                      key,
    Cell::Step_sizes::const_reverse_iterator             first,
    const size_t&                                        size,
    const size_t&                                        out_size
    )
{
    Self_ptr result;
    typename Map::iterator it = map.find(key);
    if(it == map.end())
    {
	typename Context_type::Key_type context_key = get_context(key, out_size);
	typename Margin_type::Key_type margin_key = smooth(key, *first);
	result = boost::make_shared<Self>(key);
	map.insert(typename Map::value_type(key,result));
	result->context_ = add_context_event(context_key);
	result->margin_ =
	    Margin_type::add_event(
		margin_key, first + 1, size - *first, out_size);
    } else {
	result = it->second;
    }
    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<typename T, size_t N>
typename Marginal_cell<T,N>::Context_ptr
Marginal_cell<T,N>::add_context_event(
    const typename Context_type::Key_type&  key)
{
    Context_ptr result;
    typename Context_type::Map::iterator it = Context_type::map.find(key);
    if(it == Context_type::map.end())
    {
	result = boost::make_shared<Context_type>(key);
	Context_type::map.insert(
	    typename Context_type::Map::value_type(key,result)
	    );
    } else {
	result = it->second;
    }
    return result;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
std::ostream& operator<<(
    std::ostream&                             os,
    const typename Marginal_cell<T,N>::Map&   map
    )
{
    for(typename Marginal_cell<T,N>::Map::const_iterator it = map.begin();
	it != map.end();
	it++)
    {
	os << (it->first) << " : " << (it->second->count()) << std::endl;
    }
    return os;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T>
std::ostream& operator<<(
    std::ostream&                             os,
    const typename Marginal_cell<T,1>::Map& map
    )
{
    for(typename Marginal_cell<T,1>::Map::const_iterator it = map.begin();
	it != map.end();
	it++)
    {
	os << "Total : " << (it->second->count()) << std::endl;
    }
    return os;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
std::ostream& operator<<(
    std::ostream&                             os,
    const typename Context_cell<T,N>::Map&    map
    )
{
    for(typename Context_cell<T,N>::Map::const_iterator it = map.begin();
	it != map.end();
	it++)
    {
	os << (it->first) << " : " << (it->second->count()) << std::endl;
    }
    return os;
}


/*------------------------------------------------------------
 * STATIC INITIALIZATION
 *------------------------------------------------------------*/

template<class T, size_t N>
bool& Marginal_cell<T, N>::VERBOSE = Cell::VERBOSE;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, size_t N>
bool& Context_cell<T, N>::VERBOSE = Cell::VERBOSE;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<typename T>
typename Prior_cell<T>::Map Prior_cell<T>::map;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<typename T, size_t N>
typename Marginal_cell<T, N>::Map Marginal_cell<T, N>::map;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<typename T, size_t N>
typename Context_cell<T, N>::Map Context_cell<T, N>::map;

}; // namespace semantics
    
#endif
