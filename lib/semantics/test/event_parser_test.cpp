/*!
 * @file event_parser_test.cpp
 *
 * @author Colin Dawson 
 * $Id: event_parser_test.cpp 17426 2014-08-30 00:36:27Z predoehl $ 
 */

#include "l/l_incl.h"
#include "l/l_init.h"
#include "semantics/Semspear_tree.h"
#include "semantics/Event_parser.h"
#include "semantics/Semspear_tree_parser.h"
#include "semantics/Elaboration_tree.h"
#include "semantics/sample_associations.h"
#include "spear/Parameters.h"
#include "spear/spear_train.h"
#include "collins/Collins.h"
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <algorithm>
#include <limits>
#include <utility>

using namespace semantics;

boost::shared_ptr<Elaboration_tree> build_dummy_semantic_tree()
{
    initialize_semantic_maps();
    boost::shared_ptr<Elaboration_tree> e(new Elaboration_tree("LAMP"));
    // std::cout << "0:" << std::endl;
    // e.print(std::cout);
    e->elaborate_with_relation("FRONT_OF", "BED");
    // std::cout << "1:" << std::endl;
    // e.print(std::cout);
    e->elaborate_with_relation("SUPP_BY", "TABLE");
    // std::cout << "2:" << std::endl;
    // e.print(std::cout);
    e->elaborate_color("WHITE");
    // std::cout << "3:" << std::endl;
    // e.print(std::cout);
    e->null_elaboration();
    // std::cout << "4:" << std::endl;
    // e.print(std::cout);
    e->elaborate_color("GREEN");
    // std::cout << "5:" << std::endl;
    // e.print(std::cout);
    e->null_elaboration();
    // std::cout << "6:" << std::endl;
    return e;
}

std::vector<spear::Word> read_collins_sentence(std::istream& stream)
{
    std::vector<spear::Word> sentence;
    int number = -2;
    stream >> std::ws >> number;
    if(stream.eof() || number <= 0) return sentence;
    for(int i = 0; i < number; i++)
    {
    std::string w, t;
    stream >> std::ws >> w;
    stream >> std::ws >> t;
    spear::Word word(w, t);
    sentence.push_back(word);
    }
    return sentence;
}

std::vector<std::pair<spear::TreePtr, double> >
collins_parse_file(const std::string& file)
{
    std::vector<spear::TreePtr> trees;
    std::ifstream input(file.c_str());
    std::vector<spear::Word> sentence;
    std::vector<std::pair<spear::TreePtr, double> > result;
    while(!(sentence = read_collins_sentence(input)).empty())
    {
    double prob = MINDOUBLE;
    spear::TreePtr tree = spear::Collins::parse(sentence, prob, false);
    result.push_back(std::pair<spear::TreePtr, double>(tree, prob));
    }
    return result;
}



void display_first_n_trees_in(
    const Event_db&   db,
    std::ostream&     os_trees = std::cerr,
    std::ostream&     os_probs = std::cout,
    const size_t&     n = std::numeric_limits<size_t>::max()
    )
{
    for(size_t i = 0; i < std::min(n, db.num_trees()); i++)
    {
        Syntactic_event::Event_ptr ep = db.get_tree(i);
        ep -> print_subtree(os_trees);
        os_probs  << i+1 << ": "
              << "Log probability = "
              << ep -> subtree_log_probability()
              << std::endl;
    }
}


int main(int argc, char* argv[])
{
#ifdef USE_SEMANTICS
    std::cerr << "Semantic mode on." << std::endl;
#endif
    typedef spear::Parameters Parameters;
    
    kjb_c::kjb_init();

    /// Seed a random number generator

    // Not ready to do anything in batch mode.
    if(!kjb_c::is_interactive())
    {
    return EXIT_SUCCESS;
    }


    if(argc < 2 || argc > 12)
    {
    std::cout << "Usage: " << argv[0] 
          << "--train.path=path "
          << "[--test.path=path "
          << "--test.tree=file "
          << "--verbose=true/false "
          << "--lf.thresh=n "
          << "--max.events=n]"
          << std::endl;
    return EXIT_FAILURE;
    }

    /// Read in command line parameters
    Parameters::read(argc, argv);
    int verbose;
    std::string lexicon_path, train_path, test_path, test_tree, tagged_file;
    std::string collins_output, collins_train_path, results_filename;
    int lf_thresh = 0, max_events = 10000000, iterations = 100;

    Parameters::get("train.path", train_path);
    Parameters::get("lexicon.path", lexicon_path);
    Parameters::get("lf.thresh", lf_thresh);
    Parameters::get("max.events", max_events);
    Parameters::get("test.path", test_path);
    Parameters::get("test.tree", test_tree);
    Parameters::get("tagged.file", tagged_file);
    Parameters::get("verbose", verbose);
    Parameters::get("collins.output", collins_output);
    Parameters::get("results.filename", results_filename);
    Parameters::get("collins.train.path", collins_train_path);
    Parameters::get("mcmc.iterations", iterations);

    /// Show parameter values
    Parameters::display(std::cerr);
    
    /// Set verbosity
    if(Parameters::contains("verbose") && verbose == true)
    {
    set_verbosity<Syntactic_event>(true);
    // set_verbosity<S1_view>(true);
    // set_verbosity<S2_view>(true);
    // set_verbosity<U_view>(true);
    // set_verbosity<D1_view>(true);
    // set_verbosity<D2_view>(true);
    // set_verbosity<Cell>(true);
    set_verbosity<Event_db>(true);
    set_verbosity<Semspear_tree>(true);
    set_verbosity<Semspear_tree_parser<Semspear_tree> >(true);
    }
    
    if(!Parameters::contains("collins.train.path"))
    {
    collins_train_path = train_path;
    }
    if(!Parameters::contains("lexicon.path"))
    {
    lexicon_path = train_path;
    }

    /// Define file variables
    std::string lexicon_file = lexicon_path + "/grammar.lexicon";
    std::string nt_file = lexicon_path + "/grammar.nts";
    Syntactic_event::lexicon() = Lexicon_db(lexicon_file);
    Syntactic_event::nt_lexicon() = Nonterminal_db(nt_file);
    std::ofstream results_file(results_filename.c_str());
    std::ostream& results = 
    Parameters::contains("results.filename") ? results_file : std::cout;


    /// Parse tagged.file, if provided, and output to collins.output
    std::vector<std::pair<spear::TreePtr, double> > tree_db;
    if(Parameters::contains("tagged.file"))
    {
    // train Collins' parser from train.path
    spear::Collins::initialize(collins_train_path);
    std::cerr << "Collins parser initialized." << std::endl;
    // parse tagged.file
    tree_db = collins_parse_file(tagged_file);
    std::cerr << "Collins parser finished." << std::endl;
    // output to collins.output
    if(Parameters::contains("collins.output"))
    {
        std::ofstream of(collins_output.c_str());
        for(int j = 0; j < (int) tree_db.size(); j++)
        {
        tree_db[j].first -> displayParens(of, true);
        of << std::endl;
        }
        if(Parameters::contains("test.path"))
        {
        // if test.path provided, build features from the parses and
        // put them there, so that test is working on the same parses
        spear::train_collins(collins_output, test_path, lf_thresh);
        }
    }
    }

    /// Read in training feature file
    Event_db train_db;
    if(Parameters::contains("train.path"))
    {
    train_db =
        process_event_file(train_path, lexicon_file, max_events);
    std::cerr << "Training finished." << std::endl;
    std::cerr << std::endl;
    }

    /// Read in test feature file
    Event_db test_db;
    if(Parameters::contains("test.path"))
    {
        test_db =
        process_event_file(train_path, lexicon_file, max_events, test_path);
    std::cerr << "Testing finished." << std::endl;
    }

    /// Read in parses from test.tree file or collins.output file
    std::vector<Semspear_tree::Self_ptr> sstree_db;
    if(Parameters::contains("test.tree") ||
       Parameters::contains("collins.output"))
    {
    boost::shared_ptr<Semspear_tree> epp;
    std::ifstream collins_tree_stream(collins_output.c_str());
    std::ifstream test_tree_stream(test_tree.c_str());
    std::ifstream lexicon_stream(lexicon_file.c_str());
#ifdef USE_SEMANTICS
    boost::shared_ptr<Elaboration_tree> sem_tree =
        build_dummy_semantic_tree();
    std::cerr << "Semantic tree:" << std::endl;
    sem_tree->print(std::cerr);
#endif
        Semspear_tree_parser<Semspear_tree> test_parser(
            Parameters::contains("test.tree")
        ? test_tree_stream
        : collins_tree_stream,
        lexicon_stream,
            true
            );
    size_t index = 1;
    /// Read parses from the file until exhausted
    while((epp = test_parser.parse_constituent()) != NULL)
    {
        std::cerr << "Parsed tree " << index << std::endl;
#ifdef USE_SEMANTICS
        epp -> set_semantic_tree(sem_tree);
        epp -> complete_tree();
#endif
        sstree_db.push_back(epp);
        index++;
    }
    std::cerr << "Finished parsing " << index << "trees." << std::endl;
    }


    /// Display resulting trees and log probabilities.
    results << "number\tcollins\ttrain\ttest\tparsed" << std::endl;
    for(unsigned int index = 0;
    index < std::max(train_db.num_trees(), sstree_db.size());
    ++index)
    {
    std::pair<spear::TreePtr, double> cparse;
    Syntactic_event::Event_ptr tr; 
    Syntactic_event::Event_ptr te;
    Semspear_tree::Self_ptr st;

    /// If tagged.file provided, print Collins probabilities for the results
    results << index + 1 << "\t";
    if(Parameters::contains("tagged.file") && index < tree_db.size())
    {
        cparse = tree_db[index];
        std::cerr << "COLLINS TREE " << index + 1 << ":" << std::endl;
        cparse.first -> displayPrettyParens(std::cerr, 0, true);
        results << cparse.second;
    }
    /// If train.path provided, print Event_db probabilities for training ss
    results << "\t";
    if(Parameters::contains("train.path") && index < train_db.num_trees())
    {
        tr = train_db.get_tree(index);
        std::cerr << "TRAIN TREE " << index + 1 << ":" << std::endl;
        std::cerr << std::endl;
        tr -> print_subtree(std::cerr);
        tr -> print_subtree_view_counts(std::cerr);
        results << tr -> subtree_log_probability();
    }
    // If test.path provided, print Event_db probabilities for test ss
    // (comparable to collins probs if tagged.file and collins.output
    // were provided, or if test.path was the result of a previous
    // Collins run)
    results << "\t";
    if(Parameters::contains("test.path") && index < test_db.num_trees())
    {
        te = test_db.get_tree(index);
        std::cerr << "TEST TREE " << index + 1 << ":" << std::endl;
        te -> print_subtree(std::cerr);
        te -> print_subtree_view_counts(std::cerr);
        results << te -> subtree_log_probability();
    }
    // If test.tree or collins.output provided, display probabilities
    // from the tree-reader
    results << "\t";
    if((Parameters::contains("test.tree") ||
        Parameters::contains("collins.output")) &&
       index < (int) sstree_db.size())
    {
        st = sstree_db[index];
        std::cerr << "PARSED TREE:" << index + 1 << std::endl;
        st -> print_dependency_tree(std::cerr);
#ifdef USE_SEMANTICS
        results << st->subtree_log_probability(st->semantic_root()) << "\t";
#else
        results << st->subtree_log_probability() << "\t";
#endif
        st -> print_events_with_probabilities(std::cerr);
    }
    results << std::endl;
    }

#ifdef USE_SEMANTICS    

    resample_semspear_trees(sstree_db, iterations, results, true);
    
    for(std::vector<Semspear_tree::Self_ptr>::iterator it = sstree_db.begin();
        it != sstree_db.end(); ++it)
    {
    (*it)->print_dependency_tree(std::cout);
    }
    std::cerr << "SEMANTICS!!!" << std::endl;
#endif
    return EXIT_SUCCESS;
}
