/*!
 * @file Semantic_traits.cpp
 *
 * @author Colin Dawson 
 * $Id: Semantic_traits.cpp 17350 2014-08-21 20:30:43Z cdawson $ 
 */

#include "semantics/Semantic_traits.h"
#include "semantics/Semantic_elaboration.h"
#include <boost/make_shared.hpp>

namespace semantics
{
    // const size_t Semantic_traits<Null_primitive>::type_code;
    // const size_t Semantic_traits<Category_primitive>::type_code;
    // const size_t Semantic_traits<Unary_relation_primitive>::type_code;
    // const size_t Semantic_traits<Binary_relation_primitive>::type_code;
    // const size_t Semantic_traits<Color_primitive>::type_code;
    // const size_t Semantic_traits<Size_primitive>::type_code;
    // const size_t Semantic_traits<Semantic_object>::type_code;
    // const size_t Semantic_traits<Unary_predicate>::type_code;
    // const size_t Semantic_traits<Binary_predicate>::type_code;
    
    boost::shared_ptr<Object_elaboration>
    make_plain_semantic_object(
        const Object_data::Val_type&               category,
        const Semantic_elaboration::Referent_code& referent_code)
    {
        typedef Semantic_elaboration::Referent_code Ref_code;
        typedef Semantic_elaboration::Referent_list Ref_list;
        static const Ref_code NULL_REF = Semantic_elaboration::NULL_REFERENT;
	Object_data::Arg_list args = {{0,0}};
        boost::array<Ref_code, Object_elaboration::Traits_t::n_args + 1>
            referent_args = {{referent_code, NULL_REF, NULL_REF}};
	boost::shared_ptr<Object_elaboration> result =
	    boost::make_shared<Object_elaboration>(
                category, args,
                Semantic_elaboration::Referent_list(
                    referent_args.begin(), referent_args.end()));
	result->add_child(0, null_semantic_terminal());
	result->add_child(1, null_semantic_terminal());
	return result;
    }

    const boost::shared_ptr<Null_semantic_terminal>&
    null_semantic_terminal()
    {
	static boost::shared_ptr<Null_semantic_terminal> ne =
	    boost::make_shared<Null_semantic_terminal>();
	return ne;
    }

void initialize_semantic_maps()
{
    initialize_semantic_maps("config/");
}

void initialize_semantic_maps(std::string config_file_path)
{
    const std::string color_config_file(config_file_path + "colors.config");
    const std::string size_config_file(config_file_path + "sizes.config");
    const std::string category_config_file(config_file_path + "objects.config");
    const std::string unary_config_file(config_file_path + "unary_relations.config");
    const std::string binary_config_file(config_file_path + "binary_relations.config");
    const Object_data::Arg_map_list obj_arg_map_list =
	{{&Color_data::head_map(), &Size_data::head_map()}};
    const Unary_data::Arg_map_list unary_arg_map_list =
	{{&Object_data::head_map()}};

    const Binary_data::Arg_map_list binary_arg_map_list =
	{{&Object_data::head_map(), &Object_data::head_map()}};

    std::cerr << "Initializing global semantic maps." << std::endl;
    initialize_global_semantic_maps();
    
    std::cerr << "Initializing individual semantic maps...";
    
    Object_data::head_map().initialize_from_file(category_config_file);
    Color_data::head_map().initialize_from_file(color_config_file);
    Size_data::head_map().initialize_from_file(size_config_file);
    Unary_data::head_map().initialize_from_file(unary_config_file);
    Binary_data::head_map().initialize_from_file(binary_config_file);
    Category_data::head_map().initialize_from_file(category_config_file);
    Unary_relation_data::head_map().initialize_from_file(
	unary_config_file);
    Binary_relation_data::head_map().initialize_from_file(
	binary_config_file);

    Object_data::arg_map_list() = obj_arg_map_list;
    Unary_data::arg_map_list() = unary_arg_map_list;
    Binary_data::arg_map_list() = binary_arg_map_list;

    std::cerr << "done." << std::endl;
}

void initialize_semantic_maps(
    const std::map<std::string,std::string>& config_file_path_map
    )
{

    std::cerr << "Initializing global semantic maps." << std::endl;
    initialize_global_semantic_maps();

    std::cerr << "Initializing individual semantic maps...";
    
    std::map<std::string,std::string>::const_iterator it;

    it = config_file_path_map.find("colors");
    if (it != config_file_path_map.end())
    {
        const std::string color_config_file(it->second);
        Color_data::head_map().initialize_from_file(color_config_file);
    } 

    it = config_file_path_map.find("sizes");
    if (it != config_file_path_map.end())
    {
        const std::string size_config_file(it->second);
        Size_data::head_map().initialize_from_file(size_config_file);
    }
    
    it = config_file_path_map.find("objects");
    if (it != config_file_path_map.end())
    {
        const std::string category_config_file(it->second);
        Object_data::head_map().initialize_from_file(category_config_file);
        Category_data::head_map().initialize_from_file(category_config_file);
    }

    // TODO CTM 20140610: not sure if this should be optional...
    const std::string unary_config_file("config/unary_relations.config");
    Unary_data::head_map().initialize_from_file(unary_config_file);
    Unary_relation_data::head_map().initialize_from_file(
        unary_config_file);

    it = config_file_path_map.find("binary_relations");
    if (it != config_file_path_map.end())
    {
        const std::string binary_config_file(it->second);
        Binary_data::head_map().initialize_from_file(binary_config_file);
        Binary_relation_data::head_map().initialize_from_file(
            binary_config_file);
    }

    const Object_data::Arg_map_list obj_arg_map_list =
    {{&Color_data::head_map(), &Size_data::head_map()}};
    
    const Unary_data::Arg_map_list unary_arg_map_list =
    {{&Object_data::head_map()}};

    const Binary_data::Arg_map_list binary_arg_map_list =
    {{&Object_data::head_map(), &Object_data::head_map()}};

    Object_data::arg_map_list() = obj_arg_map_list;
    Unary_data::arg_map_list() = unary_arg_map_list;
    Binary_data::arg_map_list() = binary_arg_map_list;

    std::cerr << "done." << std::endl;
}

    std::vector<Step_code::Weights> step_code_priors_by_elaboration_type()
    {
        std::vector<Step_code::Weights> out;
        out.push_back(Semantic_traits<Null_primitive>::step_filter());
        out.push_back(Semantic_traits<Unary_relation_primitive>::step_filter());
        out.push_back(Semantic_traits<Binary_relation_primitive>::step_filter());
        out.push_back(Semantic_traits<Category_primitive>::step_filter());
        out.push_back(Semantic_traits<Color_primitive>::step_filter());
        out.push_back(Semantic_traits<Size_primitive>::step_filter());
        out.push_back(Semantic_traits<Semantic_object>::step_filter());
        out.push_back(Semantic_traits<Unary_predicate>::step_filter());
        out.push_back(Semantic_traits<Binary_predicate>::step_filter());
        return out;
    }

    const std::vector<Step_code::Weights>& step_code_priors()
    {
	typedef std::vector<Step_code::Weights> Result;
	static Result result = step_code_priors_by_elaboration_type();
	return result;
    }

};

