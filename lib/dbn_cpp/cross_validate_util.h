/* =========================================================================== *
|
|  Copyright (c) 1994-2008 by Kobus Barnard (author).
|
|  Personal and educational use of this code is granted, provided that this
|  header is kept intact, and that the authorship is not misrepresented, that
|  its use is acknowledged in publications, and relevant papers are cited.
|
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).
|
|  Please note that the code in this file has not necessarily been adequately
|  tested. Naturally, there is no guarantee of performance, support, or fitness
|  for any particular task. Nonetheless, I am interested in hearing about
|  problems that you encounter.
|
| Authors:
|     Jinyan Guan
|
* =========================================================================== */

/* $Id: cross_validate_util.h 22559 2019-06-09 00:02:37Z kobus $ */

#ifndef KJB_TIES_CROSS_VALIDATE_UTIL_H
#define KJB_TIES_CROSS_VALIDATE_UTIL_H

#include <m_cpp/m_vector.h>

#include <vector>
#include <string>

#include "dbn_cpp/experiment.h"

namespace kjb{
namespace ties{


/**
 * @brief   Worker function for multithreaded cross-validation.
 *          All folds run in parallell. 
 */
void fold_worker
(
    size_t fold, 
    std::vector<Vector>& training_errors, 
    std::vector<Vector>& testing_errors, 
    std::vector<Vector>& sample_errors,
    std::vector<std::vector<Vector> >& obs_training_errors, 
    std::vector<std::vector<Vector> >& obs_testing_errors, 
    std::vector<std::vector<Vector> >& obs_sample_errors,
    const Ties_experiment& exp
);

/**
 * @brief   Write the ids in to training and testing list
 */
void write_fold_list
(
    size_t fold,
    const std::string& training_fp, 
    const std::string& testing_fp,
    const std::vector<size_t>& ids,
    int K,
    size_t num_test, 
    size_t& last_end
);

/**
 * @brief   Create all the folds 
 *
 */
size_t create_fold
(
    const std::string& out_dp,
    const std::string& list_fp,
    size_t num_folds = 0,
    const std::string& grouping_var = std::string("")
);

/** @brief  Generate training and testing data for the cross-validation. */
//size_t create_fold (const Ties_experiment& exp);

/** @brief  Run training and testing on one fold. */
void run_fold(const Ties_experiment& exp);

/** @brief  Run training and testing on all folds. */
void run_cross_validate(const Ties_experiment& exp);

}}; //namespace kjb::ties

#endif //KJB_TIES_CROSS_VALIDATE_UTIL_H
