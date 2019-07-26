/*!
 * @file tmp2.cpp
 *
 * @author Colin Dawson 
 * $Id: test_elaboration_tree.cpp 17394 2014-08-23 21:53:00Z cdawson $ 
 */

#include "semantics/Elaboration_tree.h"
#include "semantics/Semantic_elaboration.h"
#include "l_cpp/l_exception.h"
#include "boost/make_shared.hpp"
#include <iostream>

using namespace semantics;

void visit_enode(semantics::Elaboration_tree::Elab_ptr_const enode, const size_t& indent_level = 0)
   //(const boost::shared_ptr<const semantics::Semantic_elaboration> enode)
{
    std::string lead(4*indent_level, ' ');

    // std::cout << lead << "visit_node" << std::endl;

    int head_id = enode -> head();
    std::string head_string = enode->head_s();
    switch(enode->type_code())
    {
    case Unary_elaboration::Traits_t::type_code:
        std::cout << lead << "-----------------" << std::endl;
        std::cout << lead << "UNARY ELABORATION" << std::endl;
        std::cout << lead << "head:    " << head_id << " [" << head_string << "]" << std::endl;  
        std::cout << lead <<  "target:  "
                  << enode -> arg_referent(semantics::Elaboration_tree::TARGET) << std::endl;
        break;
    case Binary_elaboration::Traits_t::type_code:
        std::cout << lead << "-----------------" << std::endl;
        std::cout << lead << "BINARY ELABORATION" << std::endl;
        std::cout << lead << "head:    " << head_id << " [" << head_string << "]" << std::endl;  
        std::cout << lead << "target:  "
                  << enode -> arg_referent(semantics::Elaboration_tree::TARGET) << std::endl;
        std::cout << lead << "base:    "
                  << enode -> arg_referent(semantics::Elaboration_tree::BASE) << std::endl;
        break;
    case Object_elaboration::Traits_t::type_code:
        std::cout << lead << "-----------------" << std::endl;
        std::cout << lead << "OBJECT ELABORATION" << std::endl;
        std::cout << lead << "head:    " << head_id << " [" << head_string << "]" << std::endl;  
        std::cout << lead << "color:  "
                  << enode -> arg_referent(semantics::Elaboration_tree::TARGET) << std::endl;
        std::cout << lead << "size:    "
                  << enode -> arg_referent(semantics::Elaboration_tree::BASE) << std::endl;
        break;
    case Null_semantic_terminal::Traits_t::type_code:
        std::cout << lead << "-----------------" << std::endl;
        std::cout << lead << "NULL SEMANTIC ELABORATION" << std::endl;
        break;
    default:
            KJB_THROW_2(kjb::Cant_happen, "enode has an impossible type...");
    };

    // std::cout << lead << "(enode -> children()).size() = "
    //           << (enode -> children()).size() << std::endl;


    if(!(enode -> is_terminal()))
    {
        size_t counter = 0;
        Semantic_elaboration::Self_ptrs_read children = enode -> children();
        for(Semantic_elaboration::Self_ptrs_read::const_iterator it = children.begin();
            it != children.end(); ++it, ++counter)
        {
            if((*it) != NULL)
            {
                visit_enode(*it, indent_level + 1);
            }
            else
            {
                std::cout << lead << "*it in position "
                          << counter << " is NULL!" << std::endl;
            }
        }
    }
    //std::cout << std::endl;
}

boost::shared_ptr<Elaboration_tree> build_dummy_semantic_tree()
{
    Elaboration_tree::Self_ptr e = boost::make_shared<semantics::Elaboration_tree>("COUCH", 3);
    e -> elaborate_with_relation("RIGHT_OF", "COUCH", 0);
    e -> elaborate_with_relation("RIGHT_OF", "TABLE", 1);
    e -> null_elaboration();
    e -> elaborate_with_relation("BEHIND", "MIRROR", 2);
    e -> null_elaboration();
    e -> elaborate_color("BLUE");
    e -> null_elaboration();
    // e -> null_elaboration();
    return e;
}

int main(int argc, char* argv[])
{
    try
    {
        initialize_semantic_maps();
        boost::shared_ptr<Elaboration_tree> etree = build_dummy_semantic_tree();
        etree->print(std::cerr);
        // Elaboration_tree::Elab_ptr_const rooty = etree->root();
        // visit_enode(rooty);
    } catch(const kjb::Exception& e)
    {
        e.print_details_exit();
    }
    
}
