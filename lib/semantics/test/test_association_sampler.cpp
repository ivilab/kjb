/* $Id: test_association_sampler.cpp 18606 2015-02-28 22:30:03Z cdawson $ */

/*!
 * @file test_association_sampler.cpp
 *
 * @author Colin Dawson 
 */

#include "l/l_incl.h"
#include "l/l_init.h"
#include "semantics/Semspear_tree.h"
#include "semantics/Event_parser.h"
#include "semantics/semantic_trees_by_hand.h"
#include "semantics/Semspear_tree_parser.h"
#include "semantics/sample_associations.h"
#include "spear/Parameters.h"
#include "spear/spear_train.h"
#include <string>
#include <fstream>

int main(int argc, char** argv)
{
    // using namespace spear;
    using namespace semantics;

#ifdef USE_SEMANTICS
    std::cerr << "SEMANTICS!" << std::endl;
#endif

    typedef spear::Parameters Parameters;
    typedef Semspear_tree::Self_ptr Tree_ptr;
    typedef Elaboration_tree::Self_ptr Sem_tree_ptr;
    typedef Elaboration_tree::Elab_ptr Elab_ptr;
    typedef Elaboration_tree::Elab_ptr_const Elab_ptr_const;
    typedef Syntactic_event::Self_ptr Syntactic_ptr;

    std::string feature_path, trees_out, results_file;
    int iterations = 100;

    // read command-line parameters
    Parameters::read(argc, argv);
    Parameters::get("trees.out", trees_out);
    Parameters::get("feature.path", feature_path);
    Parameters::get("mcmc.iterations", iterations);
    Parameters::get("results.file", results_file);
    Parameters::display(std::cerr);
    
    // process arguments
    std::string lexicon_file = feature_path + "/grammar.lexicon";
    std::string nt_file = feature_path + "/grammar.nts";
    Syntactic_event::lexicon() = Lexicon_db(lexicon_file);
    Syntactic_event::nt_lexicon() = Nonterminal_db(nt_file);
    std::ofstream tree_out_file(trees_out.c_str());
    std::ofstream results_fstream(results_file.c_str());
    std::ostream& results(
        Parameters::contains("results.file") ? results_fstream : std::cout
        );

    // read in pure-syntactic data from spear-formatted event file
    Event_db sentence_db =
        process_event_file(feature_path, lexicon_file, 100000);
    std::cerr << "Processed " << sentence_db.num_trees() << " trees."
              << std::endl;

    // write resulting trees to output file with head markings
    Syntactic_ptr tr;
    for(size_t j = 0; j < sentence_db.num_trees(); ++j)
    {
        tr = sentence_db.get_tree(j);
        // tr->print_subtree(tree_out_file);
        tr->print_constituency_tree_with_head(tree_out_file);
        tree_out_file << std::endl;
    }

    // read in hard-coded semantic trees from semantic_trees_by_hand
    std::vector<Tree_ptr> sstree_db;
    std::vector<Sentence_sem> sem_tree_db = collect_semantic_trees();
    std::ifstream trees_with_heads(trees_out.c_str());
    std::ifstream lexicon_stream(lexicon_file.c_str());

    // instantiate a parser object pointing to the syntactic tree bank
    Semspear_tree_parser<Semspear_tree> parser(
        trees_with_heads,
        lexicon_stream,
        true
        );

    // Iterate through the tree bank and associate syntactic trees with
    // semantic ones
    Tree_ptr epp = parser.parse_constituent();
    for(int j = 0; epp != NULL; epp = parser.parse_constituent(), ++j)
    {
        sem_tree_db[j].etree -> print(std::cerr);
        epp->set_semantic_tree(sem_tree_db[j].etree);
        epp->complete_tree();
        sstree_db.push_back(epp);
    }
    std::cerr << "There are " << sstree_db.size() << " syntactic trees, and "
              << sem_tree_db.size() << " semantic trees." << std::endl;

    for(std::vector<Tree_ptr>::const_iterator it =
            sstree_db.begin();
        it != sstree_db.end();
        ++it)
    {
        Elab_ptr_const semantic_root((*it)->semantic_root());
        (*it)->print_dependency_tree(std::cerr);
        std::cerr << (*it)->subtree_log_probability(semantic_root) << std::endl;
        (*it)->release_event_counts();
        std::cerr << (*it)->subtree_log_probability(semantic_root) << std::endl;
        (*it)->reacquire_event_counts_recursively();
        std::cerr << (*it)->subtree_log_probability(semantic_root) << std::endl;
    }

    resample_semspear_trees(sstree_db, iterations, results, true);

    for(std::vector<Tree_ptr>::const_iterator it =
            sstree_db.begin();
        it != sstree_db.end();
        ++it)
    {
        Elab_ptr_const semantic_root((*it)->semantic_root());
        (*it)->print_dependency_tree(std::cerr);
        std::cerr << (*it)->subtree_log_probability(semantic_root) << std::endl;
        (*it)->release_event_counts();
        std::cerr << (*it)->subtree_log_probability(semantic_root) << std::endl;
        (*it)->reacquire_event_counts_recursively();
        std::cerr << (*it)->subtree_log_probability(semantic_root) << std::endl;
    }

    return 0;
}


