#ifndef SAMPLE_ASSOCIATIONS_H_
#define SAMPLE_ASSOCIATIONS_H_

/*!
 * @file sample_associations.h
 *
 * @author Colin Dawson 
 * $Id: sample_associations.h 17175 2014-07-29 19:11:35Z cdawson $ 
 */

#include "semantics/Semspear_tree.h"
#include <vector>
#include <iostream>
#include <prob_cpp/prob_sample.h>

namespace semantics
{

    void resample_semspear_trees(
        std::vector<Semspear_tree::Self_ptr>& trees,
        const size_t&                         iterations,
        std::ostream&                         results = std::cout,
        const bool&                           output_diagnostics = false        
        );
    
    void resample_associations(
        std::vector<Semspear_tree::Self_ptr>& trees,
        std::ostream&                         results,
        const bool&                           output_diagnostics
        );

    void resample_event_tables(std::vector<Semspear_tree::Self_ptr>& trees);
};

#endif
