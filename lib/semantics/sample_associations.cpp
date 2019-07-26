/* $Id: sample_associations.cpp 17326 2014-08-19 01:57:36Z cdawson $ *//*!
 * @file sample_associations.cpp
 *
 * @author Colin Dawson 
 */

#include "semantics/sample_associations.h"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <ctime>
#include <vector>

namespace semantics
{

    void resample_semspear_trees(
        std::vector<Semspear_tree::Self_ptr>& trees,
        const size_t&                         iterations,
        std::ostream&                         results,
        const bool&                           output_diagnostics
        )
    {
        results << "number\told_posterior\tposterior_delta\treverse"
                << "\tproposal_delta\tmh_ratio\taccepted"
                << std::endl;

        for(size_t cycle = 0; cycle < iterations; ++cycle)
        {
            resample_event_tables(trees);
            resample_associations(trees, results, output_diagnostics);
            resample_all_event_alphas();
        }
    }
    
    void resample_associations(
        std::vector<Semspear_tree::Self_ptr>& trees,
        std::ostream&                         results,
        const bool&                           output_diagnostics
        )
    {
        typedef Semspear_tree::Self_ptr Tree_ptr;
        typedef Elaboration_tree::Elab_ptr Elab_ptr;
        typedef std::vector<Tree_ptr> Tree_vector;
        
        static boost::mt19937 urng(std::time(0));
        static boost::random::uniform_real_distribution<> unifdist;

        int index = 1;
        
                            for(Tree_vector::iterator it = trees.begin();
            it != trees.end(); ++it, ++index)
        {   
            Tree_ptr st2;
            Elaboration_tree::Elab_ptr_const sem_root((*it)->semantic_root());
            double forward_prob, reverse_prob, old_posterior, new_posterior;
            double log_mh_ratio;
            kjb::Vector probs(2);
            double u = log(unifdist(urng));

            (*it) -> release_event_counts();
            boost::tie(st2, probs) = propose_new_tree(*it);
            (st2) -> release_event_counts();
            forward_prob = probs[0];
            reverse_prob = probs[1];
            old_posterior = (*it)->subtree_log_probability(sem_root);
            new_posterior = st2->subtree_log_probability(sem_root);
            log_mh_ratio =
                new_posterior - old_posterior - forward_prob + reverse_prob;
            if(u < log_mh_ratio)
            {
                std::cerr << index << ": Accept." << std::endl;
                (*it) = st2;
            } else {
                std::cerr << index << ": Reject." << std::endl;
            }
            (*it)->reacquire_event_counts_recursively();
            if(output_diagnostics)
            {
                results << index << "\t" << old_posterior << "\t"
                        << new_posterior - old_posterior << "\t"
                        << reverse_prob << "\t"
                        << reverse_prob - forward_prob << "\t"
                        << exp(log_mh_ratio) << "\t"
                        << (u < log_mh_ratio) << "\n";
            }
        }
    }

    void resample_event_tables(std::vector<Semspear_tree::Self_ptr>& trees)
    {
        std::for_each(
            trees.begin(), trees.end(),
            boost::bind(
                static_cast<void (*)(Semspear_tree::Self_ptr&)>(
                    resample_event_tables
                    ),
                _1)
            );
    }

};

