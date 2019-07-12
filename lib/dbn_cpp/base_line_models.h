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

/* $Id */

#ifndef KJB_TIES_BASE_LINE_MODELS_H
#define KJB_TIES_BASE_LINE_MODELS_H

#include <m_cpp/m_vector.h>
#include <g_cpp/g_line.h>

#include <string>
#include <vector>

#include "dbn_cpp/experiment.h"
#include "dbn_cpp/data.h"
#include "dbn_cpp/lss_set.h"

namespace kjb {
namespace ties{

/**
 * @brief   Fits a line to a set of points.
 */
double line_fitting
(
    const std::vector<Vector>& points, 
    Line& fitted_line
);

/**
 * @brief   Use the average of the data as the model.
 */
void average_model_fitting
(
    const std::vector<Data>& data,
    const std::string& output_dir,
    const std::string& obs_str,
    const std::string& distinguisher,
    double training_percent = 0.8,
    size_t num_obs = 1
);

/**
 * @brief   Use the average of the data as the model.
 */
inline void average_model_fitting
(
    const std::string& data_dir, 
    const std::string& list_fp, 
    const std::string& output_dir,
    const std::string& obs_str,
    const std::string& distinguisher,
    double training_percent = 0.8,
    size_t num_obs = 1
)
{
    std::vector<Data> data = parse_data(data_dir, list_fp);
    average_model_fitting(data, output_dir, obs_str, 
                          distinguisher, training_percent, num_obs);
}

/**
 * @brief   Use a line to fit the data.
 */
void line_model_fitting
(
    const std::vector<Data>& data,
    const std::string& output_dir,
    const std::string& obs_str,
    const std::string& distinguisher,
    double training_percent = 0.8,
    size_t num_obs = 1
);

/**
 * @brief   Use a line to fit the data.
 */
inline void line_model_fitting
(
    const std::string& data_dir, 
    const std::string& list_fp, 
    const std::string& output_dir,
    const std::string& obs_str,
    const std::string& distinguisher,
    double training_percent = 0.8,
    size_t num_obs = 1
)
{
    std::vector<Data> data = parse_data(data_dir, list_fp);
    line_model_fitting(data, output_dir, obs_str, distinguisher, 
                       training_percent, num_obs);
}

/**
 * @brief   Fit a coupled oscillator to each data
 */
void lss_mh_fitting
(
    const std::vector<Data>& data,
    const Ties_experiment& exp
);

/**
 * @brief   Fit a coupled oscillator to each data
 *
 * NOTE: this function is obsolete, we now use Lss_set_sampler::train_model 
 *       to train ind-clo model, which will fit the observation noise
 */
void lss_mh_fitting
(
    const Data& data,
    const Ties_experiment& exp,
    Vector& errors,
    Vector& sampled_errors,
    std::vector<Vector>& obs_errors,
    std::vector<Vector>& obs_sampled_errors
);

/**
 * @brief   Fit a coupled oscillator to each data
 */
inline void lss_mh_fitting
(
    const Ties_experiment& exp
)
{
    std::vector<Data> all_data = parse_data(
                                    exp.data.data_dp, 
                                    exp.data.id_list_fp);
    lss_mh_fitting(all_data, exp);
}

/**
 * @brief   Fit a line of the outcome_type_index'th outcome variable
 */
double line_fitting_outcome
(
    const Lss_set& lss_set,
    Line& line,
    size_t outcome_type_index = 0
);

/**
 * @brief   Fit a line to each outcome variables
 */
std::vector<double> line_fitting_outcome
(
    const Lss_set& lss_set,
    std::vector<Line>& lines
);

}} //namespace kjb::ties
#endif //KJB_TIES_BASE_LINE_MODELS_H

