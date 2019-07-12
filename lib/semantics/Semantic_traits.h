#ifndef SEMANTIC_TRAITS_H_
#define SEMANTIC_TRAITS_H_

/*!
 * @file Semantic_traits.h
 *
 * @author Colin Dawson 
 * $Id: Semantic_traits.h 17352 2014-08-21 21:19:41Z cdawson $ 
 */

#include "semantics/SemanticIO.h"
#include "semantics/Semantic_elaboration.h"
#include <map>
#include <vector>

namespace semantics
{

/*! @brief read in mappings from files, default config path
 */
void initialize_semantic_maps();

/*! @brief read in mappings from files, config file path specified
 */
void initialize_semantic_maps(const std::string config_file_path);

/*! @brief read in mappings from files, map with specified data type config paths
 */
void initialize_semantic_maps(
    const std::map<std::string, std::string>& config_file_path_map
    );

class Null_primitive;
class Unary_relation_primitive;
class Binary_relation_primitive;
class Category_primitive;
class Color_primitive;
class Size_primitive;
class Semantic_object;
class Unary_predicate;
class Binary_predicate;

template<class T>
class Semantic_traits
{};

    
template<>
struct Semantic_traits<Null_primitive>    
{
    enum
    {
	n_args = 0,
        type_code = 0
    };
    static const Step_code::Weights& step_filter()
    {
	static Step_code::Weights::Data_type f = {{0.1,1,1,1,1,1,1}};
	static Step_code::Weights filter(f);
	return filter.normalize();
    }
};

    
template<>
struct Semantic_traits<Unary_relation_primitive>    
{
    enum
    {
	n_args = 0,
        type_code = 1,
        TARGET = 0
    };
    static const Step_code::Weights& step_filter()
    {
	static Step_code::Weights::Data_type f = {{0.1,1,0,0,0,0,0}};
	static Step_code::Weights filter(f);
	return filter.normalize();
    }
};


template<>
struct Semantic_traits<Binary_relation_primitive>
{
    enum
    {
	n_args = 0,
        type_code = 2
    };
    static const Step_code::Weights& step_filter()
    {
	static Step_code::Weights::Data_type f = {{0.1,1,0,0,0,0,0}};
	static Step_code::Weights filter(f);
	return filter.normalize();
    }
};


template<>
struct Semantic_traits<Semantic_object>
{
    typedef Category_primitive Head_type;
    enum
    {
	n_args = 2,
        type_code = 6,
        COLOR = 0,
        SIZE = 1
    };
    static const Step_code::Weights& step_filter()
    {
	static Step_code::Weights::Data_type f = {{0.1,1,1,0,0,1,1}};
	static Step_code::Weights filter(f);
	return filter.normalize();
    }
};    


template<>
struct Semantic_traits<Category_primitive>
{
    enum
    {
	n_args = 0,
        type_code = 3
    };
    static const Step_code::Weights& step_filter()
    {
	static Step_code::Weights::Data_type f = {{0.1,1,0,0,0,0,0}};
	static Step_code::Weights filter(f);
	return filter.normalize();
    }
};


template<>
struct Semantic_traits<Color_primitive>
{
    enum
    {
	n_args = 0,
        type_code = 4,
        attribute = 0
    };
    static const Step_code::Weights& step_filter()
    {
	static Step_code::Weights::Data_type f = {{0.1,1,0,0,0,0,0}};
	static Step_code::Weights filter(f);
	return filter.normalize();
    }
};


template<>
struct Semantic_traits<Size_primitive>
{
    enum
    {
	n_args = 0,
        type_code = 5,
        attribute = 1
    };
    static const Step_code::Weights& step_filter()
    {
	static Step_code::Weights::Data_type f = {{0.1,1,0,0,0,0,0}};
	static Step_code::Weights filter(f);
	return filter.normalize();
    }
};




template<>
struct Semantic_traits<Unary_predicate>
{
    typedef Unary_relation_primitive Head_type;
    enum
    {
	n_args = 1,
        type_code = 7,
        TARGET = 0
    };
    static const Step_code::Weights& step_filter()
    {
	static Step_code::Weights::Data_type f = {{0.1,1,1,1,0,0,0}};
	static Step_code::Weights filter(f);
	return filter.normalize();
    }
};

    
template<>
struct Semantic_traits<Binary_predicate>
{
    typedef Binary_relation_primitive Head_type;
    enum
    {
	n_args = 2,
        type_code = 8,
        TARGET = 0,
        BASE = 1
    };
    static const Step_code::Weights& step_filter()
    {
	static Step_code::Weights::Data_type f = {{0.1,1,1,1,1,0,0}};
	static Step_code::Weights filter(f);
	return filter.normalize();
    }
};

typedef Semantic_data<Null_primitive> Null_data;
typedef Semantic_data<Category_primitive> Category_data;
typedef Semantic_data<Unary_relation_primitive> Unary_relation_data;
typedef Semantic_data<Binary_relation_primitive> Binary_relation_data;
typedef Semantic_data<Color_primitive> Color_data;
typedef Semantic_data<Size_primitive> Size_data;
typedef Semantic_data<Semantic_object> Object_data;
typedef Semantic_data<Unary_predicate> Unary_data;
typedef Semantic_data<Binary_predicate> Binary_data;
typedef Terminal_elaboration<Null_primitive> Null_semantic_terminal;
typedef Nonterminal_elaboration<Semantic_object> Object_elaboration;
typedef Nonterminal_elaboration<Unary_predicate> Unary_elaboration;
typedef Nonterminal_elaboration<Binary_predicate> Binary_elaboration;

/*! @brief make a null semantic terminal
 */
const boost::shared_ptr<Null_semantic_terminal>&
null_semantic_terminal();
  
/*! @brief create an object semantic node of category category, no attributes
 */
boost::shared_ptr<Object_elaboration>
make_plain_semantic_object
(
    const Object_data::Val_type& category,
    const Semantic_elaboration::Referent_code& referent_code =
        Semantic_elaboration::NULL_REFERENT
);

const std::vector<Step_code::Weights>& step_code_priors();
    
};
    
#endif







