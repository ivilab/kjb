/*!
 * @file tmp.cpp
 *
 * @author Colin Dawson 
 * $Id: semantic_elaboration_test.cpp 17426 2014-08-30 00:36:27Z predoehl $ 
 */

#include "semantics/Elaboration_tree.h"
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdlib>
#include <ctime>
#include <queue>

using namespace semantics;

boost::shared_ptr<Elaboration_tree> build_dummy_semantic_tree()
{
    boost::shared_ptr<Elaboration_tree> e = boost::make_shared<Elaboration_tree>("TABLE", 1);
    e->elaborate_with_relation("NEAR", "CHAIR", 2);
    e->elaborate_with_relation("NEAR", "MIRROR", 3);
    e->elaborate_with_relation("RIGHT_OF", "LAMP", 4);
    e->elaborate_size("LONG");
    e->elaborate_color("BROWN");
    e->null_elaboration();
    e->elaborate_with_relation("LEFT_OF", "BED", 5);
    e->elaborate_with_relation("NEAR", "LAMP", 4);
    e->elaborate_color("BROWN");
    e->null_elaboration();
    e->elaborate_size("LONG");
    e->null_elaboration();
    e->null_elaboration();
    return e;
}


int main(int, char**)
{
    using namespace boost;
    using namespace semantics;

    typedef shared_ptr<Semantic_elaboration> Elab_ptr;
    
    // Build a semantic tree

    initialize_semantic_maps();

    std::vector<std::queue<int> > meh(3);

    boost::shared_ptr<Elaboration_tree> eptr = build_dummy_semantic_tree();

/// Print the tree
    
    std::cout << "Full tree:" << std::endl;
    // e.print(std::cout);
    eptr->print(std::cout);

    // std::cout << "Randomly stepping down the tree" << std::endl;
    
    // shared_ptr<Semantic_elaboration> tree_walker = e.root();
    // srand(time(0));

    // boost::array<Step_code_t, 10> steps =
    //  {{Step_code::LEFT_ARG, Step_code::LEFT_ARG,
    //    Step_code::HEAD, Step_code::IDENTITY,
    //    Step_code::IDENTITY, Step_code::IDENTITY,
    //    Step_code::IDENTITY, Step_code::IDENTITY,
    //    Step_code::NULL_STEP, Step_code::NULL_STEP}};
    
    // for(int counter = 0; counter < 10; counter++)
    // {
    //  Semantic_data_base::Hash_pair hash_codes;
    //  Step_code_t step_code = steps[counter];
    //      // static_cast<Step_code_t>(rand() % Step_code::NUM_STEPS);
    //  if(tree_walker != NULL) tree_walker -> print_subtree(std::cout);
    //  tie(tree_walker, hash_codes) =
    //      Elaboration_tree::take_a_step(tree_walker, step_code);
    //  std::cout << "After step # " << counter << " : "
    //        << enum_to_string<Step_code_t>(step_code) << std::endl;
    // }

    return 0;
}
