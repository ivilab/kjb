/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
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
   |  Author:  Jinyan Guan
 * =========================================================================== */

/* $Id: experiment.cpp 22559 2019-06-09 00:02:37Z kobus $ */

#include <string>
#include <vector>
#include <iterator>
#include <sstream>

#include "dbn_cpp/experiment.h"

using namespace kjb;
using namespace kjb::ties;

std::vector<std::vector<std::string> > Lss_set_options::get_all_moderators
(
    bool ignore_clo,
    size_t num_oscillators,
    size_t total_num_params
) const
{
    std::vector<std::vector<std::string> > moderators_for_all(total_num_params);
    for(size_t j = 0; j < moderator_params.size(); j++)
    {
        /*if(moderator_params[j] == "mass-ratio")
        {
            std::copy(moderators.begin(), moderators.end(), 
                      back_inserter(moderators_for_all[0]));
        }
        else if(moderator_params[j] == "stiffness")
        {
            for(size_t i = 0; i < num_oscillators; i++)
            {
                size_t index = i + 1;
                std::copy(moderators.begin(), moderators.end(), 
                          back_inserter(moderators_for_all[index]));
            }
        }
        else if(moderator_params[j] == "damping")
        {
            size_t num_damping_params = num_oscillators;
            for(size_t i = 0; i < num_damping_params; i++)
            {
                size_t index = i + num_oscillators + 2;
                std::copy(moderators.begin(), moderators.end(), 
                          back_inserter(moderators_for_all[index]));
            }
        }
        else if(moderator_params[j] == "polynomial")
        {
            size_t num_clo = ignore_clo ? 0 : param_length(num_oscillators);
            IFT(total_num_params > num_clo, Illegal_argument,
                    "You can not specify polynomial moderator since you did not"
                    "specify modeling offset or trend in the model");
            size_t start_index = num_clo; 
            for(size_t i = start_index; i < total_num_params; i++)
            {
                std::copy(moderators.begin(), moderators.end(), 
                              back_inserter(moderators_for_all[i]));
            }
        }
        else
        {
            KJB_THROW_2(Illegal_argument, "moderator-params must be:\n "
                                          " mass-ratio \n"
                                          " stiffiness \n"
                                          " damping \n"
                                          " or \n"
                                          " damping\n");
        }*/
    }

    // The moderator apply to each parameter by default
    if(moderator_params.empty())
    {
        for(size_t i = 0; i < total_num_params; i++)
        {
            std::copy(moderators.begin(), moderators.end(), 
                      back_inserter(moderators_for_all[i]));
        }
    }

    return moderators_for_all;
}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

void kjb::ties::generate_model_name(Ties_experiment& exp)
{
    if(exp.run_average_model)
    {
        exp.model_name = "average";
        return;
    }
    if(exp.run_line_model)
    {
        exp.model_name = "line";
        return;
    }

    // Generate the model names 
    if(exp.run.fit_ind_clo)
    {
        exp.model_name = "MLE-CLO";
    }
    else if(exp.prior.fixed_clo)
    {
        exp.model_name = "Shared-CLO";
    }
    else
    {
        exp.model_name = "MAP-CLO";
    }

    if(exp.lss.polynomial_degree == 0)
    {
        exp.model_name = exp.model_name + "-offset";
    }

    if(exp.lss.polynomial_degree == 1)
    {
        exp.model_name = exp.model_name + "-linear-trend";
    }

    if(exp.lss.drift)
    {
        exp.model_name = exp.model_name + "-drift";
    }

    if(!exp.lss_set.moderators.empty())
    {
        exp.model_name = exp.model_name + "-moderator";
        BOOST_FOREACH(const std::string& mod, exp.lss_set.moderators)
        {
            exp.model_name = exp.model_name + "-" + mod; 
        }
    }

}

/* \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ */

