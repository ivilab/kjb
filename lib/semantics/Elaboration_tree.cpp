/*!
 * @file Elaboration_tree.cpp
 *
 * @author Colin Dawson 
 * $Id: Elaboration_tree.cpp 21596 2017-07-30 23:33:36Z kobus $ 
 */

#include "l/l_sys_debug.h"
#include "semantics/Elaboration_tree.h"
#include "semantics/Semantic_traits.h"
#include "l_cpp/l_exception.h"

namespace semantics
{


/*------------------------------------------------------------
 * MEMBER FUNCTION DEFINITIONS
 *------------------------------------------------------------*/
    
    Elaboration_tree::Elaboration_tree(
	const Semantic_data_base::Key_type&        subject,
        const Semantic_elaboration::Referent_code& subject_referent
	) : root_(null_semantic_terminal()),
	    live_objects_(),
	    live_object_parents_(),
            curr_object_(),
            curr_parent_(),
            curr_object_role_(TARGET)
    {
	Unary_data::Arg_list_s s_args = {{subject}};
        boost::array<Semantic_elaboration::Referent_code, 2> referent_codes =
            {{Semantic_elaboration::NULL_REFERENT, subject_referent}};
	root_ = boost::make_shared<Unary_elaboration>(
            "IS_SUBJ", s_args,
            Semantic_elaboration::Referent_list(
                referent_codes.begin(), referent_codes.end()
                ));
	// Object_data::Arg_list null_args = {{0, 0}};
	Obj_ptr subj_node =
	    make_plain_semantic_object(
		Object_data::head_map().encode(subject, false),
                subject_referent
		);
	root_ -> add_child(curr_object_role_, subj_node);
        curr_parent_ = root_;
        curr_object_ =
            boost::dynamic_pointer_cast<Object_elaboration>(root_->child(TARGET));
        IFT(curr_object_ != NULL, kjb::Cant_happen,
            "Somehow an Elaboration_tree is being instantiated with "
            "a non Object_elaboration in the top TARGET position.");
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    Elaboration_tree::Elab_ptr_const Elaboration_tree::elaborate_color(const std::string& color)
    {
	return elaborate_attribute<
	    Color_primitive, Semantic_traits<Color_primitive>
	    >(color);	
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    Elaboration_tree::Elab_ptr_const Elaboration_tree::elaborate_size(const std::string& size)
    {
	return elaborate_attribute<
	    Size_primitive, Semantic_traits<Size_primitive>
	    >(size);
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    Elaboration_tree::Elab_ptr_const Elaboration_tree::elaborate_with_relation(
	const std::string&                         relation,  
	const std::string&                         base_object_class,
        const Semantic_elaboration::Referent_code& base_referent_code
        )
    {
	const int base_object_code =
	    Object_data::head_map().encode(base_object_class);
	Elab_ptr new_base_obj = 
            make_plain_semantic_object(base_object_code, base_referent_code);
        ASSERT(curr_object_ != NULL);
	Binary_data::Arg_list new_args =
	    {{curr_object_ -> data() -> head(), (unsigned long) base_object_code}};
        boost::array<Semantic_elaboration::Referent_code,
                     Binary_elaboration::Traits_t::n_args + 1> new_referent_codes =
            {{Semantic_elaboration::NULL_REFERENT,
              curr_object_ -> head_referent(),
              base_referent_code}};
	Elab_ptr new_binary =
	    boost::make_shared<Binary_elaboration>(
		Binary_data::head_map().encode(relation),
		new_args,
                Semantic_elaboration::Referent_list(
                    new_referent_codes.begin(),
                    new_referent_codes.end()));
        ASSERT(new_binary != NULL);
	new_binary -> add_child(BASE, new_base_obj);
        ASSERT(new_binary != NULL);
        ASSERT(curr_parent_ != NULL);
	curr_parent_ ->
	    insert_child(
                curr_object_role_,
		new_binary,
		TARGET);
        curr_parent_ = new_binary;
	add_object_to_queue(new_binary, BASE);
        curr_object_role_ = TARGET;
        return new_binary;
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    Elaboration_tree::Step_result Elaboration_tree::take_a_step(
	const Elab_ptr_const     curr,
	const Step_code_t&       step_code
	)
    {
	ASSERT(curr != NULL);
	switch(step_code)
	{
	case Step_code::NULL_STEP:
	    return boost::make_tuple(
		null_semantic_terminal(),
		boost::make_tuple(0, 0)
		);
	case Step_code::IDENTITY:
	    return boost::make_tuple(
		curr,
		boost::make_tuple(curr->head_code(), curr->args_code())
		);
	case Step_code::HEAD:
	case Step_code::LEFT_ARG:
	case Step_code::RIGHT_ARG:
	case Step_code::ATT0:
	case Step_code::ATT1:
	case Step_code::NUM_STEPS:
	default:
	    return curr -> take_a_step(step_code);
	}
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Elaboration_tree::add_object_to_queue(
	const Elab_ptr     parent,
	const size_t&      which_child
	)
    {
	Elab_ptr the_child = parent -> child(which_child);
	Obj_ptr the_object =
	    boost::dynamic_pointer_cast<Object_elaboration>(the_child);
	IFT(the_object != NULL, kjb::Illegal_argument,
            "Elaboration_tree::add_object_to_queue(Elab_ptr parent, size_t which_child)"
            " received parent argument whose child is not an Object_elaboration.");
	live_object_parents_.push_front(parent);
	live_objects_.push_front(the_object);
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    void Elaboration_tree::pop_object_from_queue()
    {
        ASSERT(!live_objects_.empty());
        ASSERT(!live_object_parents_.empty());
        // if(live_objects_.empty())
        // {
        //     curr_parent_ = Elab_ptr();
        //     curr_object_ = Obj_ptr();
        //     return;
        // }
        curr_parent_ = live_object_parents_.front();
        curr_object_ = live_objects_.front();
        curr_object_role_ = BASE;
        live_object_parents_.pop_front();
        live_objects_.pop_front();
    }

/* /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ */ 

    template<class T, class Traits>
    Elaboration_tree::Elab_ptr_const Elaboration_tree::elaborate_attribute(
	const Semantic_data_base::Key_type& new_val
	)
    {
	Object_data::Arg_list new_args = curr_object_ -> args();
	Elab_ptr new_attribute =
	    boost::make_shared<Terminal_elaboration<T> >(new_val);
	if(new_args[Traits::attribute] != 0)
	{
	    std::cerr << "Error: Elaborating attribute which is already "
		"specified. Nothing done." << std::endl;
	    return Elab_ptr_const();
	} else {
	    new_args[Traits::attribute]
		= (Object_data::arg_map_list())[Traits::attribute]
		-> encode(new_val);
	    curr_object_ -> set_data_args(new_args);
	    curr_object_ -> add_child(Traits::attribute, new_attribute);
	}
        return curr_object_;
    }

/*------------------------------------------------------------
 * STATIC INITIALIZATION
 *------------------------------------------------------------*/
    
    const size_t Elaboration_tree::TARGET = Binary_elaboration::Traits_t::TARGET;
    const size_t Elaboration_tree::BASE = Binary_elaboration::Traits_t::BASE;
    const size_t Elaboration_tree::COLOR = Object_elaboration::Traits_t::COLOR;
    const size_t Elaboration_tree::SIZE = Object_elaboration::Traits_t::SIZE;
};

