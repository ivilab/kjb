#ifndef ELABORATION_TREE_H_
#define ELABORATION_TREE_H_

/*!
 * @file Elaboration_tree.h
 *
 * @author Colin Dawson 
 * $Id: Elaboration_tree.h 17497 2014-09-09 00:55:07Z cdawson $ 
 */

#include "semantics/Semantic_traits.h"
#include <boost/make_shared.hpp>
#include <vector>

namespace semantics
{

    /*! @class Elaboration_tree
     *  @brief the main "Semantic tree" object
     *
     *  Represents a conceptual description underlying a caption.
     */
    class Elaboration_tree
    {
    public:
	/*------------------------------------------------------------
	 * TYPEDEFS
	 *------------------------------------------------------------*/
	typedef boost::shared_ptr<Elaboration_tree> Self_ptr;
	typedef Semantic_data_base::Val_type Val_type;
	typedef Semantic_data_base::Key_type Key_type;
	typedef Semantic_data_base::Hash_pair Hash_pair;
	typedef Semantic_elaboration::Self_ptr Elab_ptr;
	typedef Semantic_elaboration::Self_ptr_const Elab_ptr_const;
	typedef boost::shared_ptr<Object_elaboration> Obj_ptr;
	typedef boost::shared_ptr<Binary_elaboration> Binary_ptr;
	typedef boost::shared_ptr<Unary_elaboration> Unary_ptr;
	typedef std::deque<Obj_ptr> Obj_queue;
	typedef std::deque<Elab_ptr> Obj_parent_queue;
	typedef Semantic_elaboration::Step_result Step_result;
        static const size_t TARGET;
	static const size_t BASE;
        static const size_t COLOR;
	static const size_t SIZE;
    public:
	/*------------------------------------------------------------
	 * CONSTRUCTORS/DESTRUCTOR
	 *------------------------------------------------------------*/
	
	/*! @brief construct a new elaboration tree given an object as focus
	 *  @param subject string indicating the main target object of the
	 *    description.  Referenced against a lookup table; if not known,
	 *    the target object is represented as a generic 'unknown' object
	 *
	 */
	Elaboration_tree
        (
            const Key_type&                             subject,
            const Semantic_elaboration::Referent_code&  subject_referent =
                Semantic_elaboration::NULL_REFERENT
        );

	/*------------------------------------------------------------
	 * ACCESSORS
	 *------------------------------------------------------------*/
	
	/*! @brief get a shared pointer to the root elaboration node
	 */
	Elab_ptr_const root() const {return root_;}

	/*------------------------------------------------------------
	 * MANIPULATORS
	 *
	 * These are the functions that grow the tree
	 *------------------------------------------------------------*/
	
	/*! @brief add a color elaboration to the object currently in focus
	 *  @param color a string indicating the color used to describe
	 *    the current focal object.  If not contained in the color
	 *    database, it is stored as a universal 'unknown' color.
	 */
	Elab_ptr_const elaborate_color(const Key_type& color);

	/*! @brief add a size elaboration to the object currently in focus
	 *  @param size a string indicating the size used to describe
	 *    the current focal object.  If not contained in the size
	 *    database, it is stored as a universal 'unknown' size.
	 */
	Elab_ptr_const elaborate_size(const Key_type& size);

	/*! @brief elaborate description of object in focus via spatial relation
	 *  @param relation a string representing a binary relation to be used
	 *    to describe the current focal object.  If not contained in the
	 *    database of binary relations, it is stored as a generic 'unknown'
	 *  @param base_object_class a string naming the second argument, called
	 *    the 'base', of the binary relation.  If not contained in the
	 *    object database, it is stored as a generic 'unknown' object.
	 */
	Elab_ptr_const elaborate_with_relation
        (
	    const Key_type&                            relation,  
	    const Key_type&                            base_object_class,
            const Semantic_elaboration::Referent_code& base_referent_code =
                Semantic_elaboration::NULL_REFERENT
	);

	/*! @brief finish elaborating current object and move focus to next one
	 *  @remarks
	 *    Upon selection of a "null elaboration", the current object is
	 *    assumed to be fully described, and the next one in the queue
	 *    becomes active.
	 *  
	 */
	void null_elaboration()
        {
            if(!live_objects_.empty()) pop_object_from_queue();
        }

	/*------------------------------------------------------------
	 * DISPLAY FUNCTIONS
	 *------------------------------------------------------------*/

	/*! @brief print the entire elaboration tree to ostream os
	 *  @param os a standard output stream where the tree is to be printed
	 */
	void print(std::ostream& os) const {root_ -> print_subtree(os);}
	
	/*------------------------------------------------------------
	 * STATIC FUNCTIONS
	 *------------------------------------------------------------*/

	/*! @brief step down the tree using enumerated step type Step_code_t
	 *  @param curr pointer to the node being stepped from
	 *  @param step_code enumerated code defining step type
	 *  @return tuple containing
	 *         (1) pointer to new node,
	 *         (2) hash code for head value of new node
	 *         (3) hash code for args of new node
	 */
	static Step_result take_a_step
        (
	    const Elab_ptr_const     curr,
	    const Step_code_t&       step_code
	);

    private:
	Elab_ptr root_; /*!< Pointer to the elaboration at the root of the tree*/
	Obj_queue live_objects_; /*!< current object queue */
	Obj_parent_queue live_object_parents_;  /*!< parents of objects in queue*/
        Obj_ptr curr_object_;
        Elab_ptr curr_parent_;
        size_t curr_object_role_;

	/*! @brief puts a new object node on the tree and in the elab queue
	 *  @param param the parent-to-be of the new object
	 *  @param parent which_child enumerated code giving position to insert
	 */
	void add_object_to_queue
        (
	    const Elab_ptr     parent,
	    const size_t&      which_child
	);

	/*! @brief pop the object currently in focus from the queue
	 */
	void pop_object_from_queue();

	/*! @brief add an attribute elaboration to current object, value new_val
	 *  @param new_val a string giving the new attribute value to be used
	 *  @remarks
	 *    The template parameter T is the type of attribute node
	 *    to be used in the elaboration (currently either
	 *    Color_primitive or Size_primitive).  If new_val is not a string
	 *    recognized as a value of attribute type T, the generic value
	 *    of 'unknown' is used.
	 */
	template<class T, class Traits>
	Elab_ptr_const elaborate_attribute(const Semantic_data_base::Key_type& new_val);
    };

};


#endif
