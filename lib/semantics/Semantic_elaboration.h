#ifndef SEMANTIC_OBJECT_H_
#define SEMANTIC_OBJECT_H_

/*!
 * @file Semantic_object.h
 *
E * @author Colin Dawson 
 * $Id: Semantic_elaboration.h 17357 2014-08-22 00:12:57Z cdawson $ 
 */

#include "semantics/Semantic_db.h"
#include "semantics/SemanticIO.h"
#include "l_cpp/l_exception.h"
#include <map>
#include <deque>
#include <algorithm>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/array.hpp>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string/join.hpp>

namespace semantics
{

/*------------------------------------------------------------
 * FORWARD DECLARATIONS
 *------------------------------------------------------------*/    
        
/*! @class Semantic_data_base
 *  @brief Abstract base class of Semantic Data template
 */ 
class Semantic_data_base;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*! @class Semantic_elaboration
 *  @brief Base class for semantic elaborations
 *
 *  Currently two derived classes: Terminal_elaboration, which covers
 *  attributes (color, size), and non-terminal elaborations, which
 *  are unary and binary predicates.
 */
class Semantic_elaboration;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*! @class Semantic_traits
 *  @brief Traits class storing information specific to given elaboration types
 */
template<class T> class Semantic_traits;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*! @class Semantic_data
 *  @brief Concrete semantic data class for storing hash codes about nodes
 * 
 *  template parameter T is the type of elaboration (Color_primitive,
 *  Size_primitive, Binary_relation, etc.)
 */  
template<class T, class Traits = Semantic_traits<T> > class Semantic_data;
    
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*! @class Terminal_elaboration
 *  @brief Class representing semantic nodes at the leaves of trees
 */
template<class T, class Traits = Semantic_traits<T> >
class Terminal_elaboration;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*! @class Terminal_elaboration
 *  @brief Class representing semantic nodes not at the leaves of trees
 */
template<class T, class Traits = Semantic_traits<T> >
class Nonterminal_elaboration;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
std::ostream& operator<<(std::ostream&, Semantic_data<T, Traits>);

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

class Null_primitive;

typedef Terminal_elaboration<Null_primitive> Null_semantic_terminal;

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*! @brief creates a new "null" terminal node (w/ no semantic content)
 */
const boost::shared_ptr<Null_semantic_terminal>&
null_semantic_terminal();
    
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*------------------------------------------------------------
 * CLASS DEFINITIONS
 *------------------------------------------------------------*/    
    
class Semantic_data_base
{
public:
    /*------------------------------------------------------------
     * TYPEDEFS
     *------------------------------------------------------------*/
    
    typedef Semantic_db::Val_type Val_type;
    typedef Semantic_db::Key_type Key_type;
    typedef boost::shared_ptr<Semantic_data_base> Base_ptr;
    typedef boost::tuple<size_t, size_t> Hash_pair;
    typedef std::map<size_t, std::string> Map;
    typedef std::pair<size_t, std::string> Map_entry;

    friend class Semantic_elaboration;
public:
    /*------------------------------------------------------------
     * CONSTRUCTORS/DESTRUCTOR
     *------------------------------------------------------------*/
    
    virtual ~Semantic_data_base(){}

    /*------------------------------------------------------------
     * ACCESSORS
     *------------------------------------------------------------*/
    
    virtual const Val_type& head() const = 0;
    virtual const size_t& head_code() const = 0;
    virtual const size_t& args_code() const = 0;
    virtual const Hash_pair& hash_pair() const = 0;

    /*------------------------------------------------------------
     * STATIC ACCESSORS
     *------------------------------------------------------------*/

    static Map& global_head_map()
    {
	static boost::shared_ptr<Map> ghm(new Map());
	return *ghm;
    }
    
    static Map& global_arg_map()
    {
	static boost::shared_ptr<Map> gam(new Map());
	return *gam;
    }
};

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 
    
template<class T, class Traits>    
class Semantic_data : public Semantic_data_base
{
public:
    /*------------------------------------------------------------
     * TYPEDEFS
     *------------------------------------------------------------*/
    
    typedef Semantic_data<T, Traits> Self_type;
    typedef boost::shared_ptr<Semantic_data<T, Traits> > Self_ptr;
    typedef boost::array<Val_type, Traits::n_args> Arg_list;
    typedef boost::array<Key_type, Traits::n_args> Arg_list_s;
    typedef boost::array<Token_map*, Traits::n_args> Arg_map_list;

    friend class Nonterminal_elaboration<T, Traits>;
public:
    /*------------------------------------------------------------
     * CONSTRUCTORS/DESTRUCTOR
     *------------------------------------------------------------*/
    
    /*! @brief standard constructor
     */
    Semantic_data(const Val_type& head, const Arg_list& args);

    /*! @brief constructor from labels rather than codes
     */
    Semantic_data(const Key_type& head, const Arg_list_s& args);

    /*! @brief construct an argless node from a code
     */
    Semantic_data(const Val_type& head);

    /*! @brief construct an argless node from a key
     */
    Semantic_data(const Key_type& head);

    /*------------------------------------------------------------
     * ACCESSORS
     *------------------------------------------------------------*/
    
    /*! @brief return the actual arg tuple
     */
    const Arg_list& args() const {return args_;}

    /*! @brief return the actual head value
     */
    const Val_type& head() const {return head_;}

    /*! @brief return the actual head value
     */
    const Key_type& head_s() const {return head_map().decode(head_);}
    
    /*! @brief return hash code for head
     */
    const size_t& head_code() const {return hash_pair_.get<0>();}

    /*! @brief return hash code for arg list
     */
    const size_t& args_code() const {return hash_pair_.get<1>();}

    /*! @brief return hash pair for head and args
     */
    const Hash_pair& hash_pair() const {return hash_pair_;}

    /*------------------------------------------------------------
     * FRIEND FUNCTIONS
     *------------------------------------------------------------*/
    
    /*! @brief display head and args readably
     */
    friend std::ostream& operator<< <T,Traits>(std::ostream& os, Self_type d);
public:

    /*------------------------------------------------------------
     * STATIC ACCESSORS
     *------------------------------------------------------------*/
    
    static Semantic_db& head_map() 
    {
	static boost::shared_ptr<Semantic_db> hm(new Semantic_db());
	return *hm;
    }
    static Arg_map_list& arg_map_list() 
    {
	static boost::shared_ptr<Arg_map_list> aml(new Arg_map_list);
	return *aml;
    }
    
private:
    const Val_type head_;
    const Arg_list args_;
    Hash_pair hash_pair_;

    void initialize_codes();
};


/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*! @class
 *  @brief Abstract base class for semantic elaborations
 */

class Semantic_elaboration
{
public:
   /*------------------------------------------------------------
    * TYPEDEFS
    *------------------------------------------------------------*/
    
    typedef Semantic_data_base::Key_type Key_type;
    typedef Semantic_data_base::Val_type Val_type;
    typedef Semantic_data_base::Hash_pair Hash_pair;
    typedef boost::shared_ptr<Semantic_elaboration> Self_ptr;
    typedef boost::shared_ptr<const Semantic_elaboration> Self_ptr_const;
    typedef std::deque<Self_ptr_const> Self_ptrs_read;
    typedef Semantic_data_base::Base_ptr Data_base_ptr;
    typedef boost::tuple<Self_ptr_const, Semantic_data_base::Hash_pair> Step_result;
    typedef int Referent_code;
    typedef std::vector<Referent_code> Referent_list;
    static const Referent_code NULL_REFERENT;
public:
    /*------------------------------------------------------------
     * CONSTRUCTORS/DESTRUCTOR
     *------------------------------------------------------------*/
    
    virtual ~Semantic_elaboration(){}

    /*------------------------------------------------------------
     * ACCESSORS
     *------------------------------------------------------------*/
    
    /*! @brief return underlying hash code for head value
     */
    virtual const Val_type& head() const = 0;
    
    /*! @brief return underlying hash code for head value
     */
    virtual const Key_type& head_s() const = 0;
    
    /*! @brief return underlying hash code for head value
     */
    virtual const Val_type& arg(const size_t& pos) const = 0;
    
    /*! @brief return underlying hash code for head value
     */
    virtual const Key_type& arg_s(const size_t& pos) const = 0;
    
    /*! @brief return underlying hash code for head value
     */
    virtual const size_t& head_code() const = 0;
    
    /*! @brief return underlying hash code for args
     */
    virtual const size_t& args_code() const = 0;

    /*! @brief return the code corresponding to the type of elaboration
     */
    virtual size_t type_code() const = 0;

    /*! @brief return pointer to underlying data node
     */
    virtual const Data_base_ptr data() const = 0;

    /*! @brief return a binary filter indicating what steps are allowed
     */
    virtual const Step_code::Weights& step_filter() const = 0;

    /*! @brief return pointer to child in a particular position
     */
    virtual Self_ptr child(const size_t&) const = 0;

    /*! @brief return pointer to child in a particular position
     */
    virtual Self_ptrs_read children() const = 0;

    /*! @brief get the integral code for the referent (if it exists)
     */
    virtual const Referent_code& head_referent() const {return NULL_REFERENT;}
    
    /*! @brief get the integral code for the arg in position pos (if it exists)
     */
    virtual const Referent_code& arg_referent(const size_t&) const
    {
        return NULL_REFERENT;
    }
    
    /*! @brief step down the tree according to step_code
     */
    virtual Step_result take_a_step(const Step_code_t& step_code) const = 0;
    
    /*------------------------------------------------------------
     * MANIPULATORS
     *------------------------------------------------------------*/
    
    /*! @brief put a child in a particular position
     */
    virtual void add_child(const size_t&, const Self_ptr) = 0;

    /*! @brief add child and have existing child adopt the old one
     */
    virtual void insert_child(const size_t&, const Self_ptr, const size_t&) = 0;

    /*------------------------------------------------------------
     * QUERIES
     *------------------------------------------------------------*/
    
    /*! @brief is this a terminal node?
     */
    virtual bool is_terminal() const = 0;

    /*------------------------------------------------------------
     * DISPLAY FUNCTIONS
     *------------------------------------------------------------*/
    
    /*! @brief print contents at this node
     */
    virtual void print(std::ostream& os) const = 0;
    
    /*! @brief recursively print elaboration tree contents
     */
    virtual void print_subtree(std::ostream&, int indent_level = 0) const = 0;

    /*------------------------------------------------------------
     * FRIEND FUNCTIONS
     *------------------------------------------------------------*/

    friend std::ostream& operator<<(std::ostream& os, Self_ptr);
};


/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
class Terminal_elaboration : public Semantic_elaboration
{
public:
    /*------------------------------------------------------------
     * TYPEDEFS
     *------------------------------------------------------------*/
    
    typedef Semantic_elaboration::Self_ptr Base_ptr;
    typedef Traits Traits_t;
    typedef boost::shared_ptr<Semantic_data<T> > Data_ptr;
public:
    /*------------------------------------------------------------
     * CONSTRUCTORS/DESTRUCTOR
     *------------------------------------------------------------*/
    
    /*! @brief construct with null head value
     */
    Terminal_elaboration();
    
    /*! @brief construct from numeric code for head value
     */
    Terminal_elaboration(const Val_type& head);

    /*! @brief construct from string for head category
     */
    Terminal_elaboration(const Key_type& head);

    /*------------------------------------------------------------
     * ACCESSORS
     *------------------------------------------------------------*/
    
    /*! @brief return code for head value
     */
    const Val_type& head() const {return data_ -> head();};
    
    /*! @brief return code for head value
     */
    const Key_type& head_s() const {return data_ -> head_s();};
    
    /*! @brief return underlying hash code for head value
     */
    const Val_type& arg(const size_t&) const
    {
        KJB_THROW_2(
            kjb::Cant_happen,
            "Attempted to access the argument of a Terminal_elaboration"
            );
    };
    
    /*! @brief return underlying hash code for head value
     */
    const Key_type& arg_s(const size_t&) const
    {
        KJB_THROW_2(
            kjb::Cant_happen,
            "Attempted to access the argument of a Terminal_elaboration"
            );
    };

    /*! @brief return underlying hash code for head value
     */
    const size_t& head_code() const {return data_ -> head_code();}
    
    /*! @brief return underlying hash code for args
     */
    const size_t& args_code() const {return data_ -> args_code();}

    /*! @brief return type code
     */
    size_t type_code() const {return Traits::type_code;}

    /*! @brief return pointer to underlying data node
     */
    const Data_base_ptr data() const {return data_;}

    /*! @brief return binary filter array indicating allowable steps
     */
    const Step_code::Weights& step_filter() const
    {
	return Traits::step_filter();
    }

    /*! @brief return pointer to child in a particular position
     */
    Base_ptr child(const size_t&) const {return Base_ptr();}

    /*! @brief return pointer to child in a particular position
     */
    Self_ptrs_read children() const
    {
        return Self_ptrs_read();
    }
    
    /*! @brief take a step down the semantic tree of type step_code
     */
    Step_result take_a_step(const Step_code_t& step_code) const;

    /*------------------------------------------------------------
     * MANIPULATORS
     *------------------------------------------------------------*/

    /*! @brief put a child in a particular position
     */
    void add_child(const size_t&, const Self_ptr) {}

    /*! @brief add child and have existing child "adopt" the old one
     */
    void insert_child(const size_t&, const Self_ptr, const size_t&) {}

    /*------------------------------------------------------------
     * QUERIES
     *------------------------------------------------------------*/
    
    /*! @brief is this a terminal node?
     */
    bool is_terminal() const {return true;}

    /*------------------------------------------------------------
     * DISPLAY FUNCTIONS
     *------------------------------------------------------------*/
    
    /*! @brief print contents at this node
     */
    void print(std::ostream& os) const {os << *data_;}
    
    /*! @brief recursively print elaboration tree contents
     */
    void print_subtree(std::ostream& os, int indent_level) const;
private:
    Data_ptr data_;
};

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 
    
template<class T, class Traits>
class Nonterminal_elaboration : public Semantic_elaboration
{
public:
    /*------------------------------------------------------------
     * TYPEDEFS
     *------------------------------------------------------------*/
    
    typedef Nonterminal_elaboration<T, Traits> Self_type;
    typedef Semantic_data<T> Data_type;
    typedef Traits Traits_t;
    typedef Semantic_elaboration::Self_ptr Base_ptr;
    typedef Semantic_elaboration::Self_ptr_const Base_ptr_const;
    typedef boost::shared_ptr<Semantic_data<T> > Data_ptr;
    typedef typename Data_type::Arg_list Arg_list;
    typedef typename Data_type::Arg_list_s Arg_list_s;
    typedef boost::array<Base_ptr, Traits::n_args> Child_array;
public:
    /*------------------------------------------------------------
     * CONSTRUCTORS/DESTRUCTOR
     *------------------------------------------------------------*/
    
    /*! @brief construct an elaboration from numeric codes for head and args
     *  @param head integral code for the value of the head (relation or object)
     *  @param args array of integral codes for the arguments of the predicate
     *  @param referent_code integral code of a referent corresponding to the
     *         head (if it exists).
     */
    Nonterminal_elaboration
    (
        const Val_type&      head,
        const Arg_list&      args,
        const Referent_list& referent_list =
            Referent_list(Traits::n_args + 1, NULL_REFERENT)
    );

    /*! @brief construct an elaboration from strings
     *  @param head label (string) for the value of the head
     *  @param args array of labels for the arguments of the predicate
     *  @param referent_code integral code of a referent corresponding to the
     *         head (if it exists).
     */
    Nonterminal_elaboration
    (
        const Key_type&       head,
        const Arg_list_s&     args,
        const Referent_list&  referent_list =
            Referent_list(Traits::n_args + 1, NULL_REFERENT)
    );

    /*------------------------------------------------------------
     * ACCESSORS
     *------------------------------------------------------------*/
    
    /*! @brief return code for head value
     */
    const Val_type& head() const {return data_ -> head();};
    
    /*! @brief return code for head value
     */
    const Key_type& head_s() const
    {
        return data_->head_s();
    };
    
    /*! @brief return underlying hash code for head value
     */
    const Val_type& arg(const size_t& pos) const
    {
        IFTD(pos < Traits::n_args, kjb::Index_out_of_bounds,
             "Attempted to access argument %d in a Nonterminal_elaboration"
             " with only %d arguments", (pos)(Traits::n_args));
        return args()[pos];
    };
    
    /*! @brief return code for head value
     */
    const Key_type& arg_s(const size_t& pos) const
    {
        IFTD(pos < Traits::n_args, kjb::Index_out_of_bounds,
             "Attempted to access argument %d in a Nonterminal_elaboration"
             " with only %d arguments", (pos)(Traits::n_args));
        return Data_type::arg_map_list()[pos]->decode(arg(pos));
    };
    
    /*! @brief return underlying hash code for head value
     */
    const size_t& head_code() const {return data_ -> head_code();}
    
    /*! @brief return underlying hash code for args
     */
    const size_t& args_code() const {return data_ -> args_code();}

    /*! @brief return type code
     */
    size_t type_code() const {return Traits::type_code;}
    
    /*! @brief return pointer to data node
     */
    const Data_base_ptr data() const {return data_;}
    
    /*! @brief return args (pass-through to data_)
     */
    const Arg_list& args() const {return data_ -> args();}
    
    /*! @brief get pointer to child elaboration in position "index"
     */
    Base_ptr child(const size_t& index) const;

    /*! @brief access the container of children by const reference
     */
    Self_ptrs_read children() const
    {
        return Self_ptrs_read(children_.begin(), children_.end());
    }

    const Referent_code& head_referent() const {return referent_list_[0];}
    
    const Referent_code& arg_referent(const size_t& pos) const
    {
        IFTD(pos < Traits::n_args, kjb::Illegal_argument,
             "Attempted to access referent corresponding to nonexistent "
             "argument %d in a Nonterminal_elaboration with only "
             "%d arguments", (pos)(Traits::n_args));
        return referent_list_[pos + 1];
    }
    
    /*! @brief return binary filter array indicating allowable steps
     */
    const Step_code::Weights& step_filter() const
    {
	return Traits::step_filter();
    }
    
    /*! @brief takes a step down the semantic tree, returning data
     *  @param step_code an enumeration code indicating the type of step to take
     *  @return a tuple containing (1) a pointer to the new node,
     *      (2) a hash code for the head value of the result
     *      (3) a hash code for the args of the result
     */
    Step_result take_a_step(const Step_code_t& step_code) const;
    
    /*------------------------------------------------------------
     * MANIPULATORS
     *------------------------------------------------------------*/
    
    /*! @brief set new argument values for underyling data
     *  @param args a boost::tuple of numeric codes specifying the args
     */
    void set_data_args(const Arg_list& args);

    /*! @brief add a new elaboration of argument "index", discarding any existing
     */
    void add_child(const size_t& pos, const Base_ptr child);

    /*! @brief add child and have it adopt existing one
     */
    void insert_child(
	const size_t&   pos,
	const Base_ptr new_child,
	const size_t&   pos_for_old
	);

    /*------------------------------------------------------------
     * QUERIES
     *------------------------------------------------------------*/
    
    /*! @brief query whether this elaboration is a terminal or not
     *  @return @c true if a terminal, @c false otherwise
     */
    bool is_terminal() const {return false;}

    /*------------------------------------------------------------
     * DISPLAY FUNCTIONS
     *------------------------------------------------------------*/
    
    /*! @brief print contents at this node to ostream @a os
     */
    void print(std::ostream& os) const
    {
        os << *data_;
        if(head_referent() != NULL_REFERENT) os << " " << head_referent();
        os << "(";
        for(Referent_list::const_iterator it = ++(referent_list_.begin());
            it != referent_list_.end(); ++it)
        {
            if(*it != NULL_REFERENT)
            {
                os << *it << ",";
            }
        }
        os << ")";
        os << std::endl;
    }
    
    /*! @brief recursively print the full elaboration tree
     */
    void print_subtree(std::ostream& os, int indent_level = 0) const;
    
private:
    Data_ptr data_;
    Base_ptr head_child_;
    Child_array children_;
    Referent_list referent_list_;
};

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*------------------------------------------------------------
 * MEMBER FUNCTION IMPLEMENTATION (Semantic_data)
 *------------------------------------------------------------*/    
    
template<class T, class Traits>
Semantic_data<T, Traits>::Semantic_data(
    const Val_type& head,
    const Arg_list& args
    ) : head_(head),
	args_(args),
	hash_pair_(boost::make_tuple(0UL, 0UL))
{
    initialize_codes();
    // Map hash codes to readable strings
    std::string hstring = head_map().decode(head);
    Arg_list_s args_s = multi_map_decode<Arg_list_s>(arg_map_list(), args);
    global_head_map().insert(Map_entry(hash_pair_.get<0>(), hstring));
    std::string argstring =
	"(" + boost::algorithm::join(args_s, ",") + ")";
    global_arg_map().insert(Map_entry(hash_pair_.get<1>(), argstring));
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
Semantic_data<T, Traits>::Semantic_data(
    const Key_type& head,
    const Arg_list_s& args
    ) : Semantic_data_base(),
	head_(head_map().encode(head)),
	args_(multi_map_encode<Arg_list>(args, arg_map_list())),
	hash_pair_(boost::make_tuple(0UL, 0UL))
{
    initialize_codes();
    global_head_map().insert(Map_entry(hash_pair_.get<0>(), head));
    std::string argstring =
	"(" + boost::algorithm::join(args, ",") + ")";
    global_arg_map().insert(Map_entry(hash_pair_.get<1>(), argstring));
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
Semantic_data<T, Traits>::Semantic_data(const Val_type& head)
: head_(head), args_(), hash_pair_(boost::make_tuple(0UL, 0UL))
{
    initialize_codes();
    std::string hstring = head_map().decode(head);
    global_head_map().insert(Map_entry(hash_pair_.get<0>(), hstring));
    global_arg_map().insert(Map_entry(hash_pair_.get<1>(), ""));
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
Semantic_data<T, Traits>::Semantic_data(const Key_type& head)
: head_(head_map().encode(head)), args_(), hash_pair_(boost::make_tuple(0UL, 0UL))
{
    initialize_codes();
    global_head_map().insert(Map_entry(hash_pair_.get<0>(), head));
    global_arg_map().insert(Map_entry(hash_pair_.get<1>(), ""));
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
void Semantic_data<T, Traits>::initialize_codes()
{
    boost::hash_combine<int>(hash_pair_.get<0>(), head_);
    boost::hash_combine<int>(hash_pair_.get<0>(), Traits::type_code);
    boost::hash_range(hash_pair_.get<1>(), args_.begin(), args_.end());
    boost::hash_combine<int>(hash_pair_.get<1>(), Traits::type_code);
}

/*------------------------------------------------------------
 * MEMBER FUNCTION IMPLEMENTATION (Terminal_elaboration)
 *------------------------------------------------------------*/    

template<class T, class Traits>
Terminal_elaboration<T,Traits>::Terminal_elaboration()
    : Semantic_elaboration(), data_(boost::make_shared<Semantic_data<T> >(0))
{
}
    
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
Terminal_elaboration<T,Traits>::Terminal_elaboration(const Val_type& head)
: Semantic_elaboration(),
  data_(boost::make_shared<Semantic_data<T> >(head))
{
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
Terminal_elaboration<T,Traits>::Terminal_elaboration(const Key_type& head)
: Semantic_elaboration(),
  data_(boost::make_shared<Semantic_data<T> >(head))
{
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
Semantic_elaboration::Step_result
Terminal_elaboration<T,Traits>::take_a_step(const Step_code_t& step_code) const
{
    switch(step_code)
    {
    case Step_code::NULL_STEP:
    case Step_code::IDENTITY:
    case Step_code::HEAD:
    case Step_code::LEFT_ARG:
    case Step_code::RIGHT_ARG:
    case Step_code::ATT0:
    case Step_code::ATT1:
    case Step_code::NUM_STEPS:
    default:
	return boost::make_tuple(
	    null_semantic_terminal(),
	    boost::make_tuple(0UL, 0UL)
	    );
    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
void Terminal_elaboration<T,Traits>::print_subtree(
    std::ostream& os,
    int           indent_level
    ) const
{
    std::string lead(4*indent_level, ' ');
    os << lead << *data_ << std::endl;
};

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*------------------------------------------------------------
 * MEMBER FUNCTION IMPLEMENTATION (Nonterminal_elaboration)
 *------------------------------------------------------------*/

template<class T, class Traits>
Nonterminal_elaboration<T, Traits>::Nonterminal_elaboration(
    const Val_type&      head,
    const Arg_list&      args,
    const Referent_list& referent_list
    ) : Semantic_elaboration(),
	data_(boost::make_shared<Semantic_data<T> >(head, args)),
	head_child_(
	    boost::make_shared<
		Terminal_elaboration<typename Traits::Head_type>
		>(head)
            ),
        children_(),
        referent_list_(referent_list)
{
    for(typename Child_array::iterator it = children_.begin();
        it != children_.end(); ++it)
    {
        *it = Base_ptr();
    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
Nonterminal_elaboration<T, Traits>::Nonterminal_elaboration(
    const Key_type&      head,
    const Arg_list_s&    args,
    const Referent_list& referent_list
    ) : Semantic_elaboration(),
	data_(boost::make_shared<Semantic_data<T> >(head, args)),
	head_child_(
	    boost::make_shared<
		Terminal_elaboration<typename Traits::Head_type>
		>(Semantic_data<T>::head_map().encode(head))
	    ),
        children_(),
        referent_list_(referent_list)
{
    for(typename Child_array::iterator it = children_.begin();
        it != children_.end(); ++it)
    {
        *it = Base_ptr();
    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
void Nonterminal_elaboration<T, Traits>::set_data_args(const Arg_list& args)
{
    Val_type h = data_ -> head();
    data_ = boost::make_shared<Semantic_data<T> >(h, args);
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
typename Nonterminal_elaboration<T,Traits>::Base_ptr
Nonterminal_elaboration<T, Traits>::child(const size_t& index) const
{
    IFTD(index < Traits::n_args, kjb::Illegal_argument,
        "Attempted to access child in nonexistent slot %d in"
         " Nonterminal_elaboration with only %d slots",
         (index)(Traits::n_args)); 
    return children_[index];
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
void Nonterminal_elaboration<T, Traits>::add_child(
    const size_t&     pos,
    const Base_ptr    child
    )
{
    IFTD(pos < Traits::n_args, kjb::Illegal_argument,
         "Attempted to add child to nonexistent slot %d in Nonterminal_elaboration"
         "which has only %d slots", (pos)(Traits::n_args));
    children_[pos] = child;
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
void Nonterminal_elaboration<T, Traits>::insert_child(
    const size_t&   pos,
    const Base_ptr  new_child,
    const size_t&   pos_for_old
    )
{
    IFTD(pos < Traits::n_args, kjb::Illegal_argument,
         "Attempted to insert child to nonexistent slot %d in Nonterminal_elaboration"
         "which has only %d slots", (pos)(Traits::n_args));
    Base_ptr old_child = children_[pos];
    add_child(pos, new_child);
    children_[pos] -> add_child(pos_for_old, old_child);
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
Semantic_elaboration::Step_result
Nonterminal_elaboration<T, Traits>::take_a_step(
    const Step_code_t& step_code
    ) const
{
    Base_ptr p0 = Traits::n_args > 0 ? children_[0] : Base_ptr();
    Base_ptr p1 = Traits::n_args > 1 ? children_[1] : Base_ptr();
    switch(step_code)
    {
    case Step_code::HEAD:
	return head_child_ == NULL ?
	    Step_result(
		null_semantic_terminal(),
		boost::make_tuple(0UL, 0UL)) :
	    Step_result(
		head_child_,
		boost::make_tuple(
		    head_child_ -> head_code(),
		    head_child_ -> args_code()
		    )
		);
    case Step_code::LEFT_ARG:
    case Step_code::ATT0:
	return p0 == NULL ?
	    Step_result(
		null_semantic_terminal(),
		boost::make_tuple(0UL, 0UL)) :
	    Step_result(
		p0,
		boost::make_tuple(p0 -> head_code(), p0 -> args_code())
		);
    case Step_code::RIGHT_ARG:
    case Step_code::ATT1:
	return p1 == NULL ?
	    Step_result(
		null_semantic_terminal(),
		boost::make_tuple(0UL, 0UL)) :
	    Step_result(
		p1,
		boost::make_tuple(p1 -> head_code(), p1 -> args_code())
		);
    case Step_code::IDENTITY:
    case Step_code::NULL_STEP:
    case Step_code::NUM_STEPS:
    default:
	return Step_result(
	    null_semantic_terminal(),
	    boost::make_tuple(0UL, 0UL)
	    );
    }
}

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
void Nonterminal_elaboration<T, Traits>::print_subtree(
    std::ostream& os,
    int           indent_level
    ) const
{
    std::string lead(4*indent_level, ' ');
    os << lead;
    print(os);
    head_child_ -> print_subtree(os, indent_level + 1);
    for(typename Child_array::const_iterator it = children_.begin();
	it != children_.end(); it++)
    {
	if(*it != NULL) (*it) -> print_subtree(os, indent_level + 1);
    }
} 

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

/*------------------------------------------------------------
 * FREE FUNCTIONS
 *------------------------------------------------------------*/

void initialize_global_semantic_maps();

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

std::ostream& operator<<(std::ostream& os, Semantic_elaboration::Self_ptr elab);

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

std::ostream& operator<<(std::ostream& os, Semantic_data_base::Hash_pair);
    
/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

template<class T, class Traits>
std::ostream& operator<<(
    std::ostream&           os,
    Semantic_data<T,Traits> d
    )
{
    typedef Semantic_data<T, Traits> D;
    typename D::Arg_list_s keys = multi_map_decode<typename D::Arg_list_s>(
	D::arg_map_list(), d.args_);
    os << D::head_map().decode(d.head_);
    if(Traits::n_args != 0)
    {
	os << "(";
	for(typename D::Arg_list_s::const_iterator it = keys.begin();
	    it != keys.end(); it++)
	{
	    os << *it << ",";
	}
	os << ")";
    }
    // os << " -- " << d.hash_pair_;
    return os;
}


};
    
#endif
