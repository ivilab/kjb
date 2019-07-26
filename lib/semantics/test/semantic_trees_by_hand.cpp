/*!
 * @file tmp.cpp
 *
 * @author Colin Dawson 
 * $Id: semantic_elaboration_test.cpp 16912 2014-05-28 20:27:51Z cdawson $ 
 */

#include "semantics/semantic_trees_by_hand.h"
#include <boost/make_shared.hpp>
#include <vector>

int main(int, char**)
{
    using namespace boost;
    using namespace semantics;
    
    // Build a semantic tree

    typedef std::vector<Sentence_sem> Syn_sem_vector;

    Syn_sem_vector ssem_vector = collect_semantic_trees();

    for (Syn_sem_vector::const_iterator iterator = ssem_vector.begin(),
       end = ssem_vector.end();
     iterator != end;
     ++iterator)
    {
        std::cout << std::endl;
        std::cout << "Tree Example: " << iterator->example_id << std::endl;
        std::cout << "from caption: " << iterator->caption_id << std::endl;
        std::cout << "image: " << iterator->image << std::endl;
        std::cout << "sentence: " << iterator->sentence << std::endl;
        std::cout << "Elaboration Tree:" << std::endl;
        iterator->etree->print(std::cout);
        std::cout << "Parse Tree: " << iterator->parse << std::endl;
    }

    std::cout << std::endl;
    
    return 0;
}
