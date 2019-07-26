#ifndef TREE_EVENT_H_
#define TREE_EVENT_H_

/*!
 * @file Tree_event.h
 *
 * @author Colin Dawson 
 * $Id: Tree_event.h 17119 2014-07-17 00:46:17Z cdawson $
 */

#include "semantics/Lexicon_db.h"
#include "semantics/Nonterminal_db.h"
#include "semantics/Semantic_elaboration.h"
#include "semantics/Event_traits.h"
#include <boost/make_shared.hpp>
#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>
#include <string>
#include <vector>
#include <iostream>

namespace semantics
{

    
class Tree_event
{
public:
    /*------------------------------------------------------------
     * TYPEDEFS
     *------------------------------------------------------------*/
    
    typedef boost::shared_ptr<Tree_event> Self_ptr;
    typedef Token_map::Val_type Value_type;
    typedef Token_map::Key_type Key_type;
    typedef std::vector<Value_type> Data_type;
    typedef Value_type Word_type;
    typedef Value_type Tag_type;
    typedef Value_type Label_type;
    typedef Value_type Distance_type;
    typedef boost::tuple<Word_type, Label_type, Label_type> Node_data;
    typedef Semantic_data_base::Hash_pair Sem_hash_pair;
public:
    /*------------------------------------------------------------
     * STATIC PUBLIC VARIABLES / ACCESSORS
     *------------------------------------------------------------*/
    
    static bool VERBOSE;
    static Lexicon_db& lexicon();
    static Nonterminal_db& nt_lexicon();
    static const int WORD = 0;
    static const int TAG = 1;
    static const int LABEL = 2;
    static const int HEAD = 0;
    static const int ARGS = 1;
    
protected:

    /*------------------------------------------------------------
     * CONSTRUCTORS/DESTRUCTOR
     *------------------------------------------------------------*/
    
    /*! @brief construct an empty event with data to be filled at derived level
     */
    Tree_event(bool learn);

    /*! @brief construct an event with data prespecified
     */
    Tree_event(const Data_type& data, bool learn);

public:
    /*! @brief virtual destructor
     */
    virtual ~Tree_event(){}

    /*------------------------------------------------------------
     * COMPUTATION
     *------------------------------------------------------------*/
    
    /*! @brief compute log probability of this event
     *  @param method use "Collins" to use Collins' style
     */
    virtual double log_probability(
        const bool& collins = false
        ) const = 0;
    
    /*------------------------------------------------------------
     * MANIPULATORS
     *------------------------------------------------------------*/
    
    /*! @brief turn off learn_ and remove counts from database
     */
    virtual void release_view_counts(){}
    /*! @brief turn on learn_ and add counts to database
     */
    virtual void reacquire_view_counts(){}

    /*! @brief update event views
     */
    virtual void update_event_views() = 0;

    /*! @brief update semantic parent and head (if they exist)
     */
    virtual void update_semantic_context(
	const Sem_hash_pair&,
	const Sem_hash_pair&
	) {}

    /*! @brief resample CRP table assignments for all component event views
     */
    virtual void resample_table_assignments() = 0;
    
    /*------------------------------------------------------------
     * DISPLAY
     *------------------------------------------------------------*/
    
    /*! @brief pure virtual dummy --- instantiated versions display count data
     */
    virtual void print_view_counts(std::ostream&) const = 0;

    /*! @brief display this node's data in human readable form
     */
    virtual void print(std::ostream& os) const = 0;

    /*! @brief virtual function, does nothing at base level
     */
    virtual void print_with_links(std::ostream&) const {}

    /*------------------------------------------------------------
     * FRIEND FUNCTIONS
     *------------------------------------------------------------*/
    
    /*! @brief display this node in tree context
     */
    friend
    std::ostream& operator<<(std::ostream& os, Tree_event& e)
    {
	e.print_with_links(os);
	return os;
    }

    friend
    std::ostream& operator<<(std::ostream& os, Tree_event* e)
    {
	e -> print_with_links(os);
	return os;
    }

    friend
    std::ostream& operator<<(std::ostream& os, Self_ptr e)
    {
	e -> print_with_links(os);
	return os;
    }

protected:
    /*! @brief return reference to the map between variable names and positions
     */
    virtual const Key_slots::Map& var_map() const = 0;
    /*! @brief look up data entry in position <variable> and return as word
     */
    const Key_type& data_as_word(const Key_type& variable) const;
    /*! @brief look up data entry in position <variable> and return as nt
     */
    const Key_type& data_as_nonterminal(const Key_type& variable) const;
    /*! @brief look up data entry in position <variable> and return as step code
     */
    const Key_type data_as_semantic_step(const Key_type& variable) const;
    /*! @brief look up data entry in position <variable> and return as semhead
     */
    const Key_type& data_as_semantic_head(const Key_type& variable) const;
    /*! @brief look up data entry in position <variable> and return as semargs
     */
    const Key_type& data_as_semantic_args(const Key_type& variable) const;
    /*! @brief look up data entry in position <variable> and return as code
     */
    const Value_type& data_as_code(const Key_type& variable) const;
    /*! @brief return data in position as lvalue
     */
    Value_type& data_in_slot(const Key_type& variable);
    
    Data_type data_;
    bool learn_;
};


/*------------------------------------------------------------
 * FREE FUNCTIONS
 *------------------------------------------------------------*/

template<class T>
void set_verbosity(bool verbose)
{
    T::VERBOSE = verbose;
}

template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& c)
{
    os << "(";
    for(typename std::vector<T>::const_iterator it = c.begin();
	it != c.end(); it++)
    {
	os << *it << ",";
    }
    os << ")";
    return os;
}

};

#endif
